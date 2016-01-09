This test is for Windows only!

The cli test uses .NET assemblies:
cli_types.dll,
cli_basetypes.dll,
cli_ure.dll,
cli_cppuhelper.dll.


When an office is properly installed then these assemblies can be found in the
global assembly cache (GAC), for example in c:\windows\assembly.

When the test is run then the assemblies are used from the GAC. That is
one has to make sure that one has the respective office installed.

The test can also be run without an installed office. Then the assemblies are used
which reside next to the executable. The testtools project copies the assemblies from
the build environment into the wntmscixx\bin folder.

However, if for some reason an assembly remains in the GAC than it is used no matter
what.


The qa test simply executes the cli_bridgetest_inprocess.exe. All console output is descarded.
When the test fails one should directly run that executable. Then one may see the cause
of the failure in the console.
