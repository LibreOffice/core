/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
