/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    }
}
