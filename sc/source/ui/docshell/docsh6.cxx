/*************************************************************************
 *
 *  $RCSfile: docsh6.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:55 $
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

// System - Includes -----------------------------------------------------

//#define _BASEDLGS_HXX ***
#define _BIGINT_HXX
#define _CACHESTR_HXX
//#define _CLIP_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
//#define _COLOR_HXX
//#define _CTRLBOX_HXX
//#define _CTRLTOOL_HXX
#define _DIALOGS_HXX
#define _DLGCFG_HXX
#define _DYNARR_HXX
#define _EXTATTR_HXX
//#define _FIELD_HXX
#define _FILDLG_HXX
//#define _FILTER_HXX
#define _FONTDLG_HXX
#define _FRM3D_HXX
//#define _GRAPH_HXX
//#define _GDIMTF_HXX
#define _INTRO_HXX
#define _ISETBWR_HXX
#define _NO_SVRTF_PARSER_HXX
//#define _MDIFRM_HXX
#define _MACRODLG_HXX
#define _MODALDLG_HXX
#define _MOREBUTTON_HXX
//#define _MAPMOD_HXX
#define _OUTLINER_HXX
//#define _PAL_HXX
#define _PASSWD_HXX
//#define _PRNDLG_HXX   //
#define _POLY_HXX
#define _PVRWIN_HXX
#define _QUEUE_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
//#define _SELENG_HXX
#define _SETBRW_HXX
//#define _SOUND_HXX
#define _STACK_HXX
//#define _STATUS_HXX ***
#define _STDMENU_HXX
//#define _STDCTRL_HXX
//#define _SYSDLG_HXX
//#define _TAB_HXX
#define _TABBAR_HXX
//#define _TREELIST_HXX
//#define _VALUESET_HXX
#define _VCBRW_HXX
#define _VCTRLS_HXX
//#define _VCATTR_HXX
#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX
//#define _VIEWFAC_HXX


#define _SFX_PRNMON_HXX
#define _SFX_RESMGR_HXX
//#define _SFX_SAVEOPT_HXX
#define _SFX_TEMPLDLG_HXX
//#define _SFXAPP_HXX
#define _SFXBASIC_HXX
//#define _SFXCTRLITEM_HXX
#define _SFXDISPATCH_HXX
#define _SFXFILEDLG_HXX
#define _SFXIMGMGR_HXX
#define _SFXMNUMGR_HXX
//#define _SFXMSG_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
//#define _SFXOBJFACE_HXX
//#define _SFXREQUEST_HXX
#define _SFXSTBITEM_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXCTRL_HXX
#define _SFXTBXMGR_HXX
#define _SFX_TEMPLDLG_HXX

#define _SI_DLL_HXX
#define _SIDLL_HXX
#define _SI_NOITEMS
#define _SI_NOOTHERFORMS
#define _SI_NOSBXCONTROLS
#define _SINOSBXCONTROLS
#define _SI_NODRW
#define _SI_NOCONTROL

#define _SV_NOXSOUND
#define _SVDATTR_HXX
#define _SVDETC_HXX
#define _SVDIO_HXX
#define _SVDRAG_HXX
#define _SVDLAYER_HXX
#define _SVDXOUT_HXX

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


#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <so3/ipenv.hxx>
#include "scitems.hxx"

#include <so3/ipenv.hxx>
#include <svx/pageitem.hxx>
#include <vcl/virdev.hxx>
#include <svx/linkmgr.hxx>

#include <segmentc.hxx>
#endif

// INCLUDE ---------------------------------------------------------------

//#include <svxlink.hxx>

#include "docsh.hxx"

#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "global.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "tablink.hxx"
#include "collect.hxx"

// STATIC DATA -----------------------------------------------------------

//----------------------------------------------------------------------

SEG_EOFGLOBALS()

//
//  Ole
//

#pragma SEG_FUNCDEF(docsh6_01)

String ScDocShell::CreateObjectName( const String& rPrefix )
{
    String aStr( rPrefix );
    USHORT i = 1;
    aStr += String::CreateFromInt32( i );
    while( Find( aStr ) )
    {
        i++;
        aStr = rPrefix;
        aStr += String::CreateFromInt32( i );
    }
    return aStr;
}

#pragma SEG_FUNCDEF(docsh6_02)

void __EXPORT ScDocShell::SetVisArea( const Rectangle & rVisArea )
{
    //  wenn von aussen gerufen, nur Groesse anpassen
    //  (linke obere Ecke bleibt unveraendert),
    //  weil sonst beim Verschieben des Objekts durch Pixel-Rundungsfehler
    //  manchmal ungewollt der Ausschnitt verschoben wird

    SetVisAreaOrSize( rVisArea, FALSE );
}

void ScDocShell::SetVisAreaOrSize( const Rectangle& rVisArea, BOOL bModifyStart )
{
    Rectangle aArea = rVisArea;
    if (!bModifyStart)
        aArea.SetPos( SfxInPlaceObject::GetVisArea().TopLeft() );

    //      hier Position anpassen!

    aDocument.SnapVisArea( aArea );

    SvInPlaceEnvironment* pEnv = GetIPEnv();
    if (pEnv)
    {
        Window* pWin = pEnv->GetEditWin();
        pEnv->MakeScale( aArea.GetSize(), MAP_100TH_MM,
                            pWin->LogicToPixel( aArea.GetSize() ) );
    }

    SvInPlaceObject::SetVisArea( aArea );

    if (bIsInplace)                     // Zoom in der InPlace View einstellen
    {
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        if (pViewSh)
        {
            if (pViewSh->GetViewData()->GetDocShell() == this)
                pViewSh->UpdateOleZoom();
        }
        else
            DataChanged( SvDataType() );            // fuer Zuppeln wenn nicht IP-aktiv
    }

    if (aDocument.IsEmbedded())
    {
        ScTripel aOldStart,aOldEnd;
        aDocument.GetEmbedded(aOldStart,aOldEnd);
        aDocument.SetEmbedded( aArea );
        ScTripel aNewStart,aNewEnd;
        aDocument.GetEmbedded(aNewStart,aNewEnd);
        if (aNewStart!=aOldStart || aNewEnd!=aOldEnd)
            PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB,PAINT_GRID);

        ViewChanged( ASPECT_CONTENT );          // auch im Container anzeigen
    }
}

#pragma SEG_FUNCDEF(docsh6_03)

BOOL ScDocShell::IsOle()
{
    return (eShellMode == SFX_CREATE_MODE_EMBEDDED);
}

#pragma SEG_FUNCDEF(docsh6_04)

void ScDocShell::UpdateOle( const ScViewData* pViewData, BOOL bSnapSize )
{
    //  wenn's gar nicht Ole ist, kann man sich die Berechnungen sparen
    //  (VisArea wird dann beim Save wieder zurueckgesetzt)

    if (eShellMode == SFX_CREATE_MODE_STANDARD)
        return;

    DBG_ASSERT(pViewData,"pViewData==0 bei ScDocShell::UpdateOle");

    Rectangle aOldArea = SfxInPlaceObject::GetVisArea();
    Rectangle aNewArea = aOldArea;

    BOOL bChange = FALSE;
    BOOL bEmbedded = aDocument.IsEmbedded();
    if (bEmbedded)
        aNewArea = aDocument.GetEmbeddedRect();
    else
    {
        USHORT nX = pViewData->GetPosX(SC_SPLIT_LEFT);
        USHORT nY = pViewData->GetPosY(SC_SPLIT_BOTTOM);
        Point aStart = aDocument.GetMMRect( nX,nY, nX,nY, 0 ).TopLeft();
        aNewArea.SetPos(aStart);
        if (bSnapSize)
            aDocument.SnapVisArea(aNewArea);

        if ( pViewData->GetTabNo() != aDocument.GetVisibleTab() )
        {
            aDocument.SetVisibleTab( pViewData->GetTabNo() );
            bChange = TRUE;
        }
    }

    if (aNewArea != aOldArea)
    {
        SetVisAreaOrSize( aNewArea, TRUE ); // hier muss auch der Start angepasst werden
        bChange = TRUE;
    }

    if (bChange)
        DataChanged( SvDataType() );        //! passiert auch bei SetModified
}

//
//  Style-Krempel fuer Organizer etc.
//

#pragma SEG_FUNCDEF(docsh6_05)

SfxStyleSheetBasePool* __EXPORT ScDocShell::GetStyleSheetPool()
{
    return (SfxStyleSheetBasePool*)aDocument.GetStyleSheetPool();
}


#pragma SEG_FUNCDEF(docsh6_07)

//  nach dem Laden von Vorlagen aus einem anderen Dokment (LoadStyles, Insert)
//  muessen die SetItems (ATTR_PAGE_HEADERSET, ATTR_PAGE_FOOTERSET) auf den richtigen
//  Pool umgesetzt werden, bevor der Quell-Pool geloescht wird.

void lcl_AdjustPool( SfxStyleSheetBasePool* pStylePool )
{
    pStylePool->SetSearchMask(SFX_STYLE_FAMILY_PAGE, 0xffff);
    SfxStyleSheetBase *pStyle = pStylePool->First();
    while ( pStyle )
    {
        SfxItemSet& rStyleSet = pStyle->GetItemSet();

        const SfxPoolItem* pItem;
        if (rStyleSet.GetItemState(ATTR_PAGE_HEADERSET,FALSE,&pItem) == SFX_ITEM_SET)
        {
            SfxItemSet& rSrcSet = ((SvxSetItem*)pItem)->GetItemSet();
            SfxItemSet* pDestSet = new SfxItemSet(*rStyleSet.GetPool(),rSrcSet.GetRanges());
            pDestSet->Put(rSrcSet);
            rStyleSet.Put(SvxSetItem(ATTR_PAGE_HEADERSET,pDestSet));
        }
        if (rStyleSet.GetItemState(ATTR_PAGE_FOOTERSET,FALSE,&pItem) == SFX_ITEM_SET)
        {
            SfxItemSet& rSrcSet = ((SvxSetItem*)pItem)->GetItemSet();
            SfxItemSet* pDestSet = new SfxItemSet(*rStyleSet.GetPool(),rSrcSet.GetRanges());
            pDestSet->Put(rSrcSet);
            rStyleSet.Put(SvxSetItem(ATTR_PAGE_FOOTERSET,pDestSet));
        }

        pStyle = pStylePool->Next();
    }
}

#pragma SEG_FUNCDEF(docsh6_08)

void __EXPORT ScDocShell::LoadStyles( SfxObjectShell &rSource )
{
    aDocument.StylesToNames();

    SfxObjectShell::LoadStyles(rSource);
    lcl_AdjustPool( GetStyleSheetPool() );      // SetItems anpassen

    aDocument.UpdStlShtPtrsFrmNms();

        //  Hoehen anpassen

    VirtualDevice aVDev;
    Point aLogic = aVDev.LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = aLogic.X() / 1000.0;
    double nPPTY = aLogic.Y() / 1000.0;
    Fraction aZoom(1,1);
    USHORT nTabCnt = aDocument.GetTableCount();
    for (USHORT nTab=0; nTab<nTabCnt; nTab++)
        aDocument.SetOptimalHeight( 0,MAXROW, nTab,0, &aVDev, nPPTX,nPPTY, aZoom,aZoom, FALSE );

        //  Paint

    PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID | PAINT_LEFT );
}

#pragma SEG_FUNCDEF(docsh6_0b)

BOOL __EXPORT ScDocShell::Insert( SfxObjectShell &rSource,
                                USHORT nSourceIdx1, USHORT nSourceIdx2, USHORT nSourceIdx3,
                                USHORT &nIdx1, USHORT &nIdx2, USHORT &nIdx3, USHORT &rIdxDeleted )
{
    BOOL bRet = SfxObjectShell::Insert( rSource, nSourceIdx1, nSourceIdx2, nSourceIdx3,
                                            nIdx1, nIdx2, nIdx3, rIdxDeleted );
    if (bRet)
        lcl_AdjustPool( GetStyleSheetPool() );      // SetItems anpassen

    return bRet;
}

#pragma SEG_FUNCDEF(docsh6_09)

void ScDocShell::UpdateLinks()
{
    SvxLinkManager* pLinkManager = aDocument.GetLinkManager();
    USHORT nCount;
    USHORT i;
    StrCollection aNames;

    // nicht mehr benutzte Links raus

    nCount = pLinkManager->GetLinks().Count();
    for (i=nCount; i>0; )
    {
        --i;
        SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
        if (pBase->ISA(ScTableLink))
        {
            ScTableLink* pTabLink = (ScTableLink*)pBase;
            if (pTabLink->IsUsed())
            {
                StrData* pData = new StrData(pTabLink->GetFileName());
                if (!aNames.Insert(pData))
                    delete pData;
            }
            else        // nicht mehr benutzt -> loeschen
            {
                pTabLink->SetAddUndo(TRUE);
                pLinkManager->Remove(i);
            }
        }
    }


    // neue Links eintragen

    nCount = aDocument.GetTableCount();
    for (i=0; i<nCount; i++)
        if (aDocument.IsLinked(i))
        {
            String aDocName = aDocument.GetLinkDoc(i);
            String aFltName = aDocument.GetLinkFlt(i);
            String aOptions = aDocument.GetLinkOpt(i);
            BOOL bThere = FALSE;
            for (USHORT j=0; j<i && !bThere; j++)               // im Dokument mehrfach?
                if (aDocument.IsLinked(j)
                        && aDocument.GetLinkDoc(j) == aDocName
                        && aDocument.GetLinkFlt(j) == aFltName
                        && aDocument.GetLinkOpt(j) == aOptions)
                    bThere = TRUE;

            if (!bThere)                                        // schon als Filter eingetragen?
            {
                StrData* pData = new StrData(aDocName);
                if (!aNames.Insert(pData))
                {
                    delete pData;
                    bThere = TRUE;
                }
            }
            if (!bThere)
            {
                ScTableLink* pLink = new ScTableLink( this, aDocName, aFltName, aOptions );
                pLink->SetInCreate( TRUE );
                pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aDocName, &aFltName );
                pLink->Update();
                pLink->SetInCreate( FALSE );
            }
        }
}

#pragma SEG_FUNCDEF(docsh6_0a)

BOOL ScDocShell::ReloadTabLinks()
{
    SvxLinkManager* pLinkManager = aDocument.GetLinkManager();

    BOOL bAny = FALSE;
    USHORT nCount = pLinkManager->GetLinks().Count();
    for (USHORT i=0; i<nCount; i++ )
    {
        SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
        if (pBase->ISA(ScTableLink))
        {
            ScTableLink* pTabLink = (ScTableLink*)pBase;
//          pTabLink->SetAddUndo(FALSE);        //! Undo's zusammenfassen
            pTabLink->SetPaint(FALSE);          //  Paint nur einmal am Ende
            pTabLink->Update();
            pTabLink->SetPaint(TRUE);
//          pTabLink->SetAddUndo(TRUE);
            bAny = TRUE;
        }
    }

    if ( bAny )
    {
        //  Paint nur einmal
        PostPaint( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB),
                                    PAINT_GRID | PAINT_TOP | PAINT_LEFT );

        SetDocumentModified();
    }

    return TRUE;        //! Fehler erkennen
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.25  2000/09/17 14:09:00  willem.vandorp
    OpenOffice header added.

    Revision 1.24  2000/08/31 16:38:22  willem.vandorp
    Header and footer replaced

    Revision 1.23  2000/04/20 16:51:18  nn
    unicode changes

    Revision 1.22  2000/02/11 12:25:25  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.21  1998/08/06 08:37:38  TJ
    include


      Rev 1.20   06 Aug 1998 10:37:38   TJ
   include

      Rev 1.19   11 Jun 1998 11:24:18   NN
   wenn VisArea von aussen gesetzt, Position beibehalten

      Rev 1.18   27 Jan 1998 12:42:26   TJ
   include

      Rev 1.17   05 Dec 1997 19:51:28   ANK
   Includes geaendert

      Rev 1.16   22 Sep 1997 18:08:40   NN
   #44070# ReloadTabLinks: nur 1x Paint

      Rev 1.15   10 Sep 1997 21:16:32   NN
   IsOle: Abfrage per CreateMode

      Rev 1.14   17 Sep 1996 16:10:38   NN
   Bei Link auch Filter-Options speichern

      Rev 1.13   09 Aug 1996 20:36:06   NN
   Svx-Includes aus scitems.hxx raus

      Rev 1.12   08 Aug 1996 11:03:30   NF
   includes...

      Rev 1.11   22 Apr 1996 14:57:30   NN
   UpdateOle: SetVisArea immer wenn geaendert

      Rev 1.10   20 Mar 1996 14:24:50   NN
   Zoom fuer X und Y getrennt (wegen OLE)

      Rev 1.9   09 Mar 1996 18:18:56   NN
   bei Insert und LoadStyles hinterher die Pools anpassen

      Rev 1.8   06 Mar 1996 19:17:22   NN
   ReloadTabLinks

      Rev 1.7   05 Mar 1996 19:52:32   NN
   UpdateOle: auch SetVisibleTab

      Rev 1.6   01 Mar 1996 17:04:54   NN
   UpdateLinks: unbenutzte Links loeschen

      Rev 1.5   27 Feb 1996 15:01:46   NN
   UpdateLinks

      Rev 1.4   09 Jan 1996 19:25:54   NN
   bei SetVisArea das Embedded-Rechteck (tuerkiser Rahmen) updaten

      Rev 1.3   10 Dec 1995 15:20:50   TRI
   define entfernt

      Rev 1.2   23 Nov 1995 16:11:02   NN
   DataChanged bei SetVisArea

      Rev 1.1   21 Nov 1995 15:49:14   TLX
   Insert wandert in den SFX

      Rev 1.0   17 Nov 1995 15:45:44   NN
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE
