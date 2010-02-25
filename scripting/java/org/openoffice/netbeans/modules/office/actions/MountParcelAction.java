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

import org.openide.nodes.Node;
import org.openide.util.HelpCtx;
import org.openide.util.RequestProcessor;
import org.openide.util.actions.CookieAction;

/**
 *
 * @author  adams
 * @version 1.0
 */
public class MountParcelAction extends CookieAction
{
    public MountParcelAction()
    {
    }

    public java.lang.String getName()
    {
        return "Mount Parcel"; //NOI18N
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
        return new Class[] {ParcelCookie.class};
    }

    protected void performAction(final Node[] activatedNodes)
    {
        RequestProcessor.getDefault().post(new Runnable()
        {
            public void run()
            {
                for (int i=0; i<activatedNodes.length; i++)
                {
                    ParcelCookie mpc = (ParcelCookie)activatedNodes[i].getCookie(ParcelCookie.class);
                    if (mpc != null)
                    {
                        mpc.mount();
                    }
                }
            }
        });
    }
}
