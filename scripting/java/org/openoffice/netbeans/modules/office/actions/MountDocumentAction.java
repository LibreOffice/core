/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MountDocumentAction.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:10:59 $
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
