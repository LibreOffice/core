/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BuildParcelAction.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:09:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
