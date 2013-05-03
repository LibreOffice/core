/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
            ArrayList v = new ArrayList(1);
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
    }
}
