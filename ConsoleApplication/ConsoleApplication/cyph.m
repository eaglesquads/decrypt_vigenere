dir = '';%'C:\Users\svetlanal\Documents\Visual Studio 2015\Projects\ConsoleApplication\ConsoleApplication\';
%N01_STREAM0 = importdata([dir 'ctext_N01_STREAM0.txt']);
%figure; plot(1:255,N01_STREAM0,'-bo');
%N02_STREAM0 = importdata([dir 'ctext_N02_STREAM0.txt']);
%figure; plot(1:255,N02_STREAM0,'-ro');
%N03_STREAM0 = importdata([dir 'ctext_N03_STREAM0.txt']);
%figure; plot(1:255,N03_STREAM0,'-go');

frequencies_N = importdata([dir 'data/frequencies_N.txt']);
figure; plot(1:13,frequencies_N,'-ko','MarkerSize',5,'MarkerFaceColor','g');
hold on; plot(1:13,(1/256)*[1 1 1 1 1 1 1 1 1 1 1 1 1],'--b');
hold off;

frequencies_sum = importdata([dir 'data/frequencies_sum.txt']);
n = size(frequencies_sum,1);
figure; plot(1:n,frequencies_sum,'-ko','MarkerSize',5,'MarkerFaceColor','r');
hold on; plot([1 n],(0.056)*[1 1],'--b');
hold off;

wiki = importdata('wiki_symbols.txt');
wiki'*wiki
