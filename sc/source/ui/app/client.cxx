/*************************************************************************
 *
 *  $RCSfile: client.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:53 $
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
#define _SVMEMPOOL_HXX
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
#define _HELP_HXX
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
#define _STATUS_HXX
#define _SVTCTRL3_HXX
//#define _DIALOG_HXX
//#define _MSGBOX_HXX
//#define _SYSDLG_HXX
//#define _FILDLG_HXX ***
//#define _PRNDLG_HXX
#define _COLDLG_HXX
//#define _TABDLG_HXX
#define _MENU_HXX
//#define _GDIMTF_HXX
//#define _POLY_HXX
//#define _ACCEL_HXX
//#define _GRAPH_HXX
#define _SOUND_HXX

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
#define _SFX_BINDINGS_HXX
#define _SFXDISPATCH_HXX  //???
//#define _SFXMSG_HXX ***
//#define _SFXOBJFACE_HXX ***
//#define _SFXREQUEST_HXX
#define _SFXMACRO_HXX

//sfxdoc.hxx
//#define _SFXDOC_HXX
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX
#define _SFX_DOCFILT_HACK_HXX
#define _SFX_FCONTNR_HXX
#define _SFXDOCFILE_HXX
//#define _SFXFRAME_HXX
//#define _VIEWFAC_HXX
//#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX
#define _MDIFRM_HXX
#define _SFX_IPFRM_HXX
#define _SFX_INTERNO_HXX
#define _FSETOBJSH_HXX

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

//inet
#define _INETINDP_HXX
#define _INETIMP_HXX
#define _INETWRAP_HXX
#define _INETKEEP_HXX
//#define _PLUGMGR_HXX
#ifdef WIN
#define _URLOBJ_HXX
#define _INETSESS_HXX
#define _INETMAIN_HXX
#define _INET_HXX
#endif

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include "segmentc.hxx"
#endif

// INCLUDE ---------------------------------------------------------------

#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif

//#include <socore.hxx>
//#include <soipenv.hxx>
/*
#ifndef _SOREF_HXX
#include <soref.hxx>
#endif
*/
#include <sot/sotref.hxx>
#include <so3/ipobj.hxx>
#include <so3/persist.hxx>
#include <so3/embobj.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <svx/svdograf.hxx>
#include <sim2/simdll.hxx>


/*
#include <svdmodel.hxx>
#include <svdrwetc.hxx>
#include <svdrwobj.hxx>
#include <svdrwobx.hxx>
#include <svdpage.hxx>
#include <svdrview.hxx>
#include <simdll.hxx>
*/
#include "client.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"

SEG_EOFGLOBALS()

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(client_01)

ScClient::ScClient( ScTabViewShell* pViewShell, Window* pDraw, SdrModel* pSdrModel ) :
    SfxInPlaceClient( pViewShell, pDraw ),
    pModel( pSdrModel ),
    pGrafEdit( 0 )
{
}

#pragma SEG_FUNCDEF(client_02)

__EXPORT ScClient::~ScClient()
{
}

#pragma SEG_FUNCDEF(client_05)

SdrOle2Obj* ScClient::GetDrawObj()
{
    SvEmbeddedObject* pMyObj = GetEmbedObj();
    SvInfoObject* pInfoObj = GetViewShell()->GetViewFrame()->GetObjectShell()->Find( pMyObj );
    SdrOle2Obj* pOle2Obj = NULL;
//  String aName = pMyObj->GetName()->GetName();

    String aName;

    if ( pInfoObj )
        aName = pInfoObj->GetObjName();
    else
        DBG_ERROR( "IP-Object not found :-/" );

    USHORT nPages = pModel->GetPageCount();
    for (USHORT nPNr=0; nPNr<nPages && !pOle2Obj; nPNr++)
    {
        SdrPage* pPage = pModel->GetPage(nPNr);
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject && !pOle2Obj)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                if ( ((SdrOle2Obj*)pObject)->GetName() == aName )
                    pOle2Obj = (SdrOle2Obj*)pObject;
            }
            pObject = aIter.Next();
        }
    }
    return pOle2Obj;
}

#pragma SEG_FUNCDEF(client_03)

void __EXPORT ScClient::RequestObjAreaPixel( const Rectangle& rObjRect )
{
    SfxViewShell* pSfxViewSh = GetViewShell();
    ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
    if (!pViewSh)
    {
        DBG_ERROR("RequestObjAreaPixel: das ist nicht meine ViewShell");
        SfxInPlaceClient::RequestObjAreaPixel( rObjRect );
        return;
    }

    //  Position anpassen

    Rectangle aRect = rObjRect;
    Window* pWin = pViewSh->GetActiveWin();
    Rectangle aLogicRect( pWin->PixelToLogic( aRect.TopLeft() ),
                            pWin->PixelToLogic( aRect.GetSize() ) );

    BOOL bChange = FALSE;
    USHORT nTab = pViewSh->GetViewData()->GetTabNo();
    SdrPage* pPage = pModel->GetPage(nTab);
    if (pPage)
    {
        Size aSize = pPage->GetSize();
        if (aLogicRect.Right() >= aSize.Width())                    // rechts
        {
            long nDiff = aLogicRect.Right() - aSize.Width() + 1;
            aLogicRect.Left() -= nDiff;
            aLogicRect.Right() -= nDiff;
            bChange = TRUE;
        }
        if (aLogicRect.Bottom() >= aSize.Height())                  // unten
        {
            long nDiff = aLogicRect.Bottom() - aSize.Height() + 1;
            aLogicRect.Top() -= nDiff;
            aLogicRect.Bottom() -= nDiff;
            bChange = TRUE;
        }
    }
    else
        DBG_ERROR("RequestObjAreaPixel: Page ist weg");

    if (aLogicRect.Left() < 0)                                      // links
    {
        aLogicRect.Right() -= aLogicRect.Left();
        aLogicRect.Left() = 0;
        bChange = TRUE;
    }
    if (aLogicRect.Top() < 0)                                       // oben
    {
        aLogicRect.Bottom() -= aLogicRect.Top();
        aLogicRect.Top() = 0;
        bChange = TRUE;
    }

        //  wieder in Pixel umrechnen

    if (bChange)
        aRect = Rectangle( pWin->LogicToPixel( aLogicRect.TopLeft() ),
                            pWin->LogicToPixel( aLogicRect.GetSize() ) );

    //  Basisklasse

    SfxInPlaceClient::RequestObjAreaPixel( aRect );

    //  Position und Groesse ins Dokument uebernehmen

    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        //  das ist schon das skalierte Rechteck (wie im Draw-Model)

        Rectangle aOld = pDrawObj->GetLogicRect();
        if ( aLogicRect != aOld )
        {
            //  #56590# Rundungsfehler vermeiden - nur, wenn mindestens 1 Pixel Unterschied
            //  (getrennt fuer Position und Groesse)

            Size aOnePixel = pWin->PixelToLogic( Size(1,1) );
            Size aLogicSize = aLogicRect.GetSize();
            Rectangle aNew = aOld;
            Size aNewSize = aNew.GetSize();

            if ( Abs( aLogicRect.Left() - aOld.Left() ) >= aOnePixel.Width() )
                aNew.SetPos( Point( aLogicRect.Left(), aNew.Top() ) );
            if ( Abs( aLogicRect.Top() - aOld.Top() ) >= aOnePixel.Height() )
                aNew.SetPos( Point( aNew.Left(), aLogicRect.Top() ) );

            if ( Abs( aLogicSize.Width() - aNewSize.Width() ) >= aOnePixel.Width() )
                aNewSize.Width() = aLogicSize.Width();
            if ( Abs( aLogicSize.Height() - aNewSize.Height() ) >= aOnePixel.Height() )
                aNewSize.Height() = aLogicSize.Height();
            aNew.SetSize( aNewSize );

            if ( aNew != aOld )                     // veraendert nur, wenn mindestens 1 Pixel
                pDrawObj->SetLogicRect( aNew );
        }
    }

    //
    //  evtl. scrollen, um Objekt sichtbar zu halten
    //

    if (pDrawObj)
        pViewSh->ScrollToObject( pDrawObj );
}

#pragma SEG_FUNCDEF(client_04)

void __EXPORT ScClient::ViewChanged( USHORT nAspect )
{
    SfxInPlaceClient::ViewChanged( nAspect );

    SvEmbeddedObject* pObj = GetEmbedObj();
    Rectangle aObjVisArea = OutputDevice::LogicToLogic( pObj->GetVisArea(),
                                                pObj->GetMapUnit(), MAP_100TH_MM );
    Size aVisSize = aObjVisArea.GetSize();

    //  Groesse ins Dokument uebernehmen

    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        Rectangle aLogicRect = pDrawObj->GetLogicRect();

//      SvClientData* pClientData = GetEnv();
        SvClientData* pClientData = GetClientData();
        if (pClientData)
        {
            Fraction aFractX = pClientData->GetScaleWidth();
            Fraction aFractY = pClientData->GetScaleHeight();
            aFractX *= aVisSize.Width();
            aFractY *= aVisSize.Height();
            aVisSize = Size( (long) aFractX, (long) aFractY );      // skaliert fuer Draw-Model

                //  pClientData->SetObjArea vor pDrawObj->SetLogicRect, damit keine
                //  falschen Skalierungen ausgerechnet werden:

            Rectangle aObjArea = aLogicRect;
            aObjArea.SetSize( aObjVisArea.GetSize() );          // Dokument-Groesse vom Server
            pClientData->SetObjArea( aObjArea );
        }

            //  Groesse im Draw-Model setzen, dabei wird Scale neu eingestellt

        if ( aLogicRect.GetSize() != aVisSize )
        {
            aLogicRect.SetSize( aVisSize );
            pDrawObj->SetLogicRect( aLogicRect );
        }
    }
}

#pragma SEG_FUNCDEF(client_08)

void __EXPORT ScClient::MakeViewData()
{
    SfxInPlaceClient::MakeViewData();

    SvClientData* pClientData = GetClientData();
    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pClientData && pDrawObj)
    {
        //  Groesse und Scale wie in tabvwshb

        Rectangle aRect = pDrawObj->GetLogicRect();
        Size aDrawSize = aRect.GetSize();
        Size aOleSize = GetEmbedObj()->GetVisArea().GetSize();

            // sichtbarer Ausschnitt wird nur inplace veraendert!
        aRect.SetSize( aOleSize );
        pClientData->SetObjArea( aRect );

        Fraction aScaleWidth (aDrawSize.Width(),  aOleSize.Width() );
        Fraction aScaleHeight(aDrawSize.Height(), aOleSize.Height() );
        aScaleWidth.ReduceInaccurate(10);       // kompatibel zum SdrOle2Obj
        aScaleHeight.ReduceInaccurate(10);
        pClientData->SetSizeScale(aScaleWidth,aScaleHeight);
    }
}

#pragma SEG_FUNCDEF(client_06)

void __EXPORT ScClient::MakeVisible()
{
    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        SfxViewShell* pSfxViewSh = GetViewShell();
        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
        if (pViewSh)
            pViewSh->ScrollToObject( pDrawObj );
    }
}

#pragma SEG_FUNCDEF(client_0a)

//  Mit Optimierung gibt es Abstuerze beim Deaktivieren von Grafik-Image-Objekten

#ifdef WNT
#pragma optimize ( "", off )
#endif

void lcl_ReplaceObject( SdrOle2Obj* pGrafOle, SdrGrafObj* pGrafObj,
                        SfxViewShell* pSfxViewSh )
{
    //  das Ole-Objekt wieder durch das Grafikobjekt ersetzen

    ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
    if (pViewSh)
    {
        SdrView* pSdrView = pViewSh->GetSdrView();
        SdrPageView* pPV = pSdrView->GetPageViewPvNum(0);

        if ( pGrafOle )
        {
            SvInPlaceObjectRef aIPObj = pGrafOle->GetObjRef();
            Rectangle          aRect  = pGrafOle->GetLogicRect();

            // #41302# ReplaceObject ohne zu markieren, weil's aus dem
            // MarkListHasChanged heraus gerufen wird
            pSdrView->ReplaceObject( pGrafOle, *pPV, pGrafObj, FALSE );
            pSdrView->EndUndo();
            pGrafObj->SetLogicRect(aRect);
            pGrafObj->SetGraphic(SimDLL::GetGraphic(aIPObj));
        }
        else                            // zwischendurch geloescht ?
        {
            DBG_ERROR("Grafik-Ole-Objekt ist weg");

            pSdrView->EndUndo();        //  damit das Undo nicht offen bleibt
                                        //! anderen Text fuer Undo setzen ???
        }
    }
}

#ifdef WNT
#pragma optimize ( "", on )
#endif

#pragma SEG_FUNCDEF(client_09)

void __EXPORT ScClient::UIActivate( BOOL bActivate )
{
    SvInPlaceClientRef aIPClient( this );   // nicht aus versehen zwischendrin loeschen

    SfxInPlaceClient::UIActivate(bActivate);

    if ( !bActivate && pGrafEdit )          // wurde eine Grafik bearbeitet?
    {
        SdrOle2Obj* pGrafOle = GetDrawObj();
        SdrGrafObj* pGrafObj = pGrafEdit;
        pGrafEdit = NULL;                   // vor lcl_ReplaceObject

        lcl_ReplaceObject( pGrafOle, pGrafObj, GetViewShell() );
    }

    if ( !bActivate )       //  Chart-Daten-Hervorhebung aufheben
    {
        SfxViewShell* pSfxViewSh = GetViewShell();
        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pSfxViewSh );
        if (pViewSh)
        {
            pViewSh->ClearHighlightRanges();

            //  Move an der ViewShell soll eigentlich vom Sfx gerufen werden, wenn sich
            //  das Frame-Window wegen unterschiedlicher Toolboxen o.ae. verschiebt
            //  (um nicht aus Versehen z.B. Zeichenobjekte zu verschieben, #56515#).
            //  Dieser Mechanismus funktioniert aber momentan nicht, darum hier der Aufruf
            //  per Hand (im Move wird verglichen, ob die Position wirklich geaendert ist).
            pViewSh->ForceMove();
        }
    }
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.44  2000/09/17 14:08:53  willem.vandorp
    OpenOffice header added.

    Revision 1.43  2000/08/31 16:38:17  willem.vandorp
    Header and footer replaced

    Revision 1.42  2000/02/11 12:21:23  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.41  1999/04/19 17:04:18  NN
    #65014# Objekte/Charts koennen auch gruppiert vorkommen


      Rev 1.40   19 Apr 1999 19:04:18   NN
   #65014# Objekte/Charts koennen auch gruppiert vorkommen

      Rev 1.39   26 Mar 1999 18:12:16   HJS
   sim ist tot

      Rev 1.38   03 Mar 1999 18:08:40   NN
   #62107# GetDrawObj: Name vom SdrOle2Obj statt vom IPObj

      Rev 1.37   27 Nov 1998 11:30:16   NN
   #41302#/#56515# ReplaceObject ohne bMark, ForceMove beim Deaktivieren

      Rev 1.36   16 Oct 1998 21:09:12   NN
   #57916# ClearHighlightRanges beim Deaktivieren

      Rev 1.35   13 Oct 1998 16:00:34   NN
   #56590# RequestObjAreaPixel: Rundungsfehler bei SetLogicRect vermeiden

      Rev 1.34   05 Dec 1997 20:08:20   ANK
   Includes geaendert

      Rev 1.33   04 Sep 1997 20:29:48   RG
   change header

      Rev 1.32   29 Apr 1997 17:25:50   HJS
   includes/defines

      Rev 1.31   10 Jan 1997 13:45:44   NN
   lcl_ReplaceObject ohne Optimierung

      Rev 1.30   04 Dec 1996 14:50:04   NN
   Image-Objekt wieder durch Grafik ersetzen jetzt in UIActivate

      Rev 1.29   25 Oct 1996 11:39:28   TRI
   socore.hxx

      Rev 1.28   23 Aug 1996 11:31:40   NF
   clooks

      Rev 1.27   22 Aug 1996 10:58:20   NF
   clooks

      Rev 1.26   15 Aug 1996 20:24:50   HJS
   define raus

      Rev 1.25   15 Aug 1996 19:12:32   HJS
   clooks

      Rev 1.24   12 Aug 1996 11:37:04   NF
   clooks

      Rev 1.23   06 Aug 1996 09:10:54   NF
   clooks

      Rev 1.22   01 Aug 1996 14:24:08   NF
   clooks

      Rev 1.21   03 Jul 1996 14:51:32   NN
   ScrollToObject an der View statt DoScroll

      Rev 1.20   30 May 1996 14:53:24   NF
   CLOOKS ... !

      Rev 1.19   18 Apr 1996 17:15:24   NN
   313: MakeViewData

      Rev 1.18   18 Apr 1996 15:43:32   NN
   313: MakeVisible

      Rev 1.17   27 Mar 1996 14:58:04   NN
   RequestObjAreaPixel: Bereich auf Seite anpassen, Scrollen wenn noetig

      Rev 1.16   21 Mar 1996 11:43:18   ER
   SV311 soref changes

      Rev 1.15   14 Mar 1996 14:42:30   NN
   #25327# ViewChanged: erst SetObjArea, dann SetLogicRect (wegen Scale)

      Rev 1.14   29 Feb 1996 18:59:40   NF
   CLOOKS

      Rev 1.13   23 Feb 1996 12:18:40   NF
   CLOOKS

      Rev 1.12   16 Feb 1996 14:48:10   MO
   306er SO2-Aenderungen

      Rev 1.11   26 Jan 1996 09:57:10   NN
   RequestObjAreaPixel: statt PixelToLogic mit Rechteck
   Position und Groesse getrennt umrechnen

      Rev 1.10   22 Jan 1996 16:24:02   NN
   #24481# Scale beruecksichtigen

      Rev 1.9   18 Nov 1995 18:21:34   NN
   pDrawObject nicht mehr merken, stattdessen suchen

      Rev 1.8   08 Nov 1995 13:04:04   MO
   301-Aenderungen

      Rev 1.7   26 Sep 1995 18:10:40   JN
   DrawObject merken

      Rev 1.6   14 Sep 1995 12:01:44   NN
   soipenv.hxx

      Rev 1.5   06 Sep 1995 09:44:10   NN
   bei ViewChanged Position behalten

      Rev 1.4   04 Sep 1995 14:09:00   NN
   SetObjArea am Environment statt SetObjSize (2.55)

      Rev 1.3   09 Aug 1995 13:12:02   NN
   bGrafEdit-Flag

      Rev 1.2   31 Jul 1995 10:26:44   NN
   Container Anpassung in RequestObjAreaPixel

      Rev 1.1   13 Jul 1995 13:10:38   NN
   SdrOle2Obj als Member fuer Groessenaenderungen

      Rev 1.0   13 Jul 1995 12:19:30   NN
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


