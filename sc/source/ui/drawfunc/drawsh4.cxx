/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawsh4.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 13:47:10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/drawitem.hxx>
#include <svx/fontwork.hxx>
#include <svx/svdotext.hxx>
#include <svx/xdef.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>

#include "drawsh.hxx"
#include "drawview.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"


//------------------------------------------------------------------

void ScDrawShell::GetFormTextState(SfxItemSet& rSet)
{
    const SdrObject*    pObj        = NULL;
    SvxFontWorkDialog*  pDlg        = NULL;
    ScDrawView*         pDrView     = pViewData->GetScDrawView();
    const SdrMarkList&  rMarkList   = pDrView->GetMarkedObjectList();
    USHORT              nId = SvxFontWorkChildWindow::GetChildWindowId();

    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    if ( pViewFrm->HasChildWindow(nId) )
        pDlg = (SvxFontWorkDialog*)(pViewFrm->GetChildWindow(nId)->GetWindow());

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetObj();

    if ( pObj == NULL || !pObj->ISA(SdrTextObj) ||
        !((SdrTextObj*) pObj)->HasText() )
    {
        if ( pDlg )
            pDlg->SetActive(FALSE);

        rSet.DisableItem(XATTR_FORMTXTSTYLE);
        rSet.DisableItem(XATTR_FORMTXTADJUST);
        rSet.DisableItem(XATTR_FORMTXTDISTANCE);
        rSet.DisableItem(XATTR_FORMTXTSTART);
        rSet.DisableItem(XATTR_FORMTXTMIRROR);
        rSet.DisableItem(XATTR_FORMTXTSTDFORM);
        rSet.DisableItem(XATTR_FORMTXTHIDEFORM);
        rSet.DisableItem(XATTR_FORMTXTOUTLINE);
        rSet.DisableItem(XATTR_FORMTXTSHADOW);
        rSet.DisableItem(XATTR_FORMTXTSHDWCOLOR);
        rSet.DisableItem(XATTR_FORMTXTSHDWXVAL);
        rSet.DisableItem(XATTR_FORMTXTSHDWYVAL);
    }
    else
    {
        if ( pDlg )
        {
            SfxObjectShell* pDocSh = SfxObjectShell::Current();

            if ( pDocSh )
            {
                const SfxPoolItem*  pItem = pDocSh->GetItem( ITEMID_COLOR_TABLE );
                XColorTable*        pColorTable = NULL;

                if ( pItem )
                    pColorTable = ((SvxColorTableItem*)pItem)->GetColorTable();

                pDlg->SetActive();

                if ( pColorTable )
                    pDlg->SetColorTable( pColorTable );
                else
                    { DBG_ERROR( "ColorList not found :-/" ); }
            }
        }
        SfxItemSet aViewAttr(pDrView->GetModel()->GetItemPool());
        pDrView->GetAttributes(aViewAttr);
        rSet.Set(aViewAttr);
    }
}



