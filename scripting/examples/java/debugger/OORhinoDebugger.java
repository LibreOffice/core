import drafts.com.sun.star.script.framework.runtime.XScriptContext;
import javax.swing.SwingUtilities;
import org.mozilla.javascript.*;
import org.mozilla.javascript.tools.debugger.*;

public class OORhinoDebugger implements OOScriptDebugger {

    public void go(final XScriptContext xsctxt, String filename) {
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
            sdb.openFile(filename);
        } catch (Exception exc) {
            exc.printStackTrace();
        }
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
