cpnt.cxx contains a uno component that is uses to test the olebridge within a
JavaScript context.  In particular it has functions using out, in/out and
sequence parameter that have to be specifically treated in JScript because they
are represented by IDispatch objects.  Other clients, such as VBScript, VB and
C++ applications can also use this component for testing.
The service name is oletest.OleTest.
The uno types are also build in this folder. The idl file is in ..\idl. The
types rdb is in wntmscixx.pro\bin\oletest.rdb

The service is used by:
OleTest.htm 
JScriptNewStyle.htm
ScriptTest.html


Prerequisites:

The oletest.dll must be registered, for example in the services.rdb, using
regcomp. Then the uno types contained in oletest.rdb (wntmscixx/bin) must be merged
for example in the offapi.rdb. This is done with regmerge.

