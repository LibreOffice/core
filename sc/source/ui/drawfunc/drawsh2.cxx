/*************************************************************************
 *
 *  $RCSfile: drawsh2.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:27:07 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#include <svx/sizeitem.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xdef.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/ptitem.hxx>
#include <svtools/whiter.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdouno.hxx>
#ifndef _SVX_EXTRUSION_BAR_HXX
#include <svx/extrusionbar.hxx>
#endif

#include "drawsh.hxx"
#include "drawview.hxx"
#include "viewdata.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"


#ifndef _IPOBJ_HXX
#include <so3/ipobj.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif

USHORT ScGetFontWorkId();       // in drtxtob

//------------------------------------------------------------------

ScDrawShell::ScDrawShell( ScViewData* pData ) :
    SfxShell(pData->GetViewShell()),
    pViewData( pData )
{
    SetPool( &pViewData->GetScDrawView()->GetModel()->GetItemPool() );
    SetUndoManager( pViewData->GetSfxDocShell()->GetUndoManager() );
    SetHelpId( HID_SCSHELL_DRAWSH );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Drawing")));
}

ScDrawShell::~ScDrawShell()
{
}

void ScDrawShell::GetState( SfxItemSet& rSet )          // Zustaende / Toggles
{
    ScDrawView* pView    = pViewData->GetScDrawView();
    SdrDragMode eMode    = pView->GetDragMode();

    rSet.Put( SfxBoolItem( SID_OBJECT_ROTATE, eMode == SDRDRAG_ROTATE ) );
    rSet.Put( SfxBoolItem( SID_OBJECT_MIRROR, eMode == SDRDRAG_MIRROR ) );
    rSet.Put( SfxBoolItem( SID_BEZIER_EDIT, !pView->IsFrameDragSingles() ) );

    USHORT nFWId = ScGetFontWorkId();
    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    rSet.Put(SfxBoolItem(SID_FONTWORK, pViewFrm->HasChildWindow(nFWId)));

    switch( pView->GetAnchor() )
    {
        case SCA_PAGE:
        rSet.Put( SfxBoolItem( SID_ANCHOR_PAGE, TRUE ) );
        rSet.Put( SfxBoolItem( SID_ANCHOR_CELL, FALSE ) );
        break;

        case SCA_CELL:
        rSet.Put( SfxBoolItem( SID_ANCHOR_PAGE, FALSE ) );
        rSet.Put( SfxBoolItem( SID_ANCHOR_CELL, TRUE ) );
        break;

        default:
        rSet.Put( SfxBoolItem( SID_ANCHOR_PAGE, FALSE ) );
        rSet.Put( SfxBoolItem( SID_ANCHOR_CELL, FALSE ) );
        break;
    }
}

void ScDrawShell::GetDrawFuncState( SfxItemSet& rSet )      // Funktionen disablen
{
    ScDrawView* pView = pViewData->GetScDrawView();

    //  #111711# call IsMirrorAllowed first to make sure ForcePossibilities (and thus CheckMarked)
    //  is called before GetMarkCount, so the nMarkCount value is valid for the rest of this method.
    if (!pView->IsMirrorAllowed(TRUE,TRUE))
    {
        rSet.DisableItem( SID_MIRROR_HORIZONTAL );
        rSet.DisableItem( SID_MIRROR_VERTICAL );
    }

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    ULONG nMarkCount = rMarkList.GetMarkCount();

    if ( nMarkCount <= 1 || !pView->IsGroupPossible() )
        rSet.DisableItem( SID_GROUP );
    if ( nMarkCount == 0 || !pView->IsUnGroupPossible() )
        rSet.DisableItem( SID_UNGROUP );
    if ( nMarkCount != 1 || !pView->IsGroupEnterPossible() )
        rSet.DisableItem( SID_ENTER_GROUP );
    if ( !pView->IsGroupEntered() )
        rSet.DisableItem( SID_LEAVE_GROUP );

    if ( nMarkCount <= 1 )                      // nichts oder nur ein Objekt selektiert
    {
            //  Ausrichtung
        rSet.DisableItem( SID_OBJECT_ALIGN_LEFT );      // keine Ausrichtung an der Seite
        rSet.DisableItem( SID_OBJECT_ALIGN_CENTER );
        rSet.DisableItem( SID_OBJECT_ALIGN_RIGHT );
        rSet.DisableItem( SID_OBJECT_ALIGN_UP );
        rSet.DisableItem( SID_OBJECT_ALIGN_MIDDLE );
        rSet.DisableItem( SID_OBJECT_ALIGN_DOWN );
    }

    if ( !nMarkCount || pView->HasMarkedControl() )
    {
        //  Layer von Controls darf nicht veraendert werden
        rSet.DisableItem( SID_OBJECT_HEAVEN );
        rSet.DisableItem( SID_OBJECT_HELL );
    }
    else
    {
        if(AreAllObjectsOnLayer(SC_LAYER_FRONT,rMarkList))
        {
            rSet.DisableItem( SID_OBJECT_HEAVEN );
        }
        else if(AreAllObjectsOnLayer(SC_LAYER_BACK,rMarkList))
        {
            rSet.DisableItem( SID_OBJECT_HELL );
        }
    }

    BOOL bCanRename = FALSE;
    if ( nMarkCount == 1 )
    {
        UINT16 nObjType = rMarkList.GetMark( 0 )->GetObj()->GetObjIdentifier();
        if ( nObjType == OBJ_OLE2 || nObjType == OBJ_GRAF || nObjType == OBJ_GRUP )
            bCanRename = TRUE;

        // #91929#; don't show original size entry if not possible
        if ( nObjType == OBJ_OLE2 )
        {
            SdrOle2Obj* pOleObj = static_cast<SdrOle2Obj*>(rMarkList.GetMark( 0 )->GetObj());
            if (pOleObj->GetObjRef().Is() && ((pOleObj->GetObjRef()->GetMiscStatus() & SVOBJ_MISCSTATUS_SERVERRESIZE) == SVOBJ_MISCSTATUS_SERVERRESIZE))
                rSet.DisableItem(SID_ORIGINALSIZE);
        }
    }
    if ( !bCanRename )
        rSet.DisableItem( SID_RENAME_OBJECT );

    if ( !nMarkCount )                          // nichts selektiert
    {
            //  Anordnung
        rSet.DisableItem( SID_FRAME_UP );
        rSet.DisableItem( SID_FRAME_DOWN );
        rSet.DisableItem( SID_FRAME_TO_TOP );
        rSet.DisableItem( SID_FRAME_TO_BOTTOM );
            //  Clipboard / loeschen
        rSet.DisableItem( SID_DELETE );
        rSet.DisableItem( SID_DELETE_CONTENTS );
        rSet.DisableItem( SID_CUT );
        rSet.DisableItem( SID_COPY );
            //  sonstiges
        rSet.DisableItem( SID_ANCHOR_TOGGLE );
        rSet.DisableItem( SID_ORIGINALSIZE );
        rSet.DisableItem( SID_ATTR_TRANSFORM );
    }

    if ( rSet.GetItemState( SID_ENABLE_HYPHENATION ) != SFX_ITEM_UNKNOWN )
    {
        SfxItemSet aAttrs( pView->GetModel()->GetItemPool() );
        pView->GetAttributes( aAttrs );
        if( aAttrs.GetItemState( EE_PARA_HYPHENATE ) >= SFX_ITEM_AVAILABLE )
        {
            BOOL bValue = ( (const SfxBoolItem&) aAttrs.Get( EE_PARA_HYPHENATE ) ).GetValue();
            rSet.Put( SfxBoolItem( SID_ENABLE_HYPHENATION, bValue ) );
        }
    }

    svx::ExtrusionBar::getState( pView, rSet );
}

//
//          Attribute fuer Drawing-Objekte
//

void ScDrawShell::GetDrawAttrState( SfxItemSet& rSet )
{
    Point       aMousePos   = pViewData->GetMousePosPixel();
    Window*     pWindow     = pViewData->GetActiveWin();
    ScDrawView* pDrView     = pViewData->GetScDrawView();
    Point       aPos        = pWindow->PixelToLogic(aMousePos);
    BOOL        bHasMarked  = pDrView->AreObjectsMarked();

    if( bHasMarked )
    {
        rSet.Put( pDrView->GetAttrFromMarked(FALSE) );

        // Wenn die View selektierte Objekte besitzt, muessen entspr. Items
        // von SFX_ITEM_DEFAULT (_ON) auf SFX_ITEM_DISABLED geaendert werden

        SfxWhichIter aIter( rSet, XATTR_LINE_FIRST, XATTR_FILL_LAST );
        USHORT nWhich = aIter.FirstWhich();
        while( nWhich )
        {
            if( SFX_ITEM_DEFAULT == rSet.GetItemState( nWhich ) )
                rSet.DisableItem( nWhich );

            nWhich = aIter.NextWhich();
        }
    }
    else
        rSet.Put( pDrView->GetDefaultAttr() );

    //  Items for position and size (see ScGridWindow::UpdateStatusPosSize, #108137#)

    //  SvxPosSizeStatusBarControl needs a SvxSizeItem also in SID_TABLE_CELL (in conflict
    //  with the sdi definition), because an invalid item state or unknown item type would
    //  enable date/time display, and a SfxStringItem would show the text!

    BOOL bActionItem = FALSE;
    if ( pDrView->IsAction() )              // action rectangle
    {
        Rectangle aRect;
        pDrView->TakeActionRect( aRect );
        if ( !aRect.IsEmpty() )
        {
            pDrView->GetPageViewPvNum(0)->LogicToPagePos(aRect);
            rSet.Put( SfxPointItem( SID_ATTR_POSITION, aRect.TopLeft() ) );
            Size aSize( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() );
            rSet.Put( SvxSizeItem( SID_ATTR_SIZE, aSize ) );
            rSet.Put( SvxSizeItem( SID_TABLE_CELL, aSize ) );
            bActionItem = TRUE;
        }
    }
    if ( !bActionItem )
    {
        if ( pDrView->AreObjectsMarked() )      // selected objects
        {
            Rectangle aRect = pDrView->GetAllMarkedRect();
            pDrView->GetPageViewPvNum(0)->LogicToPagePos(aRect);
            rSet.Put( SfxPointItem( SID_ATTR_POSITION, aRect.TopLeft() ) );
            Size aSize( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() );
            rSet.Put( SvxSizeItem( SID_ATTR_SIZE, aSize ) );
            rSet.Put( SvxSizeItem( SID_TABLE_CELL, aSize ) );
        }
        else                                // mouse position
        {
            // aPos is initialized above
            pDrView->GetPageViewPvNum(0)->LogicToPagePos(aPos);
            rSet.Put( SfxPointItem( SID_ATTR_POSITION, aPos ) );
            rSet.Put( SvxSizeItem( SID_ATTR_SIZE, Size( 0, 0 ) ) );
            rSet.Put( SvxSizeItem( SID_TABLE_CELL, Size( 0, 0 ) ) );
        }
    }
}

void ScDrawShell::GetAttrFuncState(SfxItemSet &rSet)
{
    //  Dialoge fuer Draw-Attribute disablen, wenn noetig

    ScDrawView* pDrView = pViewData->GetScDrawView();
    SfxItemSet aViewSet = pDrView->GetAttrFromMarked(FALSE);

    if ( aViewSet.GetItemState( XATTR_LINESTYLE ) == SFX_ITEM_DEFAULT )
    {
        rSet.DisableItem( SID_ATTRIBUTES_LINE );
        rSet.DisableItem( SID_ATTR_LINEEND_STYLE );     // Tbx-Controller
    }

    if ( aViewSet.GetItemState( XATTR_FILLSTYLE ) == SFX_ITEM_DEFAULT )
        rSet.DisableItem( SID_ATTRIBUTES_AREA );
}

BOOL ScDrawShell::AreAllObjectsOnLayer(USHORT nLayerNo,const SdrMarkList& rMark)
{
    BOOL bResult=TRUE;
    ULONG nCount = rMark.GetMarkCount();
    for (ULONG i=0; i<nCount; i++)
    {
        SdrObject* pObj = rMark.GetMark(i)->GetObj();
        if ( !pObj->ISA(SdrUnoObj) )
        {
            if(nLayerNo!=pObj->GetLayer())
            {
                bResult=FALSE;
                break;
            }
        }
    }
    return bResult;
}


