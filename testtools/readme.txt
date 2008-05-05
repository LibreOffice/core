How to check compatibility between compilers
============================================

Since the interfaces used in the cpp bridgetest are not changed often
one can just build the cppobj.uno.dll and the constructors.uno.dll
(testtools/source/bridgetest) in an 
old environment and then use them in the new environment. That is the files 
are copied into the testtools/wntmsciXX.pro folder which corresponds to the 
new environment.

On Windows this test will typically fail because the tests use the 
cppu::getCaughtException function, which only works when all libs are build 
using the same runtime.

This part of the test can switched off. To do this go into the 
testtools/source/bridgetest folder and call
dmake compcheck=1

This will add a new compiler define (-DCOMPCHECK) and will be used in the 
bridgetest.cxx to switch off the code which uses the getCaughtException function.
However, there is still a test which causes the test component to throw
and IllegalArgumentException. This still works.
