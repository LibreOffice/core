This macro is used to verify the correct handling of the extension.
Install the extension, copy the code to the basic-ide, run it. You should get two messageboxes listing some interfaces.
This macro only works in conjunction with the sample extension in this directory.
Note that the testtool uses the same macro from framework/tools/input/macros.txt otherwise we would have to implement yet
another way of entering macros to the IDE.




' The sample macro
Sub Main
'test service
o= createUnoService("TestNamesComp")
msgbox o.dbg_supportedInterfaces

'test singleton
ctx = getDefaultContext
factory = ctx.getValueByName("org.openoffice.test.Names")
msgbox o.dbg_supportedInterfaces

End Sub
