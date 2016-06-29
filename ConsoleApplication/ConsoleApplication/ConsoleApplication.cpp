// ConsoleApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>

void text2ascii() { // text.txt -> xtext.txt
	unsigned char ch = 'A';
	FILE *fpOut, *fpIn;
	fpIn = fopen("text.txt", "r");
	fpOut = fopen("xtext.txt", "w");
	while (fscanf(fpIn, "%c", &ch) != EOF) {
		if (ch != '\n') {
			fprintf(fpOut, "%02X", ch);
		}
	}
	fclose(fpOut);
	fclose(fpIn);
}

void ascii2text() { // ptext.txt -> ctext.txt
	unsigned char ch;
	FILE *fpOut, *fpIn;
	fpIn = fopen("ptext.txt", "r");
	fpOut = fopen("ctext.txt", "w");
	while (fscanf(fpIn, "%02X", &ch) != EOF) {
		fprintf(fpOut, "%c", ch);
	}
	fclose(fpOut);
	fclose(fpIn);
}

int main(int argc, char *argv[])
{
	unsigned int ch;
	FILE *fpOut, *fpIn;
	fpIn = fopen("ptext.txt", "r");
	fpOut = fopen("ctext.txt", "w");

	// READ //

	//read and calculate symbol frequencies
	unsigned int strlength = 0;
	double p_frequencies[256]; //0xFF
	for (unsigned int i = 0; i < 256; i++)
		p_frequencies[i] = 0;

	while (fscanf(fpIn, "%02X", &ch) != EOF) {
		strlength++;
		(p_frequencies[ch])++;
	}
	if (strlength != 0) {
		for (unsigned int i = 0x00; i < 0xFF; i += 0x01) {
			p_frequencies[i] = p_frequencies[i] / strlength;
			fprintf(fpOut, "%02X : %f\n", i, p_frequencies[i]);
		}
	}
	fclose(fpIn);

	//rewrite file to string
	fpIn = fopen("ptext.txt", "r");
	unsigned int *str = new (unsigned int[strlength]); 
	int k = 0;
	while (fscanf(fpIn, "%02X", &ch) != EOF) {
		str[k] = ch;
		k++;
	}

	fclose(fpOut);
	fclose(fpIn);

	// DETECT N //

	double zero_stream_p_frequencies[256];
	for (unsigned int i = 0; i < 256; i++)
		zero_stream_p_frequencies[i] = 0;

	double maxsum = 0.0; unsigned int theN = 0;
	char* fileNameF = "data/frequencies_N.txt";
	FILE *fpOutF = fopen(fileNameF, "w");
	for (unsigned int N = 1; N < 14; N++) {

		//calculate zero-stream symbol frequencies with N
		char fileName[27];
		sprintf(fileName, "%s%02d%s", "data/ctext_N", N, "_STREAM0.txt");
		fpOut = fopen(fileName, "w");
		unsigned int strl0 = 0;
		for (unsigned int i = 0; i < strlength; i += N) {
			ch = str[i]; strl0++;
			(zero_stream_p_frequencies[ch])++;
		}
		if (strl0 != 0) {
			for (unsigned int i = 0x00; i < 0xFF; i += 0x01) {
				zero_stream_p_frequencies[i] = zero_stream_p_frequencies[i] / strl0;
				fprintf(fpOut, "%f\n", zero_stream_p_frequencies[i]);
			}
		}
		fclose(fpOut);

		//if close to uniform => | frequency ~ (1/256) | => | sum squares ~= 256*(1/256)^2 = 1/256 |
		double sum = 0.0;
		for (unsigned int i = 0x00; i < 0xFF; i += 0x01) {
			sum += (zero_stream_p_frequencies[i])*(zero_stream_p_frequencies[i]);
		}
		fprintf(fpOutF, "%f\n", sum);
		if (maxsum < sum) {
			maxsum = sum; theN = N;
		}

	}
	fclose(fpOutF);
	unsigned int *theKey = new (unsigned int[theN]); 
	for (int i=0; i<theN; i++)
		theKey[i] = 0x00;

	// DECRYPT TEXT //

	//decrypt stream0..streamN with any keyB
	char* fileNameS = "data/frequencies_sum.txt";
	FILE *fpOutS = fopen(fileNameS, "w"); 
	for (unsigned int stream = 0; stream < theN; stream++) {
		double maxsum = 0.0;
		for (unsigned int keyB = 0x00; keyB < 0xFF; keyB += 0x01) {

			char fileName[24];
			sprintf(fileName, "%s%02X%s%02d%s", "data/B_", keyB, "_STREAM", stream,".txt");
			fpOut = fopen(fileName, "w");
			double q_frequencies[256];
			for (unsigned int i = 0; i < 256; i++)
				q_frequencies[i] = 0;
			unsigned int strl0 = 0;
			for (unsigned int i = stream; i < strlength; i += theN) {

				//xor with B
				ch = str[i]; strl0++;
				unsigned int xor = ch ^ keyB;
				fprintf(fpOut, "%02X\n", xor);

				//check keys with all printable symbols
				if ((xor >= 32) && (xor <= 127)) {

					//if true result => sum_{s='a'..'z'} frequency('of symbol s')*frequency(keys0['symbol s']) = sum_{s='a'..'z'} (frequency('of symbol s'))^2 ~= 0.065 (*)
					if ((xor >= 97) && (xor <= 122) && (strl0 != 0)) { //'a'..'z'
						(q_frequencies[xor])++;
					}
				}
				else {
					i = strlength + 1; //next key B
					strl0 = 0;
				}
			}
			fclose(fpOut);
			if (strl0 != 0) {
				for (unsigned int i = 0x00; i < 0xFF; i += 0x01) {
					q_frequencies[i] = q_frequencies[i] / strl0;
				}

				//read wiki symbol frequencies
				double wiki_symbols[26] = { 0.0817,    0.0149,    0.0278,    0.0425,    0.1270,    0.0223,    0.0202,    0.0609,    0.0697,    0.0015,    0.0077,    0.0403,
										   0.0241,    0.0675,    0.0751,    0.0193,    0.0010,    0.0599,    0.0633,    0.0906,    0.0276,    0.0098,    0.0236,    0.0015,
										   0.0197,    0.0007 };
				double p[256];
				for (unsigned int i = 0; i < 256; i++) {
					p[i] = 0.0;
				}
				for (unsigned int i = 0; i < 26; i++) {
					p[97 + i] = wiki_symbols[i];
				}

				//calculate summation (*)
				double sum = 0.0;
				for (unsigned int i = 97; i <= 122; i++) {
					sum = sum + q_frequencies[i] * p[i];
				}
				fprintf(fpOutS, "%d, %02X, %f\n", stream, keyB, sum);
				if (maxsum < sum) {
					maxsum = sum;
					theKey[stream] = keyB;
				}
			}
		}
	}
	fclose(fpOutS);
	
	//the Key
	fpOutS = fopen("data/result_key.txt", "w");
	for (unsigned int stream = 0; stream < theN; stream++) {
		fprintf(fpOutS, "0x%02X ", theKey[stream]);
	}
	fclose(fpOutS);

	//the Result
	fpOutS = fopen("data/result.txt", "w");
	for (unsigned int i = 0; i < strlength; i++) {
		ch = str[i];
		unsigned char xor = ch ^ (theKey[i % theN]);
		fprintf(fpOutS, "0x%02X ", xor);
	}
	fprintf(fpOutS, "\n");
	for (unsigned int i = 0; i < strlength; i++) {
		ch = str[i];
		unsigned char xor = ch ^ (theKey[i % theN]);
		fprintf(fpOutS, "%c", xor);
	}
	fclose(fpOutS);

	delete []str;
	delete []theKey;
	return 0;
}


