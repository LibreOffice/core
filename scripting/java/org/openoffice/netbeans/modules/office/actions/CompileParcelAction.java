package org.openoffice.netbeans.modules.office.actions;

import org.openide.nodes.Node;
import org.openide.actions.CompileAllAction;

import org.openoffice.netbeans.modules.office.utils.FrameworkJarChecker;

public class CompileParcelAction extends CompileAllAction {
    public String getName() {
        return "Compile";
    }

    protected void performAction(Node[] activatedNodes) {
        FrameworkJarChecker.mountDependencies();
        super.performAction(activatedNodes);
        // FrameworkJarChecker.unmountDependencies();
    }
}
