/*************************************************************************
 *
 *  $RCSfile: fuline.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:03:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include "fuline.hxx"

#include <svx/svxids.hrc>
#ifndef _SVX_TAB_LINE_HXX //autogen
#include <svx/tabline.hxx>
#endif
#ifndef _XENUM_HXX //autogen
#include <svx/xenum.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _XDEF_HXX //autogen
#include <svx/xdef.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#include "app.hrc"
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001

namespace sd {

TYPEINIT1( FuLine, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuLine::FuLine (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    BOOL        bHasMarked = pView->AreObjectsMarked();

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        // erst einmal alle eingabeparameter fuer den dialog retten
        SfxItemSet aInputAttr( pDoc->GetPool() );
        pView->GetAttributes( aInputAttr );

        const XLineStyleItem &rILineStyleItem = (const XLineStyleItem &) aInputAttr.Get (XATTR_LINESTYLE);
        XLineStyle           eILineStyle      = rILineStyleItem.GetValue ();

        const XLineDashItem  &rILineDashItem  = (const XLineDashItem &) aInputAttr.Get (XATTR_LINEDASH);
        const XDash          &rIDash          = rILineDashItem.GetValue ();

        const XLineWidthItem &rILineWidthItem = (const XLineWidthItem &) aInputAttr.Get (XATTR_LINEWIDTH);
        long                 nILineWidth      = rILineWidthItem.GetValue ();

        const XLineColorItem &rILineColorItem = (const XLineColorItem &) aInputAttr.Get (XATTR_LINECOLOR);
        const Color          &rIColor         = rILineColorItem.GetValue ();

        const SdrObject* pObj = NULL;
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        if( rMarkList.GetMarkCount() == 1 )
            pObj = rMarkList.GetMark(0)->GetObj();

        SfxItemSet* pNewAttr = new SfxItemSet( pDoc->GetPool() );
        pView->GetAttributes( *pNewAttr );

        //CHINA001 SvxLineTabDialog* pDlg = new SvxLineTabDialog( NULL, pNewAttr, pDoc, pObj, bHasMarked );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet Factory fail!");//CHINA001
        SfxAbstractTabDialog * pDlg = pFact->CreateSvxLineTabDialog(NULL,
                    pNewAttr,
                    pDoc,
                ResId(RID_SVXDLG_LINE),
                pObj,
                bHasMarked);
        DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
        if ( pDlg->Execute() == RET_OK )
        {
            // die ausgabeparameter des dialogs bestimmen
            SfxItemSet           *pOutputAttr     = (SfxItemSet *) pDlg->GetOutputItemSet();

            const XLineStyleItem &rOLineStyleItem = (const XLineStyleItem &) pOutputAttr->Get (XATTR_LINESTYLE);
            XLineStyle           eOLineStyle      = rOLineStyleItem.GetValue ();

            const XLineDashItem  &rOLineDashItem  = (const XLineDashItem &) pOutputAttr->Get (XATTR_LINEDASH);
            const XDash          &rODash          = rOLineDashItem.GetValue ();

            const XLineWidthItem &rOLineWidthItem = (const XLineWidthItem &) pOutputAttr->Get (XATTR_LINEWIDTH);
            long                 nOLineWidth      = rOLineWidthItem.GetValue ();

            const XLineColorItem &rOLineColorItem = (const XLineColorItem &) pOutputAttr->Get (XATTR_LINECOLOR);
            const Color          &rOColor         = rOLineColorItem.GetValue ();

            pView->SetAttributes (*(pDlg->GetOutputItemSet ()));
        }

        // Attribute wurden geaendert, Listboxes in Objectbars muessen aktualisiert werden
        static USHORT SidArray[] = {
                        SID_ATTR_LINE_STYLE,
                        SID_ATTR_LINE_DASH,
                        SID_ATTR_LINE_WIDTH,
                        SID_ATTR_LINE_COLOR,
                        0 };

        pViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );

        delete pDlg;
        delete pNewAttr;
    }

    rReq.Ignore ();
}

} // end of namespace sd
