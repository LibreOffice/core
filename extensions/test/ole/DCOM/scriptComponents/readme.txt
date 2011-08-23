WriterDemo.wsc
=================================================================================
Register the component by right-clicking on the file in the file explorer and select
register.
The component registered as being remotable.That is, it has got an AppID entry (with
a DllSurrogate subkey).That enables us to use dcomcnfg.exe in order to set AccessPermissions,etc.
which is necessary because of the use of the JScript Array object. The automation
bridge will query for IDispatchEx,which is a call from server to client.Hence the server
needs the proper right within the client.

The component implements a run function, which runs the demo example that is written
in JScript.

