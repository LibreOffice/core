/*************************************************************************
 *
 *  $RCSfile: drawvie3.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:30:15 $
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

// INCLUDE ---------------------------------------------------------------

#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>

#include "drawview.hxx"
#include "drwlayer.hxx"
#include "viewdata.hxx"
#include "dbfunc.hxx"
#include "document.hxx"
#include "userdat.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"

void ScIMapDlgSet( const Graphic& rGraphic, const ImageMap* pImageMap,
                    const TargetList* pTargetList, void* pEditingObj );     // imapwrap
USHORT ScIMapChildWindowId();

// STATIC DATA -----------------------------------------------------------

ScDrawView::ScDrawView( OutputDevice* pOut, ScViewData* pData ) :
    FmFormView( pData->GetDocument()->GetDrawLayer(), pOut ),
    pViewData( pData ),
    pDev( pOut ),
    pDoc( pData->GetDocument() ),
    nTab( pData->GetTabNo() ),
    pDropMarker( NULL ),
    pDropMarkObj( NULL ),
    bDisableHdl( FALSE ),
    bInConstruct( TRUE )
{
    Construct();
}

ScDrawView::ScDrawView( OutputDevice* pOut, ScDocument* pDocument, SCTAB nTable ) :
    FmFormView( pDocument->GetDrawLayer(), pOut ),
    pViewData( NULL ),
    pDev( pOut ),
    pDoc( pDocument ),
    nTab( nTable ),
    pDropMarker( NULL ),
    pDropMarkObj( NULL ),
    bDisableHdl( FALSE ),
    bInConstruct( TRUE )
{
    Construct();
}

// Verankerung setzen

void ScDrawView::SetAnchor( ScAnchorType eType )
{
    SdrObject* pObj = NULL;
    if( AreObjectsMarked() )
    {
        const SdrMarkList* pMark = &GetMarkedObjectList();
        ULONG nCount = pMark->GetMarkCount();
        for( ULONG i=0; i<nCount; i++ )
        {
            pObj = pMark->GetMark(i)->GetObj();
            ScDrawLayer::SetAnchor( pObj, eType );
        }

        if ( pViewData )
            pViewData->GetDocShell()->SetDrawModified();
    }
}

ScAnchorType ScDrawView::GetAnchor() const
{
    BOOL bPage = FALSE;
    BOOL bCell = FALSE;
    const SdrObject* pObj = NULL;
    if( AreObjectsMarked() )
    {
        const SdrMarkList* pMark = &GetMarkedObjectList();
        ULONG nCount = pMark->GetMarkCount();
        Point p0;
        for( ULONG i=0; i<nCount; i++ )
        {
            pObj = pMark->GetMark(i)->GetObj();
            if( ScDrawLayer::GetAnchor( pObj ) == SCA_CELL )
                bCell =TRUE;
            else
                bPage = TRUE;
        }
    }
    if( bPage && !bCell )
        return SCA_PAGE;
    if( !bPage && bCell )
        return SCA_CELL;
    return SCA_DONTKNOW;
}

void __EXPORT ScDrawView::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                     const SfxHint& rHint, const TypeId& rHintType )
{
    if (rHint.ISA(ScTabDeletedHint))                        // Tabelle geloescht
    {
        SCTAB nDelTab = ((ScTabDeletedHint&)rHint).GetTab();
        if (ValidTab(nDelTab))
            HidePagePgNum(static_cast<sal_uInt16>(nDelTab));
    }
    else if (rHint.ISA(ScTabSizeChangedHint))               // Groesse geaendert
    {
        if ( nTab == ((ScTabSizeChangedHint&)rHint).GetTab() )
            UpdateWorkArea();
    }
    else
        FmFormView::SFX_NOTIFY( rBC,rBCType,rHint,rHintType );
}

void ScDrawView::UpdateIMap( SdrObject* pObj )
{
    if ( pViewData &&
         pViewData->GetViewShell()->GetViewFrame()->HasChildWindow( ScIMapChildWindowId() ) &&
         pObj && ( pObj->ISA(SdrGrafObj) || pObj->ISA(SdrOle2Obj) ) )
    {
        Graphic     aGraphic;
        TargetList  aTargetList;
        ScIMapInfo* pIMapInfo = ScDrawLayer::GetIMapInfo( pObj );
        const ImageMap* pImageMap = NULL;
        if ( pIMapInfo )
            pImageMap = &pIMapInfo->GetImageMap();

        // Target-Liste besorgen
        pViewData->GetViewShell()->GetViewFrame()->GetTargetList( aTargetList );

        // Grafik vom Objekt besorgen
        if ( pObj->ISA( SdrGrafObj ) )
            aGraphic = ( (SdrGrafObj*) pObj )->GetGraphic();
        else
            aGraphic = ScDrawLayer::GetGraphicFromOle2Obj(
                        (const SdrOle2Obj*) pObj );

        ScIMapDlgSet( aGraphic, pImageMap, &aTargetList, pObj );    // aus imapwrap

        // TargetListe kann von uns wieder geloescht werden
        String* pEntry = aTargetList.First();
        while( pEntry )
        {
            delete pEntry;
            pEntry = aTargetList.Next();
        }
    }
}




