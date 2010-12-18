import javax.swing.SwingUtilities;
import java.io.InputStream;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.Scriptable;
import org.mozilla.javascript.ImporterTopLevel;
import org.mozilla.javascript.tools.debugger.Main;
import org.mozilla.javascript.tools.debugger.ScopeProvider;

import drafts.com.sun.star.script.framework.runtime.XScriptContext;

public class OORhinoDebugger implements OOScriptDebugger {

    public void go(final XScriptContext xsctxt, String filename) {
        Main sdb = initUI(xsctxt);

        // This is the method we've added to open a file when starting
        // the Rhino debugger
        sdb.openFile(filename);
    }

    public void go(final XScriptContext xsctxt, InputStream in) {
        Main sdb = initUI(xsctxt);

        // Open a stream in the debugger
        sdb.openStream(in);
    }

    // This code is based on the main method of the Rhino Debugger Main class
    // We pass in the XScriptContext in the global scope for script execution
    private Main initUI(final XScriptContext xsctxt) {
        try {
            final Main sdb = new Main("Rhino JavaScript Debugger");
            swingInvoke(new Runnable() {
                    public void run() {
                        sdb.pack();
                        sdb.setSize(640, 640);
                        sdb.setVisible(true);
                    }
                });
            sdb.setExitAction(new Runnable() {
                    public void run() {
                        sdb.dispose();
                    }
                });
            Context.addContextListener(sdb);
            sdb.setScopeProvider(new ScopeProvider() {
                    public Scriptable getScope() {
                        Context ctxt = Context.enter();
                        ImporterTopLevel scope = new ImporterTopLevel(ctxt);
                        Scriptable jsArgs = Context.toObject(xsctxt, scope);
                        scope.put("XSCRIPTCONTEXT", scope, jsArgs);
                        Context.exit();
                        return scope;
                    }
                });
            return sdb;
        } catch (Exception exc) {
            exc.printStackTrace();
        }
        return null;
    }

    static void swingInvoke(Runnable f) {
        if (SwingUtilities.isEventDispatchThread()) {
            f.run();
            return;
        }
        try {
            SwingUtilities.invokeAndWait(f);
        } catch (Exception exc) {
            exc.printStackTrace();
        }
    }
}
