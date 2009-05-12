Runs a test written in VisualBasic. If no error message appears then the test was ok.

Requirements:
Installed office
Component oletest.Oletest (extensions/test/ole/cpnt)
Component VBasicEventListener.VBEventListener (extensions/test/ole/EventListenerSample/VBEventListener


OleTest is a UNO component. It needs to be registered with the office rdb. Also there are additional
types (oletest.rdb in wntmsci7/bin/) which must be merged with the rdb.
VBEventListener is an ActiveX component. The directory contains a Visual Basic Project and also
the binary VBasicEventListener.dll. This is necessary because the dll contains the type library
which is needed by VB. Otherwise VB would generate a new CLSID on a new build.
The dll must be registered on the system. This is done by
regsvr32 VBasicEventListener.dll
or
by a rebuild of the project.

