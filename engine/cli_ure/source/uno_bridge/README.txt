Because of the LoaderLock bug in .NET Framework 1.0 and 1.1 the cli_uno.dll is linked 
with the /NOENTRY switch, which prevent that the C-runtime is initialized when loading 
the dll.

Also I removed all static c++ objects which need construction by the CRT, 
exception handling seems to need an initialised CRT. Therefore
I added CRT initialization code in  uno_initEnvironment (cli_bridge.cxx)
However there is no deinitialization done because bridge libraries remain 
in memory until the process dies. There is actually no good place where 
this had to be called. If we would do that we would have to implement that
the bridge can be disposed.


Sell also:

http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dv_vstechart/html/vcconmixeddllloadingproblem.asp
http://support.microsoft.com/?id=814472
http://www.ddj.com/dept/windows/184416689
http://blogs.msdn.com/cbrumme/archive/2003/08/20/51504.aspx
http://msdn2.microsoft.com/en-US/library/ms172219.aspx