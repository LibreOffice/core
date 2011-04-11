The test program test the unloading API, and the default factories from cppuhelper 
for their new XUnloadingPreference interface.

The makefile produces the executable unloading.exe as wells as two 
libraries, samplelib1 and samplelib2, which contain services.


Usage:
 Copy unloading, samplelib1, samplelib2 into the program directory
of an office.

Register samplelib1 and samplelib2 with regcomp

Run unloading.