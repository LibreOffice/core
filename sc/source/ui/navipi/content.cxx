/*************************************************************************
 *
 *  $RCSfile: content.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:55:47 $
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

// TOOLS
#define _BIGINT_HXX
#define _SFXMULTISEL_HXX
#define _STACK_HXX
#define _QUEUE_HXX
#define _DYNARR_HXX
#define _TREELIST_HXX
#define _CACHESTR_HXX
#define _NEW_HXX
//#define _SHL_HXX
//#define _LINK_HXX
//#define _ERRCODE_HXX
//#define _GEN_HXX
//#define _FRACT_HXX
//#define _STRING_HXX
//#define _MTF_HXX
//#define _CONTNR_HXX
//#define _LIST_HXX
//#define _TABLE_HXX
#define _DYNARY_HXX
//#define _UNQIDX_HXX
//#define _SVMEMPOOL_HXX
//#define _UNQID_HXX
//#define _DEBUG_HXX
//#define _DATE_HXX
//#define _TIME_HXX
//#define _DATETIME_HXX
//#define _INTN_HXX
//#define _WLDCRD_HXX
//#define _FSYS_HXX
//#define _STREAM_HXX
#define _CACHESTR_HXX
#define _SV_MULTISEL_HXX

//SV
//#define _CLIP_HXX ***
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX
//#define _COLOR_HXX
//#define _PAL_HXX
//#define _BITMAP_HXX
//#define _GDIOBJ_HXX
//#define _POINTR_HXX
//#define _ICON_HXX
//#define _IMAGE_HXX
//#define _KEYCOD_HXX
//#define _EVENT_HXX
//#define _HELP_HXX
//#define _APP_HXX
//#define _MDIAPP_HXX
//#define _TIMER_HXX
//#define _METRIC_HXX
//#define _REGION_HXX
//#define _OUTDEV_HXX
//#define _SYSTEM_HXX
//#define _VIRDEV_HXX
//#define _JOBSET_HXX
//#define _PRINT_HXX
//#define _WINDOW_HXX
//#define _SYSWIN_HXX
//#define _WRKWIN_HXX
#define _MDIWIN_HXX
//#define _FLOATWIN_HXX
//#define _DOCKWIN_HXX
//#define _CTRL_HXX
//#define _SCRBAR_HXX
//#define _BUTTON_HXX
//#define _IMAGEBTN_HXX
//#define _FIXED_HXX
//#define _GROUP_HXX
//#define _EDIT_HXX
//#define _COMBOBOX_HXX
//#define _LSTBOX_HXX
//#define _SELENG_HXX ***
//#define _SPLIT_HXX
#define _SPIN_HXX
//#define _FIELD_HXX
//#define _MOREBTN_HXX ***
//#define _TOOLBOX_HXX
//#define _STATUS_HXX ***
//#define _DIALOG_HXX
//#define _MSGBOX_HXX
//#define _SYSDLG_HXX
//#define _FILDLG_HXX
//#define _PRNDLG_HXX
#define _COLDLG_HXX
//#define _TABDLG_HXX
//#define _MENU_HXX
//#define _GDIMTF_HXX
//#define _POLY_HXX
//#define _ACCEL_HXX
//#define _GRAPH_HXX
//#define _SOUND_HXX

#if defined  WIN
#define _MENUBTN_HXX
#endif

//svtools
#define _SCRWIN_HXX
#define _RULER_HXX
//#define _TABBAR_HXX
//#define _VALUESET_HXX
#define _STDMENU_HXX
//#define _STDCTRL_HXX
//#define _CTRLBOX_HXX
#define _CTRLTOOL_HXX
#define _EXTATTR_HXX
#define _FRM3D_HXX
#define _EXTATTR_HXX

//SVTOOLS
//#define _SVTREELIST_HXX ***
#define _FILTER_HXX
//#define _SVLBOXITM_HXX ***
//#define _SVTREEBOX_HXX ***
#define _SVICNVW_HXX
#define _SVTABBX_HXX

//sfxcore.hxx
//#define _SFXINIMGR_HXX ***
//#define _SFXCFGITEM_HXX
//#define _SFX_PRINTER_HXX
#define _SFXGENLINK_HXX
#define _SFXHINTPOST_HXX
//#define _SFXDOCINF_HXX
#define _SFXLINKHDL_HXX
//#define _SFX_PROGRESS_HXX

//sfxsh.hxx
//#define _SFX_SHELL_HXX
//#define _SFXAPP_HXX
//#define _SFXDISPATCH_HXX
//#define _SFXMSG_HXX ***
//#define _SFXOBJFACE_HXX ***
//#define _SFXREQUEST_HXX
#define _SFXMACRO_HXX

// SFX
//#define _SFXAPPWIN_HXX ***
#define _SFX_SAVEOPT_HXX
//#define _SFX_CHILDWIN_HXX
//#define _SFXCTRLITEM_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX

//sfxdoc.hxx
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX
//#define _SFXDOCINF_HXX
//#define _SFX_OBJFAC_HXX
#define _SFX_DOCFILT_HXX
//#define _SFXDOCFILE_HXX ***
//define _VIEWFAC_HXX
//#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX
//#define _MDIFRM_HXX ***
#define _SFX_IPFRM_HXX
//#define _SFX_INTERNO_HXX

//sfxdlg.hxx
//#define _SFXTABDLG_HXX
//#define _BASEDLGS_HXX ***
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
//#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
//#define _SFXDOCTEMPL_HXX ***
//#define _SFXDOCTDLG_HXX ***
//#define _SFX_TEMPLDLG_HXX ***
//#define _SFXNEW_HXX ***
#define _SFXDOCMAN_HXX
//#define _SFXDOCKWIN_HXX

//sfxitems.hxx
#define _SFX_WHMAP_HXX
//#define _ARGS_HXX //*
//#define _SFXPOOLITEM_HXX
//#define _SFXINTITEM_HXX
//#define _SFXENUMITEM_HXX
#define _SFXFLAGITEM_HXX
//#define _SFXSTRITEM_HXX
#define _SFXPTITEM_HXX
#define _SFXRECTITEM_HXX
//#define _SFXITEMPOOL_HXX
//#define _SFXITEMSET_HXX
#define _SFXITEMITER_HXX
#define _SFX_WHITER_HXX
#define _SFXPOOLCACH_HXX
//#define _AEITEM_HXX
#define _SFXRNGITEM_HXX
//#define _SFXSLSTITM_HXX
//#define _SFXSTYLE_HXX

//xout.hxx
//#define _XENUM_HXX
//#define _XPOLY_HXX
//#define _XATTR_HXX
//#define _XOUTX_HXX
//#define _XPOOL_HXX
//#define _XTABLE_HXX

//svdraw.hxx
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS
//#define _SDR_NOVIEWS ***
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
//#define _SDR_NOUNDO
#define _SDR_NOXOUTDEV
//#define _SDR_NOITEMS



//#define SI_NOITEMS
//#define SI_NODRW
#define _SI_NOSBXCONTROLS
//#define _VCATTR_HXX
#define _VCONT_HXX
//#define _VCSBX_HXX
#define _SI_NOOTHERFORMS
#define _VCTRLS_HXX
//#define _VCDRWOBJ_HXX
#define _SI_NOCONTROL
#define _SETBRW_HXX
#define _VCBRW_HXX
#define _SI_NOSBXCONTROLS
//#define _SIDLL_HXX ***

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX

// INCLUDE ---------------------------------------------------------------

#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/svdxcgv.hxx>
#include <svx/linkmgr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/drag.hxx>
#include <vcl/help.hxx>
#include <vcl/sound.hxx>
#include <tools/urlobj.hxx>
#include <svtools/urlbmk.hxx>
#include <stdlib.h>

#include "content.hxx"
#include "navipi.hxx"
#include "global.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "tablink.hxx"          // fuer Loader
#include "popmenu.hxx"
#include "drwlayer.hxx"
#include "dataobj.hxx"
#include "cell.hxx"
#include "dociter.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "navipi.hrc"
#include "arealink.hxx"
#include "navicfg.hxx"

//  Reihenfolge der Kategorien im Navigator -------------------------------------

static USHORT pTypeList[SC_CONTENT_COUNT] =
{
    SC_CONTENT_ROOT,            // ROOT (0) muss vorne stehen
    SC_CONTENT_TABLE,
    SC_CONTENT_RANGENAME,
    SC_CONTENT_DBAREA,
    SC_CONTENT_AREALINK,
    SC_CONTENT_GRAPHIC,
    SC_CONTENT_OLEOBJECT,
    SC_CONTENT_NOTE
};

BOOL ScContentTree::bIsInDrag = FALSE;


ScDocShell* ScContentTree::GetManualOrCurrent()
{
    ScDocShell* pSh = NULL;
    if ( aManualDoc.Len() )
    {
        TypeId aScType = TYPE(ScDocShell);
        SfxObjectShell* pObjSh = SfxObjectShell::GetFirst( &aScType );
        while ( pObjSh && !pSh )
        {
            if ( pObjSh->GetTitle() == aManualDoc )
                pSh = PTR_CAST( ScDocShell, pObjSh );
            pObjSh = SfxObjectShell::GetNext( *pObjSh, &aScType );
        }
    }
    else
    {
        //  Current nur, wenn keine manuell eingestellt ist
        //  (damit erkannt wird, wenn das Dokument nicht mehr existiert)

        SfxViewShell* pViewSh = SfxViewShell::Current();
        if ( pViewSh )
        {
            SfxObjectShell* pObjSh = pViewSh->GetViewFrame()->GetObjectShell();
            pSh = PTR_CAST( ScDocShell, pObjSh );
        }
    }

    return pSh;
}

//
//          ScContentTree
//

ScContentTree::ScContentTree( Window* pParent, const ResId& rResId ) :
    SvTreeListBox   ( pParent, rResId ),
    aExpBmp         ( ScResId( RID_BMP_EXPAND ) ),
    aCollBmp        ( ScResId( RID_BMP_COLLAPSE ) ),
    aEntryImages    ( ScResId( RID_IMAGELIST_NAVCONT ) ),
    nRootType       ( SC_CONTENT_ROOT ),
    bHiddenDoc      ( FALSE ),
    pHiddenDocument ( NULL )
{
    USHORT i;
    for (i=0; i<SC_CONTENT_COUNT; i++)
        pPosList[pTypeList[i]] = i;         // invers zum suchen

    pParentWindow = (ScNavigatorDlg*)pParent;

    pRootNodes[0] = NULL;
    for (i=1; i<SC_CONTENT_COUNT; i++)
        InitRoot(i);

    SetNodeBitmaps( aExpBmp, aCollBmp );

    SetDoubleClickHdl( LINK( this, ScContentTree, DoubleClickHdl ) );
}

ScContentTree::~ScContentTree()
{
}

void ScContentTree::InitRoot( USHORT nType )
{
    if ( !nType )
        return;

    if ( nRootType && nRootType != nType )              // ausgeblendet ?
    {
        pRootNodes[nType] = NULL;
        return;
    }

    const Image& rImage = aEntryImages.GetImage( nType );
    String aName( ScResId( SCSTR_CONTENT_ROOT + nType ) );
    // wieder an die richtige Position:
    USHORT nPos = nRootType ? 0 : pPosList[nType]-1;
    SvLBoxEntry* pNew = InsertEntry( aName, rImage, rImage, NULL, FALSE, nPos );
    pRootNodes[nType] = pNew;
}

void ScContentTree::ClearAll()
{
    Clear();
    for (USHORT i=1; i<SC_CONTENT_COUNT; i++)
        InitRoot(i);
}

void ScContentTree::ClearType(USHORT nType)
{
    if (!nType)
        ClearAll();
    else
    {
        SvLBoxEntry* pParent = pRootNodes[nType];
        if ( !pParent || GetChildCount(pParent) )       // nicht, wenn ohne Children schon da
        {
            if (pParent)
                GetModel()->Remove( pParent );          // mit allen Children
            InitRoot( nType );                          // ggf. neu eintragen
        }
    }
}

void ScContentTree::InsertContent( USHORT nType, const String& rValue )
{
    if (nType >= SC_CONTENT_COUNT)
    {
        DBG_ERROR("ScContentTree::InsertContent mit falschem Typ");
        return;
    }

    SvLBoxEntry* pParent = pRootNodes[nType];
    if (pParent)
        InsertEntry( rValue, pParent );
    else
        DBG_ERROR("InsertContent ohne Parent");
}

USHORT ScContentTree::GetCurrentContent( String& rValue )
{
    SvLBoxEntry* pEntry = GetCurEntry();
    if (!pEntry)
    {
        DBG_ERROR("kein aktueller Eintrag!");
        rValue.Erase();
        return 0;
    }
    SvLBoxEntry* pParent = GetParent(pEntry);   // kann 0 sein, wenn umgeschaltet ist

    USHORT nType = 0;
    BOOL bRoot = FALSE;
    for (USHORT i=1; i<SC_CONTENT_COUNT; i++)
    {
        if ( pEntry == pRootNodes[i] )
        {
            bRoot = TRUE;
            rValue = GetEntryText(pEntry);
        }
        else if ( pParent && pParent == pRootNodes[i] )
        {
            nType = i;
            rValue = GetEntryText(pEntry);
        }
    }

    DBG_ASSERT( bRoot || nType, "ScContentTree: unbekannter Eintrag" );

    return nType;
}

ULONG ScContentTree::GetCurrentIndex( SvLBoxEntry* pCurrent )
{
    if (!pCurrent)
        pCurrent = GetCurEntry();
    if (!pCurrent)
    {
        DBG_ERROR("kein aktueller Eintrag!");
        return 0;
    }
    SvLBoxEntry* pParent = GetParent(pCurrent);
    if (!pParent)
    {
        DBG_ERROR("kein Parent!");
        return 0;
    }

    ULONG nIndex = 0;
    SvLBoxEntry* pEntry = FirstChild( pParent );
    while (pEntry)
    {
        if ( pEntry == pCurrent )
            return nIndex;

        pEntry = NextSibling( pEntry );
        ++nIndex;
    }

    DBG_ERROR("Eintrag nicht gefunden");
    return 0;
}

String lcl_GetDBAreaRange( ScDocument* pDoc, const String& rDBName )
{
    String aRet;
    if (pDoc)
    {
        ScDBCollection* pDbNames = pDoc->GetDBCollection();
        USHORT nCount = pDbNames->GetCount();
        for ( USHORT i=0; i<nCount; i++ )
        {
            ScDBData* pData = (*pDbNames)[i];
            if ( pData->GetName() == rDBName )
            {
                ScRange aRange;
                pData->GetArea(aRange);
                aRange.Format( aRet, SCR_ABS_3D, pDoc );
                break;
            }
        }
    }
    return aRet;
}

IMPL_LINK( ScContentTree, DoubleClickHdl, ScContentTree *, EMPTYARG )
{
    String aText;
    USHORT nType = GetCurrentContent( aText );
    if ( nType )
    {
        if ( bHiddenDoc )
            return 0;               //! spaeter...

        if ( aManualDoc.Len() )
            pParentWindow->SetCurrentDoc( aManualDoc );

        switch( nType )
        {
            case SC_CONTENT_TABLE:
                pParentWindow->SetCurrentTableStr( aText );
                break;
            case SC_CONTENT_RANGENAME:
                pParentWindow->SetCurrentCellStr( aText );
                break;
            case SC_CONTENT_DBAREA:
                {
                    //  #47905# Wenn gleiche Bereichs- und DB-Namen existieren, wird
                    //  bei SID_CURRENTCELL der Bereichsname genommen.
                    //  DB-Bereiche darum direkt ueber die Adresse anspringen.

                    String aRangeStr = lcl_GetDBAreaRange( GetSourceDocument(), aText );
                    if (aRangeStr.Len())
                        pParentWindow->SetCurrentCellStr( aRangeStr );
                }
                break;
            case SC_CONTENT_OLEOBJECT:
            case SC_CONTENT_GRAPHIC:
                pParentWindow->SetCurrentObject( aText );
                break;
            case SC_CONTENT_NOTE:
                {
                    ULONG nIndex = GetCurrentIndex();
                    ScAddress aPos = GetNotePos(nIndex);
                    pParentWindow->SetCurrentTable( aPos.Tab() );
                    pParentWindow->SetCurrentCell( aPos.Col(), aPos.Row() );
                }
                break;
            case SC_CONTENT_AREALINK:
                {
                    ULONG nIndex = GetCurrentIndex();
                    const ScAreaLink* pLink = GetLink(nIndex);
                    if (pLink)
                    {
                        ScRange aRange = pLink->GetDestArea();
                        String aRangeStr;
                        aRange.Format( aRangeStr, SCR_ABS_3D, GetSourceDocument() );
                        pParentWindow->SetCurrentCellStr( aRangeStr );
                    }
                }
                break;
        }

        ScNavigatorDlg::ReleaseFocus();     // set focus into document
    }

    return 0;
}

void ScContentTree::KeyInput( const KeyEvent& rKEvt )
{
    BOOL bUsed = FALSE;

    const KeyCode aCode = rKEvt.GetKeyCode();
    if (aCode.GetCode() == KEY_RETURN)
    {
        switch (aCode.GetModifier())
        {
            case KEY_MOD1:
                ToggleRoot();       // toggle root mode (as in Writer)
                bUsed = TRUE;
                break;
            case 0:
                {
                    String aText;
                    USHORT nType = GetCurrentContent( aText );
                    if ( nType == SC_CONTENT_ROOT )
                    {
                        SvLBoxEntry* pEntry = GetCurEntry();
                        if ( pEntry )
                        {
                            if ( IsExpanded(pEntry) )
                                Collapse(pEntry);
                            else
                                Expand(pEntry);
                        }
                    }
                    else
                        DoubleClickHdl(0);      // select content as if double clicked
                    bUsed = TRUE;
                }
                break;
        }
    }

    if (!bUsed)
        SvTreeListBox::KeyInput(rKEvt);
}

BOOL __EXPORT ScContentTree::Drop( const DropEvent& rEvt )
{
    return pParentWindow->Drop(rEvt);           // Drop auf Navigator
}

BOOL __EXPORT ScContentTree::QueryDrop( DropEvent& rEvt )
{
    return pParentWindow->QueryDrop(rEvt);      // Drop auf Navigator
}

void __EXPORT ScContentTree::Command( const CommandEvent& rCEvt )
{
    BOOL bDone = FALSE;

    switch ( rCEvt.GetCommand() )
    {
        case COMMAND_STARTDRAG:
            //  Aus dem ExecuteDrag heraus kann der Navigator geloescht werden
            //  (beim Umschalten auf einen anderen Dokument-Typ), das wuerde aber
            //  den StarView MouseMove-Handler, der Command() aufruft, umbringen.
            //  Deshalb Drag&Drop asynchron:

//          DoDrag();

            Application::PostUserEvent( STATIC_LINK( this, ScContentTree, ExecDragHdl ) );

            bDone = TRUE;
            break;

        case COMMAND_CONTEXTMENU:
            {
                //  Drag-Drop Modus

                PopupMenu aPop;
                ScPopupMenu aDropMenu( ScResId( RID_POPUP_DROPMODE ) );
                aDropMenu.CheckItem( RID_DROPMODE_URL + pParentWindow->GetDropMode() );
                aPop.InsertItem( 1, pParentWindow->GetStrDragMode() );
                aPop.SetPopupMenu( 1, &aDropMenu );

                //  angezeigtes Dokument

                ScPopupMenu aDocMenu;
                aDocMenu.SetMenuFlags( aDocMenu.GetMenuFlags() | MENU_FLAG_NOAUTOMNEMONICS );
                USHORT i=0;
                USHORT nPos=0;
                //  geladene Dokumente
                ScDocShell* pCurrentSh = PTR_CAST( ScDocShell, SfxObjectShell::Current() );
                SfxObjectShell* pSh = SfxObjectShell::GetFirst();
                while ( pSh )
                {
                    if ( pSh->ISA(ScDocShell) )
                    {
                        String aName = pSh->GetTitle();
                        String aEntry = aName;
                        if ( pSh == pCurrentSh )
                            aEntry += pParentWindow->aStrActive;
                        else
                            aEntry += pParentWindow->aStrNotActive;
                        aDocMenu.InsertItem( ++i, aEntry );
                        if ( !bHiddenDoc && aName == aManualDoc )
                            nPos = i;
                    }
                    pSh = SfxObjectShell::GetNext( *pSh );
                }
                //  "aktives Fenster"
                aDocMenu.InsertItem( ++i, pParentWindow->aStrActiveWin );
                if (!bHiddenDoc && !aManualDoc.Len())
                    nPos = i;
                //  verstecktes Dokument
                if ( aHiddenTitle.Len() )
                {
                    String aEntry = aHiddenTitle;
                    aEntry += pParentWindow->aStrHidden;
                    aDocMenu.InsertItem( ++i, aEntry );
                    if (bHiddenDoc)
                        nPos = i;
                }
                aDocMenu.CheckItem( nPos );
                aPop.InsertItem( 2, pParentWindow->GetStrDisplay() );
                aPop.SetPopupMenu( 2, &aDocMenu );

                //  ausfuehren

                aPop.Execute( this, rCEvt.GetMousePosPixel() );

                if ( aDropMenu.WasHit() )               //  Drag-Drop Modus
                {
                    USHORT nId = aDropMenu.GetSelected();
                    if ( nId >= RID_DROPMODE_URL && nId <= RID_DROPMODE_COPY )
                        pParentWindow->SetDropMode( nId - RID_DROPMODE_URL );
                }
                else if ( aDocMenu.WasHit() )           //  angezeigtes Dokument
                {
                    USHORT nId = aDocMenu.GetSelected();
                    String aName = aDocMenu.GetItemText(nId);
                    SelectDoc( aName );
                }
            }
            break;
    }

    if (!bDone)
        SvTreeListBox::Command(rCEvt);
}

void __EXPORT ScContentTree::RequestHelp( const HelpEvent& rHEvt )
{
    BOOL bDone = FALSE;
    if( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvLBoxEntry* pEntry = GetEntry( aPos );
        if ( pEntry )
        {
            BOOL bRet = FALSE;
            String aHelpText;
            SvLBoxEntry* pParent = GetParent(pEntry);
            if ( !pParent )                                 // Top-Level ?
            {
                aHelpText = String::CreateFromInt32( GetChildCount(pEntry) );
                aHelpText += ' ';
                aHelpText += GetEntryText(pEntry);
                bRet = TRUE;
            }
            else if ( pParent == pRootNodes[SC_CONTENT_NOTE] )
            {
                aHelpText = GetEntryText(pEntry);           // Notizen als Help-Text
                bRet = TRUE;
            }
            else if ( pParent == pRootNodes[SC_CONTENT_AREALINK] )
            {
                ULONG nIndex = GetCurrentIndex(pEntry);
                const ScAreaLink* pLink = GetLink(nIndex);
                if (pLink)
                {
                    aHelpText = pLink->GetFile();           // Source-Datei als Help-Text
                    bRet = TRUE;
                }
            }

            if (bRet)
            {
                SvLBoxTab* pTab;
                SvLBoxString* pItem = (SvLBoxString*)(GetItem( pEntry, aPos.X(), &pTab ));
                if( pItem )
                {
                    aPos = GetEntryPos( pEntry );
                    aPos.X() = GetTabPos( pEntry, pTab );
                    aPos = OutputToScreenPixel(aPos);
                    Size aSize( pItem->GetSize( this, pEntry ) );

                    Rectangle aItemRect( aPos, aSize );
                    Help::ShowQuickHelp( this, aItemRect, aHelpText );
                    bDone = TRUE;
                }
            }
        }
    }
    if (!bDone)
        Window::RequestHelp( rHEvt );
}

ScDocument* ScContentTree::GetSourceDocument()
{
    if (bHiddenDoc)
        return pHiddenDocument;
    else
    {
        ScDocShell* pSh = GetManualOrCurrent();
        if (pSh)
            return pSh->GetDocument();

    }
    return NULL;
}

void ScContentTree::Refresh(USHORT nType)
{
    if ( bHiddenDoc && !pHiddenDocument )
        return;                                 // anderes Dokument angezeigt

    //  wenn sich nichts geaendert hat, gleich abbrechen (gegen Geflacker)

    if ( nType == SC_CONTENT_NOTE )
        if (!NoteStringsChanged())
            return;
    if ( nType == SC_CONTENT_GRAPHIC )
        if (!DrawNamesChanged(SC_CONTENT_GRAPHIC, OBJ_GRAF))
            return;
    if ( nType == SC_CONTENT_OLEOBJECT )
        if (!DrawNamesChanged(SC_CONTENT_OLEOBJECT, OBJ_OLE2))
            return;

    //  Expand-Zustand merken
    //! und Selektion

    USHORT i;
    BOOL bExpanded[SC_CONTENT_COUNT];
    BOOL bSelected[SC_CONTENT_COUNT];
    for (i=1; i<SC_CONTENT_COUNT; i++)
    {
        if (pRootNodes[i])
        {
            bExpanded[i] = IsExpanded( pRootNodes[i] );
            bSelected[i] = IsSelected( pRootNodes[i] );
        }
        else
            bExpanded[i] = bSelected[i] = FALSE;
    }

    SetUpdateMode(FALSE);

    ClearType( nType );

    if ( !nType || nType == SC_CONTENT_TABLE )
        GetTableNames();
    if ( !nType || nType == SC_CONTENT_RANGENAME )
        GetAreaNames();
    if ( !nType || nType == SC_CONTENT_DBAREA )
        GetDbNames();
    if ( !nType || nType == SC_CONTENT_GRAPHIC )
        GetGraphicNames();
    if ( !nType || nType == SC_CONTENT_OLEOBJECT )
        GetOleNames();
    if ( !nType || nType == SC_CONTENT_NOTE )
        GetNoteStrings();
    if ( !nType || nType == SC_CONTENT_AREALINK )
        GetLinkNames();

    for (i=1; i<SC_CONTENT_COUNT; i++)
        if (pRootNodes[i])
        {
            if ( bExpanded[i] != IsExpanded(pRootNodes[i]) )
            {
                if ( bExpanded[i] )
                    Expand(pRootNodes[i]);
                else
                    Collapse(pRootNodes[i]);
            }
            if ( bSelected[i] != IsSelected(pRootNodes[i]) )
                Select( pRootNodes[i], bSelected[i] );
        }

    SetUpdateMode(TRUE);
}

void ScContentTree::GetTableNames()
{
    if ( nRootType && nRootType != SC_CONTENT_TABLE )       // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    String aName;
    USHORT nCount = pDoc->GetTableCount();
    for ( USHORT i=0; i<nCount; i++ )
    {
        pDoc->GetName( i, aName );
        InsertContent( SC_CONTENT_TABLE, aName );
    }
}

void ScContentTree::GetAreaNames()
{
    if ( nRootType && nRootType != SC_CONTENT_RANGENAME )       // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    ScRangeName* pRangeNames = pDoc->GetRangeName();
    USHORT nCount = pRangeNames->GetCount();
    if ( nCount > 0 )
    {
        USHORT nValidCount = 0;
        ScRange aDummy;
        USHORT i;
        for ( i=0; i<nCount; i++ )
        {
            ScRangeData* pData = (*pRangeNames)[i];
            if (pData->IsReference(aDummy))
                nValidCount++;
        }
        if ( nValidCount )
        {
            ScRangeData** ppSortArray = new ScRangeData* [ nValidCount ];
            USHORT j;
            for ( i=0, j=0; i<nCount; i++ )
            {
                ScRangeData* pData = (*pRangeNames)[i];
                if (pData->IsReference(aDummy))
                    ppSortArray[j++] = pData;
            }
#ifndef ICC
            qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
                &ScRangeData::QsortNameCompare );
#else
            qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
                ICCQsortNameCompare );
#endif
            for ( j=0; j<nValidCount; j++ )
                InsertContent( SC_CONTENT_RANGENAME, ppSortArray[j]->GetName() );
            delete [] ppSortArray;
        }
    }
}

void ScContentTree::GetDbNames()
{
    if ( nRootType && nRootType != SC_CONTENT_DBAREA )      // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    ScDBCollection* pDbNames = pDoc->GetDBCollection();
    USHORT nCount = pDbNames->GetCount();
    if ( nCount > 0 )
    {
        String aStrNoName( ScGlobal::GetRscString(STR_DB_NONAME) );
        for ( USHORT i=0; i<nCount; i++ )
        {
            ScDBData* pData = (*pDbNames)[i];
            String aStrName = pData->GetName();
            if ( aStrName != aStrNoName )
                InsertContent( SC_CONTENT_DBAREA, aStrName );
        }
    }
}

void ScContentTree::GetDrawNames( USHORT nType, USHORT nId )
{
    if ( nRootType && nRootType != nType )              // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pDrawLayer && pShell)
    {
        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT nTab=0; nTab<nTabCount; nTab++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            DBG_ASSERT(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == nId )
                    {
                        String aName = pObject->GetName();
                        if (aName.Len())
                            InsertContent( nType, aName );
                    }

                    pObject = aIter.Next();
                }
            }
        }
    }
}

void ScContentTree::GetGraphicNames()
{
    GetDrawNames( SC_CONTENT_GRAPHIC, OBJ_GRAF );
}

void ScContentTree::GetOleNames()
{
    GetDrawNames( SC_CONTENT_OLEOBJECT, OBJ_OLE2 );
}

void ScContentTree::GetLinkNames()
{
    if ( nRootType && nRootType != SC_CONTENT_AREALINK )                // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
    DBG_ASSERT(pLinkManager, "kein LinkManager am Dokument?")
    const SvBaseLinks& rLinks = pLinkManager->GetLinks();
    USHORT nCount = rLinks.Count();
    for (USHORT i=0; i<nCount; i++)
    {
        SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScAreaLink))
            InsertContent( SC_CONTENT_AREALINK, ((ScAreaLink*)pBase)->GetSource() );

            //  in der Liste die Namen der Quellbereiche
    }
}

const ScAreaLink* ScContentTree::GetLink( ULONG nIndex )
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return NULL;

    ULONG nFound = 0;
    SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
    DBG_ASSERT(pLinkManager, "kein LinkManager am Dokument?")
    const SvBaseLinks& rLinks = pLinkManager->GetLinks();
    USHORT nCount = rLinks.Count();
    for (USHORT i=0; i<nCount; i++)
    {
        SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScAreaLink))
        {
            if (nFound == nIndex)
                return (const ScAreaLink*) pBase;
            ++nFound;
        }
    }

    DBG_ERROR("Link nicht gefunden");
    return NULL;
}

String lcl_NoteString( const ScPostIt& rNote )
{
    String aText = rNote.GetText();
    aText.ConvertLineEnd( LINEEND_CR );
    xub_StrLen nAt;
    while ( (nAt = aText.Search( CHAR_CR )) != STRING_NOTFOUND )
        aText.SetChar( nAt, ' ' );
    return aText;
}

void ScContentTree::GetNoteStrings()
{
    if ( nRootType && nRootType != SC_CONTENT_NOTE )        // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    USHORT nTabCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<nTabCount; nTab++)
    {
        ScCellIterator aIter( pDoc, 0,0,nTab, MAXCOL,MAXROW,nTab );
        ScBaseCell* pCell = aIter.GetFirst();
        while (pCell)
        {
            const ScPostIt* pNote = pCell->GetNotePtr();
            if (pNote)
                InsertContent( SC_CONTENT_NOTE, lcl_NoteString(*pNote) );

            pCell = aIter.GetNext();
        }
    }
}

ScAddress ScContentTree::GetNotePos( ULONG nIndex )
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return ScAddress();

    ULONG nFound = 0;
    USHORT nTabCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<nTabCount; nTab++)
    {
        ScCellIterator aIter( pDoc, 0,0,nTab, MAXCOL,MAXROW,nTab );
        ScBaseCell* pCell = aIter.GetFirst();
        while (pCell)
        {
            const ScPostIt* pNote = pCell->GetNotePtr();
            if (pNote)
            {
                if (nFound == nIndex)
                    return ScAddress( aIter.GetCol(), aIter.GetRow(), nTab );   // gefunden
                ++nFound;
            }
            pCell = aIter.GetNext();
        }
    }

    DBG_ERROR("Notiz nicht gefunden");
    return ScAddress();
}

BOOL ScContentTree::NoteStringsChanged()
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return FALSE;

    SvLBoxEntry* pParent = pRootNodes[SC_CONTENT_NOTE];
    if (!pParent)
        return FALSE;

    SvLBoxEntry* pEntry = FirstChild( pParent );

    BOOL bEqual = TRUE;
    USHORT nTabCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<nTabCount && bEqual; nTab++)
    {
        ScCellIterator aIter( pDoc, 0,0,nTab, MAXCOL,MAXROW,nTab );
        ScBaseCell* pCell = aIter.GetFirst();
        while (pCell && bEqual)
        {
            const ScPostIt* pNote = pCell->GetNotePtr();
            if (pNote)
            {
                if ( !pEntry )
                    bEqual = FALSE;
                else
                {
                    if ( lcl_NoteString(*pNote) != GetEntryText(pEntry) )
                        bEqual = FALSE;

                    pEntry = NextSibling( pEntry );
                }
            }
            pCell = aIter.GetNext();
        }
    }

    if ( pEntry )
        bEqual = FALSE;             // kommt noch was

    return !bEqual;
}

BOOL ScContentTree::DrawNamesChanged( USHORT nType, USHORT nId )
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return FALSE;

    SvLBoxEntry* pParent = pRootNodes[nType];
    if (!pParent)
        return FALSE;

    SvLBoxEntry* pEntry = FirstChild( pParent );

    BOOL bEqual = TRUE;
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pDrawLayer && pShell)
    {
        USHORT nTabCount = pDoc->GetTableCount();
        for (USHORT nTab=0; nTab<nTabCount && bEqual; nTab++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            DBG_ASSERT(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject && bEqual)
                {
                    if ( pObject->GetObjIdentifier() == nId )
                    {
                        if ( !pEntry )
                            bEqual = FALSE;
                        else
                        {
                            if ( pObject->GetName() != GetEntryText(pEntry) )
                                bEqual = FALSE;

                            pEntry = NextSibling( pEntry );
                        }
                    }
                    pObject = aIter.Next();
                }
            }
        }
    }

    if ( pEntry )
        bEqual = FALSE;             // kommt noch was

    return !bEqual;
}

BOOL lcl_GetRange( ScDocument* pDoc, USHORT nType, const String& rName, ScRange& rRange )
{
    BOOL bFound = FALSE;
    USHORT nPos;

    if ( nType == SC_CONTENT_RANGENAME )
    {
        ScRangeName* pList = pDoc->GetRangeName();
        if (pList)
            if (pList->SearchName( rName, nPos ))
                if ( (*pList)[nPos]->IsReference( rRange ) )
                    bFound = TRUE;
    }
    else if ( nType == SC_CONTENT_DBAREA )
    {
        ScDBCollection* pList = pDoc->GetDBCollection();
        if (pList)
            if (pList->SearchName( rName, nPos ))
            {
                USHORT nTab,nCol1,nRow1,nCol2,nRow2;
                (*pList)[nPos]->GetArea(nTab,nCol1,nRow1,nCol2,nRow2);
                rRange = ScRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab );
                bFound = TRUE;
            }
    }

    return bFound;
}

void lcl_DoDragObject( ScDocument* pSrcDoc, const String& rName, USHORT nType, Window* pWin )
{
    ScDrawLayer* pModel = pSrcDoc->GetDrawLayer();
    if (pModel)
    {
        BOOL bOle = ( nType == SC_CONTENT_OLEOBJECT );
        USHORT nDrawId = bOle ? OBJ_OLE2 : OBJ_GRAF;
        USHORT nTab = 0;
        SdrObject* pObject = pModel->GetNamedObject( rName, nDrawId, nTab );
        if (pObject)
        {
            ScModule* pScMod = SC_MOD();

            SdrView aEditView( pModel );
            aEditView.ShowPagePgNum( nTab, Point() );
            SdrPageView* pPV = aEditView.GetPageViewPvNum(0);
            aEditView.MarkObj(pObject, pPV);

            SdrModel* pDragModel = aEditView.GetAllMarkedModel();

            SvDataObjectRef pDragServer = new ScDataObject( pDragModel,
                                                (ScDocShell*)pSrcDoc->GetDocumentShell(), bOle );
            pScMod->SetDragObject(pDragModel, &aEditView, SC_DROP_NAVIGATOR);
            pWin->ReleaseMouse();
            DropAction eDropAction = pDragServer->ExecuteDrag(pWin,
                                        POINTER_COPYDATA, POINTER_COPYDATA, POINTER_LINKDATA,
                                        DRAG_ALL);
            BOOL bIntern = pScMod->GetDragIntern();
            pScMod->ResetDragObject();
            pDragServer.Clear();
        }
    }
}

void ScContentTree::DoDrag()
{
    ScDocumentLoader* pDocLoader = NULL;
    bIsInDrag = TRUE;

    DragServer::Clear();
    BOOL bOk = FALSE;
    ScModule* pScMod = SC_MOD();

    String aText;
    USHORT nType = GetCurrentContent( aText );
    if ( nType && nType != SC_CONTENT_NOTE
               && nType != SC_CONTENT_AREALINK )        // Notizen und AreaLinks gar nicht
    {
        ScDocument* pLocalDoc = NULL;                   // fuer URL-Drop
        String aDocName;
        if (bHiddenDoc)
            aDocName = aHiddenName;
        else
        {
            ScDocShell* pDocSh = GetManualOrCurrent();
            if (pDocSh)
            {
                if (pDocSh->HasName())
                    aDocName = pDocSh->GetMedium()->GetName();
                else
                    pLocalDoc = pDocSh->GetDocument();      // Drop nur in dieses Dokument
            }
        }

        USHORT nDropMode = pParentWindow->GetDropMode();
        switch ( nDropMode )
        {
            case SC_DROPMODE_URL:
                {
                    String aUrl = aDocName;
                    aUrl += '#';
                    aUrl += aText;

                    pScMod->SetDragJump( pLocalDoc, aUrl, aText );

                    if (aDocName.Len())
                    {
                        //  URL nur nach aussen geben, wenn das Doc einen Namen hat
                        //  (ohne Namen nur intern, ueber SetDragJump)
                        INetBookmark aBmk(aUrl, aText);
                        aBmk.CopyDragServer();
                    }
                    bOk = TRUE;
                }
                break;
            case SC_DROPMODE_LINK:
                {
                    if ( aDocName.Len() )           // Verknuepfung nur mit benannten Docs
                    {
                        // fuer Drop im Calc Flag setzen, dass Verknuepfung eingefuegt wird

                        switch ( nType )
                        {
                            case SC_CONTENT_TABLE:
                                pScMod->SetDragLink( aDocName, aText, EMPTY_STRING );
                                bOk = TRUE;
                                break;
                            case SC_CONTENT_RANGENAME:
                            case SC_CONTENT_DBAREA:
                                pScMod->SetDragLink( aDocName, EMPTY_STRING, aText );
                                bOk = TRUE;
                                break;

                            // andere koennen nicht gelinkt werden
                        }
                    }
                }
                break;
            case SC_DROPMODE_COPY:
                {
                    ScDocShell* pSrcShell = NULL;
                    if ( bHiddenDoc )
                    {
                        String aFilter, aOptions;
                        pDocLoader = new ScDocumentLoader( aHiddenName, aFilter, aOptions );
                        if (!pDocLoader->IsError())
                            pSrcShell = pDocLoader->GetDocShell();
                    }
                    else
                        pSrcShell = GetManualOrCurrent();

                    if ( pSrcShell )
                    {
                        ScDocument* pSrcDoc = pSrcShell->GetDocument();
                        if ( nType == SC_CONTENT_RANGENAME || nType == SC_CONTENT_DBAREA )
                        {
                            ScRange aRange;
                            if ( lcl_GetRange( pSrcDoc, nType, aText, aRange ) )
                            {
                                // @ 05.01.98
                                // sollte noch ueberarbeitet werden
                                pScMod->SetDragObject(ScMarkData(), aRange, 0,0, pSrcDoc, SC_DROP_NAVIGATOR );
                                bOk = TRUE;
                            }
                        }
                        else if ( nType == SC_CONTENT_TABLE )
                        {
                            USHORT nTab;
                            if ( pSrcDoc->GetTable( aText, nTab ) )
                            {
                                ScRange aRange( 0,0,nTab, MAXCOL,MAXROW,nTab );

                                ScMarkData aMarkData;

                                aMarkData.SetMarkArea(aRange);
                                aMarkData.SelectTable(nTab, TRUE);

                                pScMod->SetDragObject( aMarkData, aRange, 0,0, pSrcDoc,
                                                        SC_DROP_NAVIGATOR | SC_DROP_TABLE );
                                bOk = TRUE;
                            }
                        }
                        else if ( nType == SC_CONTENT_GRAPHIC || nType == SC_CONTENT_OLEOBJECT )
                        {
                            lcl_DoDragObject( pSrcDoc, aText, nType, this );

                            //  in ExecuteDrag kann der Navigator geloescht worden sein
                            //  -> nicht mehr auf Member zugreifen !!!
                        }
                    }
                }
                break;
        }

        if (bOk)
        {
            //  #41821# Unter OS/2 muss fuer ExecuteDrag immer etwas im DragServer sein
            //  #45443# dito mit VCL, CopyPrivateData geht mit VCL auch nicht
            if (!DragServer::GetFormatCount(0))
                DragServer::CopyRequest(FORMAT_PRIVATE);

            Pointer aCopy(POINTER_COPYDATA);
            ReleaseMouse();
            ExecuteDrag( aCopy, aCopy, DROP_LINK );

            //  in ExecuteDrag kann der Navigator geloescht worden sein
            //  -> nicht mehr auf Member zugreifen !!!

            pScMod->ResetDragObject();
        }
    }

    bIsInDrag = FALSE;              // static Member

    delete pDocLoader;              // falls Dokument zum Draggen geladen wurde
}

IMPL_STATIC_LINK(ScContentTree, ExecDragHdl, void*, EMPTYARG)
{
    //  als Link, damit asynchron ohne ImpMouseMoveMsg auf dem Stack auch der
    //  Navigator geloescht werden darf

    pThis->DoDrag();
    return 0;
}

void ScContentTree::AdjustTitle()
{
    String aTitle = pParentWindow->aTitleBase;
    if (bHiddenDoc)
    {
        aTitle.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " - " ));
        aTitle += aHiddenTitle;
    }
    pParentWindow->SetText(aTitle);
}

BOOL ScContentTree::LoadFile( const String& rUrl )
{
    String aDocName = rUrl;
    xub_StrLen nPos = aDocName.Search('#');
    if ( nPos != STRING_NOTFOUND )
        aDocName.Erase(nPos);           // nur der Name, ohne #...

    BOOL bReturn = FALSE;
    String aFilter, aOptions;
    ScDocumentLoader aLoader( aDocName, aFilter, aOptions );
    if ( !aLoader.IsError() )
    {
        bHiddenDoc = TRUE;
        aHiddenName = aDocName;
        aHiddenTitle = aLoader.GetTitle();
        pHiddenDocument = aLoader.GetDocument();

        Refresh();                      // Inhalte aus geladenem Dokument holen

        pHiddenDocument = NULL;
//      AdjustTitle();

        pParentWindow->GetDocNames( &aHiddenTitle );            // Liste fuellen
    }
    else
        Sound::Beep();          // Fehler beim Laden

    //  Dokument wird im dtor von ScDocumentLoader wieder geschlossen

    return bReturn;
}

void ScContentTree::InitWindowBits( BOOL bButtons )
{
    WinBits nFlags = WB_CLIPCHILDREN|WB_HSCROLL;
    if (bButtons)
        nFlags |= WB_HASBUTTONS|WB_HASBUTTONSATROOT;

    SetWindowBits( nFlags );
}

void ScContentTree::SetRootType( USHORT nNew )
{
    if ( nNew != nRootType )
    {
        nRootType = nNew;
        InitWindowBits( nNew == 0 );
        Refresh();

        ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
        rCfg.SetRootType( nRootType );
    }
}

void ScContentTree::ToggleRoot()        // nach Selektion
{
    USHORT nNew = SC_CONTENT_ROOT;
    if ( nRootType == SC_CONTENT_ROOT )
    {
        SvLBoxEntry* pEntry = GetCurEntry();
        if (pEntry)
        {
            SvLBoxEntry* pParent = GetParent(pEntry);
            for (USHORT i=1; i<SC_CONTENT_COUNT; i++)
                if ( pEntry == pRootNodes[i] || pParent == pRootNodes[i] )
                    nNew = i;
        }
    }

    SetRootType( nNew );
}

void ScContentTree::ResetManualDoc()
{
    aManualDoc.Erase();
    bHiddenDoc = FALSE;

    ActiveDocChanged();
}

void ScContentTree::ActiveDocChanged()
{
    if ( !bHiddenDoc && !aManualDoc.Len() )
        Refresh();                                  // Inhalte nur wenn automatisch

        //  Listbox muss immer geupdated werden, wegen aktiv-Flag

    String aCurrent;
    if ( bHiddenDoc )
        aCurrent = aHiddenTitle;
    else
    {
        ScDocShell* pSh = GetManualOrCurrent();
        if (pSh)
            aCurrent = pSh->GetTitle();
        else
        {
            //  eingestelltes Dokument existiert nicht mehr

            aManualDoc.Erase();             // wieder automatisch
            Refresh();
            pSh = GetManualOrCurrent();     // sollte jetzt aktives sein
            if (pSh)
                aCurrent = pSh->GetTitle();
        }
    }
    pParentWindow->GetDocNames( &aCurrent );        // selektieren
}

void ScContentTree::SetManualDoc(const String& rName)
{
    aManualDoc = rName;
    if (!bHiddenDoc)
    {
        Refresh();
        pParentWindow->GetDocNames( &aManualDoc );      // selektieren
    }
}

void ScContentTree::SelectDoc(const String& rName)      // rName wie im Menue/Listbox angezeigt
{
    if ( rName == pParentWindow->aStrActiveWin )
    {
        ResetManualDoc();
        return;
    }

    //  "aktiv" oder "inaktiv" weglassen

    String aRealName = rName;
    xub_StrLen nLen = rName.Len();
    xub_StrLen nActiveStart = nLen - pParentWindow->aStrActive.Len();
    if ( rName.Copy( nActiveStart ) == pParentWindow->aStrActive )
        aRealName = rName.Copy( 0, nActiveStart );
    xub_StrLen nNotActiveStart = nLen - pParentWindow->aStrNotActive.Len();
    if ( rName.Copy( nNotActiveStart ) == pParentWindow->aStrNotActive )
        aRealName = rName.Copy( 0, nNotActiveStart );

    //

    BOOL bLoaded = FALSE;

        // ist es ein normal geladenes Doc ?

    SfxObjectShell* pSh = SfxObjectShell::GetFirst();
    while ( pSh && !bLoaded )
    {
        if ( pSh->ISA(ScDocShell) )
            if ( pSh->GetTitle() == aRealName )
                bLoaded = TRUE;
        pSh = SfxObjectShell::GetNext( *pSh );
    }

    if (bLoaded)
    {
        bHiddenDoc = FALSE;
        SetManualDoc(aRealName);
    }
    else if (aHiddenTitle.Len())                // verstecktes ausgewaehlt
    {
        if (!bHiddenDoc)
            LoadFile(aHiddenName);
    }
    else
        DBG_ERROR("SelectDoc: nicht gefunden");
}

//
//------------------------------------------------------------------------
//





