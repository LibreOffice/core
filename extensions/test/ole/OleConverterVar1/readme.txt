The program test the Service com.sun.star.bridge.OleBridgeSupplierVar1.
While running in debug mode it writes infos to the debug - output.

The executable needs a applicat.rdb present with the registered services
com.sun.star.bridge.OleBridgeSupplierVar1, oletest.OleTest. The OleTest 
component resides in extensions/test/ole/cpnt. The build creates an 
oletest.rdb file that must be merged with the applicat.rdb and OleTest
must be registered to the applicat.rdb.