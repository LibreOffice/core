testcppuno.exe currently tests the conversion of SAFEARRAYs. There is
no output from the program. Just step through the code.

To run the test, the service oletest.OleTest ( extensions/test/ole/cpnt)
must be in the same directory as the office dlls and it must be registered
at the applicat.rdb. Also the type information must have been merged with
applicat.rdb. The program requires an office to be present. Put testcppuno
into the Office/program folder and run it there.
	