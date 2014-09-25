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

import org.openide.nodes.Node;
import org.openide.util.HelpCtx;
import org.openide.util.RequestProcessor;
import org.openide.util.actions.CookieAction;

import org.openoffice.netbeans.modules.office.utils.FrameworkJarChecker;

public class ConfigureParcelAction extends CookieAction {

    public String getName() {
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

    protected void performAction(final Node[] activatedNodes) {
        FrameworkJarChecker.mountDependencies();

        RequestProcessor.getDefault().post(new Runnable() {
            public void run() {
                for (int i = 0; i < activatedNodes.length; i++) {
                    ParcelFolderCookie pfc = (ParcelFolderCookie)
                                             activatedNodes[i].getCookie(ParcelFolderCookie.class);

                    if (pfc != null)
                        pfc.configure();
                }
            }
        });
    }
}
