package org.openoffice.netbeans.modules.office.actions;

import java.util.Vector;
import java.util.Enumeration;

import org.openide.nodes.Node;
import org.openide.util.HelpCtx;
import org.openide.util.RequestProcessor;
import org.openide.actions.BuildAllAction;

import org.openide.compiler.Compiler;
import org.openide.compiler.CompilerJob;
import org.openide.compiler.CompilerTask;

import org.openoffice.netbeans.modules.office.utils.FrameworkJarChecker;

public class BuildParcelAction extends BuildAllAction {
    public String getName() {
        return "Build";
    }

    protected void performAction(Node[] activatedNodes) {
        FrameworkJarChecker.mountDependencies();

        for (int i = 0; i < activatedNodes.length; i++) {
            Vector v = new Vector(1);
            v.addElement(activatedNodes[i]);

            CompilerJob job = createJob(v.elements(), Compiler.DEPTH_INFINITE);
            CompilerTask task = job.start();
            task.waitFinished();

            if (task.isSuccessful()) {
                ParcelFolderCookie cookie = (ParcelFolderCookie)
                    activatedNodes[i].getCookie(ParcelFolderCookie.class);

                if (cookie != null)
                    cookie.generate();
            }
        }
        // FrameworkJarChecker.unmountDependencies();
    }
}
