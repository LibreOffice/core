importClass(Packages.com.sun.star.uno.UnoRuntime);
importClass(Packages.com.sun.star.lang.XMultiComponentFactory);
importClass(Packages.com.sun.star.awt.XDialogProvider);
importClass(Packages.com.sun.star.awt.XDialog);
importClass(Packages.com.sun.star.uno.Exception);
importClass(Packages.drafts.com.sun.star.script.provider.XScriptContext);

importClass(java.lang.Thread);
importClass(java.lang.System);

function getDialogProvider()
{
    // UNO awt components of the Highlight dialog
    xmcf = XSCRIPTCONTEXT.getComponentContext().getServiceManager();

    args = new Array;
    args[0] = XSCRIPTCONTEXT.getDocument();

    try {
        obj = xmcf.createInstanceWithArgumentsAndContext(
            "com.sun.star.awt.DialogProvider", args,
            XSCRIPTCONTEXT.getComponentContext());
    }
    catch (e) {
        System.err.println("Error getting DialogProvider object");
        return null;
    }

    return UnoRuntime.queryInterface(XDialogProvider, obj);
}

xDialogProvider = getDialogProvider();

if (xDialogProvider != null)
{
    try {
        findDialog = xDialogProvider.createDialog("vnd.sun.star.script:" +
            "ScriptBindingLibrary.Highlight?location=application");
        findDialog.execute();
    }
    catch (e) {
        System.err.println("Got exception on first creating dialog: " +
            e.getMessage());
    }
}
