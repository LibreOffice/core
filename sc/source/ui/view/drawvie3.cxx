/*************************************************************************
 *
 *  $RCSfile: drawvie3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:08 $
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

//sfxitems.hxx
//#define _SFXINTITEM_HXX
//#define _SFXENUMITEM_HXX
#define _SFXFLAGITEM_HXX
//#define _SFXSTRITEM_HXX
#define _SFXPTITEM_HXX
#define _SFXRECTITEM_HXX
#define _AEITEM_HXX
#define _SFXRNGITEM_HXX
#define _SFXSLSTITM_HXX

//sfx.hxx
//#define _SFX_HXX ***
#define _SFXAPPWIN_HXX
#define _SFX_SAVEOPT_HXX
//#define _SFX_CHILDWIN_HXX ***
#define _SFXCTRLITEM_HXX
#define _SFX_PRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSW_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX

//sfxdlg.hxx
//#define _SFXDLG_HXX ***
//#define _SFXTABDLG_HXX ***
#define _BASEDLGS_HXX
#define _DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
#define _SFXDOCTEMPL_HXX
#define _SFXDOCTDLG_HXX
//#define _SFXDOCKWIN_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXNEW_HXX
#define _SFXDOCMAN_HXX

//sfxsh.hxx
//#define _SFXSH_HXX
//#define _SFX_SHELL_HXX
//#define _SFXAPP_HXX
//#define _SFX_BINDINGS_HXX
#define _SFXDISPATCH_HXX
#define _SFXMSG_HXX
#define _SFXOBJFACE_HXX
#define _SFXREQUEST_HXX
#define _SFXMACRO_HXX

//sfxdoc.hxx
//#define _SFXDOC_HXX ***
//#define _SFX_OBJSH_HXX ***
//#define _SFX_CLIENTSH_HXX ***
#define _SFX_DOCFILT_HACK_HXX
#define _SFX_FCONTNR_HXX
#define _SFXDOCFILE_HXX
//#define _SFXFRAME_HXX ***
//#define _SFXVIEWFRM_HXX ***
//#define _SFXVIEWSH_HXX ***
#define _MDIFRM_HXX
#define _SFX_IPFRM_HXX
#define _SFX_INTERNO_HXX
#define _FSETOBJSH_HXX

//si.hxx
//#define _SI_HXX ***
#define _SIDLL_HXX
#define _VCATTR_HXX
#define _VCONT_HXX
//#define _VCSBX_HXX ???
//#define _SI_NOVCSBXFORM ***
#define SI_NOOTHERFORMS
#define _SI_NOVCSBXBUTTON
#define _SI_NOVCSBXCHECKBOX
#define _SI_NOVCSBXRADIOBUTTON
#define _SI_NOVCSBXPUSHBUTTON
#define _SI_NOVCSBXSPINBUTTON
#define _SI_NOVCSBXFIXEDTEXT
#define _SI_NOVCSBXGROUPBOX
#define _SI_NOVCSBXLISTBOX
#define _SI_NOVCSBXCOMBOBOX
#define _SI_NOVCSBXEDIT
//#define _SI_NOVCSBXSCROLLBAR ***
#define _SI_NOVCSBXHSCROLLBAR
#define _SI_BOVCSBXVSCROLLBAR
#define _SI_NOVCSBXPREVIEW
#define _SI_NOVCSBXURLBUTTON
#define _VCTRLS_HXX
//#define _VCDRWOBJ_HXX ***
//#define _SI_NOVCDRAWMODEL
#define _SI_NOVCDRAWPAGE
//#define _SI_NOVCDRAWVIEW ***
#define _VCBRW_HXX
#define _VCFORM_HXX

//svdrwobt.hxx
//#define _SVDOATTR_HXX
//#define _SVDOTEXT_HXX
#define _SVDRAW_HXX

//svdraw.hxx
#define _SVDRAW_HXX
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS
#define _SDR_NOVIEWS
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
#define _SDR_NOUNDO
#define _SDR_NOXOUTDEV

//svdrwobx.hxx
//#define _SVDRWOBX_HXX ***
#define _SVDOUKWN_HXX
//#define _SVDORECT_HXX ***
#define _SVDCAPT_HXX
#define _SVDOCIRC_HXX
//#define _SVDOEDGE_HXX
//#define _SVDOGRAF_HXX
#define _SVDOGRP_HXX
#define _SVDOMEAS_HXX
//#define _SVDOOLE2_HXX ***
#define _SVDOPAGE_HXX
#define _SVDOPATH_HXX

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

ScDrawView::ScDrawView( OutputDevice* pOut, ScDocument* pDocument, USHORT nTable ) :
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
    if( HasMarkedObj() )
    {
        const SdrMarkList* pMark = &GetMarkList();
        ULONG nCount = pMark->GetMarkCount();
        for( ULONG i=0; i<nCount; i++ )
        {
            pObj = pMark->GetMark(i)->GetObj();
            ScDrawLayer::SetAnchor( pObj, eType );
        }
    }
}

ScAnchorType ScDrawView::GetAnchor() const
{
    BOOL bPage = FALSE;
    BOOL bCell = FALSE;
    const SdrObject* pObj = NULL;
    if( HasMarkedObj() )
    {
        const SdrMarkList* pMark = &GetMarkList();
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
        USHORT nDelTab = ((ScTabDeletedHint&)rHint).GetTab();
        if (nDelTab <= MAXTAB)
            HidePagePgNum(nDelTab);
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




