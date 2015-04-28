mftp: mftp.o
	g++ mftp.o -o mftp
mftp.o: mftp.cpp
	g++ mftp.cpp -c