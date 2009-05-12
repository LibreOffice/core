cpnt.cxx contains a uno component that is uses to test the olebridge
within a JavaScript context.
In particular it has functions using out, in/out and sequence parameter
that have to be specifically treated in JScript because they are represented
by IDispatch objects.

The test is run by  JavaScript contained in OleTest.htm and JScriptNewStyle.htm.


Prerequisites:
The oletest.dll ( build in this directory) must reside in the <Office Dir>/program
directory and it must have been registered with regcomp.exe.

The types in wntmsci/bin/oletest.rdb must be merged with the applicat.rdb. oletest.rdb
is automatically build in this directory. However the idl file is in test/ole/idl.