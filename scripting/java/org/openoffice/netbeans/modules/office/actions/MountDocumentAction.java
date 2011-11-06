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

import org.openide.nodes.Node;
import org.openide.util.HelpCtx;
import org.openide.util.RequestProcessor;
import org.openide.util.actions.CookieAction;

/**
 *
 * @author  adams
 * @version 1.0
 */
public class MountDocumentAction extends CookieAction
{
    public MountDocumentAction()
    {
    }

    public java.lang.String getName()
    {
        return "Mount Document"; //NOI18N
    }

    public HelpCtx getHelpCtx()
    {
        return HelpCtx.DEFAULT_HELP;
    }

    protected int mode()
    {
        // enable duplication for as many qualifying nodes as are selected:
        return CookieAction.MODE_ALL;
    }

    protected java.lang.Class[] cookieClasses()
    {
        // just the DuplicateCookie:
        return new Class[] {OfficeDocumentCookie.class};
    }

    protected void performAction(final Node[] activatedNodes)
    {
        RequestProcessor.getDefault().post(new Runnable()
        {
            public void run()
            {
                for (int i=0; i<activatedNodes.length; i++)
                {
                    OfficeDocumentCookie cookie = (OfficeDocumentCookie)activatedNodes[i].getCookie(OfficeDocumentCookie.class);
                    if (cookie != null)
                    {
                        cookie.mount();
                    }
                }
            }
        });
    }
}
