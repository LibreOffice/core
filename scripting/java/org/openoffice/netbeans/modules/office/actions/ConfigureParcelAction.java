package org.openoffice.netbeans.modules.office.actions;

import org.openide.nodes.Node;
import org.openide.util.HelpCtx;
import org.openide.util.RequestProcessor;
import org.openide.util.actions.CookieAction;

import org.openoffice.netbeans.modules.office.utils.FrameworkJarChecker;

public class ConfigureParcelAction extends CookieAction {

    public java.lang.String getName() {
        return "Configure";
    }

    protected java.lang.Class[] cookieClasses() {
        return new Class[] {ParcelFolderCookie.class};
    }

    protected int mode() {
        return CookieAction.MODE_EXACTLY_ONE;
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    protected void performAction(final Node[] activatedNodes)
    {
        RequestProcessor.getDefault().post(new Runnable() {
            public void run() {
                FrameworkJarChecker.mountDependencies();
                for (int i = 0; i < activatedNodes.length; i++) {
                    ParcelFolderCookie pfc = (ParcelFolderCookie)
                        activatedNodes[i].getCookie(ParcelFolderCookie.class);
                    if (pfc != null)
                        pfc.configure();
                }
                // FrameworkJarChecker.unmountDependencies();
            }
        });
    }
}
