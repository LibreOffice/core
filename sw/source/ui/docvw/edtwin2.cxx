/*************************************************************************
 *
 *  $RCSfile: edtwin2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:35 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <tools/ref.hxx>
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef PRODUCT
#include <stdio.h>
#endif

#ifndef _HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _IPENV_HXX //autogen
#include <so3/ipenv.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _TXTRFMRK_HXX //autogen wg. SwTxtRefMark
#include <txtrfmrk.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen wg. SwFmtRefMark
#include <fmtrfmrk.hxx>
#endif

#ifndef _SVX_FLDITEM_HXX
#   ifndef ITEMID_FIELD
#       ifndef _EEITEM_HXX //autogen
#           include <svx/eeitem.hxx>
#       endif
#       define ITEMID_FIELD EE_FEATURE_FIELD  /* wird fuer #include <flditem.hxx> benoetigt */
#   endif
#   ifndef _SVX_FLDITEM_HXX //autogen
#       include <svx/flditem.hxx>
#   endif
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#define _EEITEMID_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif


#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _DPAGE_HXX
#include <dpage.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _SHDWCRSR_HXX
#include <shdwcrsr.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _FMTFTN_HXX
#include <fmtftn.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _REDLENUM_HXX
#include <redlenum.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif

#ifndef _DOCVW_HRC
#include <docvw.hrc>
#endif
#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif

#define C2S(cChar) UniString::CreateFromAscii(cChar)

/*--------------------------------------------------------------------
    Beschreibung:   KeyEvents
 --------------------------------------------------------------------*/
void lcl_GetRedlineHelp( const SwRedline& rRedl, String& rTxt, BOOL bBalloon )
{
    USHORT nResId = 0;
    switch( rRedl.GetType() )
    {
    case REDLINE_INSERT:    nResId = STR_REDLINE_INSERT; break;
    case REDLINE_DELETE:    nResId = STR_REDLINE_DELETE; break;
    case REDLINE_FORMAT:    nResId = STR_REDLINE_FORMAT; break;
    case REDLINE_TABLE:     nResId = STR_REDLINE_TABLE; break;
    case REDLINE_FMTCOLL:   nResId = STR_REDLINE_FMTCOLL; break;
    }

    if( nResId )
    {
        const DateTime& rDT = rRedl.GetTimeStamp();
        const International& rIntl = Application::GetAppInternational();
        rTxt = SW_RESSTR( nResId );
        (rTxt += C2S(": " )) += rRedl.GetAuthorString();
        (rTxt += C2S(" - " )) += rIntl.GetDate( rDT );
        (rTxt += ' ') += rIntl.GetTime( rDT, FALSE, FALSE );
        if( bBalloon && rRedl.GetComment().Len() )
            ( rTxt += '\n' ) += rRedl.GetComment();
    }
}


void SwEditWin::RequestHelp(const HelpEvent &rEvt)
{
    BOOL bWeiter = TRUE;
    SwWrtShell &rSh = rView.GetWrtShell();
    SET_CURR_SHELL(&rSh);
    String sTxt;
    Point aPos( PixelToLogic( ScreenToOutputPixel( rEvt.GetMousePosPixel() ) ));
    BOOL bBalloon = rEvt.GetMode() & HELPMODE_BALLOON;

    SdrView *pSdrView = rSh.GetDrawView();

    if( rEvt.GetMode() & ( HELPMODE_QUICK | HELPMODE_BALLOON ) )
    {
        if( pSdrView )
        {
            SdrPageView* pPV = pSdrView->GetPageViewPvNum(0);
            bWeiter = ((SwDPage*)pPV->GetPage())->RequestHelp(this,
                                                            pSdrView, rEvt);
        }
    }

    if( bWeiter && rEvt.GetMode() & ( HELPMODE_QUICK | HELPMODE_BALLOON ))
    {
        SwRect aFldRect;
        SwContentAtPos aCntntAtPos( SwContentAtPos::SW_FIELD |
                                    SwContentAtPos::SW_INETATTR |
                                    SwContentAtPos::SW_FTN |
                                    SwContentAtPos::SW_REDLINE |
                                    SwContentAtPos::SW_TOXMARK |
                                    SwContentAtPos::SW_REFMARK |
#ifndef PRODUCT
                                    SwContentAtPos::SW_TABLEBOXVALUE |
                        ( bBalloon ? SwContentAtPos::SW_CURR_ATTRS : 0) |
#endif
                                    SwContentAtPos::SW_TABLEBOXFML );

        if( rSh.GetContentAtPos( aPos, aCntntAtPos, FALSE, &aFldRect ) )
        {
            switch( aCntntAtPos.eCntntAtPos )
            {
            case SwContentAtPos::SW_TABLEBOXFML:
                ( sTxt = C2S("= ")) += ((SwTblBoxFormula*)aCntntAtPos.aFnd.pAttr)->GetFormula();
                break;
#ifndef PRODUCT
            case SwContentAtPos::SW_TABLEBOXVALUE:
            {
                ByteString sTmp;
                sprintf( sTmp.AllocBuffer( 30 ), "%E",
                    ((SwTblBoxValue*)aCntntAtPos.aFnd.pAttr)->GetValue() );
                sTxt = UniString(sTmp.GetBuffer(), gsl_getSystemTextEncoding());
            }
            break;
            case SwContentAtPos::SW_CURR_ATTRS:
                sTxt = aCntntAtPos.sStr;
                break;
#endif

            case SwContentAtPos::SW_INETATTR:
                sTxt = ((SfxStringItem*)aCntntAtPos.aFnd.pAttr)->GetValue();
                sTxt = URIHelper::removePassword( sTxt,
                                        INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_WITH_CHARSET );

                break;

            case SwContentAtPos::SW_FTN:
                if( aCntntAtPos.pFndTxtAttr && aCntntAtPos.aFnd.pAttr )
                {
                    const SwFmtFtn* pFtn = (SwFmtFtn*)aCntntAtPos.aFnd.pAttr;
                    pFtn->GetFtnText( sTxt );
                    sTxt.Insert( SW_RESSTR( pFtn->IsEndNote()
                                    ? STR_ENDNOTE : STR_FTNNOTE ), 0 );
                }
                break;

            case SwContentAtPos::SW_REDLINE:
                ::lcl_GetRedlineHelp( *aCntntAtPos.aFnd.pRedl, sTxt, bBalloon );
                break;

            case SwContentAtPos::SW_TOXMARK:
                sTxt = aCntntAtPos.sStr;
                if( sTxt.Len() && aCntntAtPos.pFndTxtAttr )
                {
                    const SwTOXType* pTType = aCntntAtPos.pFndTxtAttr->
                                        GetTOXMark().GetTOXType();
                    if( pTType && pTType->GetTypeName().Len() )
                    {
                        sTxt.Insert( C2S(": "), 0 );
                        sTxt.Insert( pTType->GetTypeName(), 0 );
                    }
                }
                break;
            case SwContentAtPos::SW_REFMARK:
                if(aCntntAtPos.aFnd.pAttr)
                {
                    sTxt = SW_RES(STR_CONTENT_TYPE_SINGLE_REFERENCE);
                    sTxt += C2S(": ");
                    sTxt += ((const SwFmtRefMark*)aCntntAtPos.aFnd.pAttr)->GetRefName();
                }
            break;

            default:
                {
                    const SwField* pFld = aCntntAtPos.aFnd.pFld;
                    switch( pFld->Which() )
                    {
                    case RES_SETEXPFLD:
                    case RES_TABLEFLD:
                    case RES_GETEXPFLD:
                    {
                        USHORT nOldSubType = pFld->GetSubType();
                        ((SwField*)pFld)->SetSubType(SUB_CMD);
                        sTxt = pFld->Expand();
                        ((SwField*)pFld)->SetSubType(nOldSubType);
                    }
                    break;

                    case RES_POSTITFLD:
                    case RES_INPUTFLD:  // BubbleHelp, da der Hinweis ggf ziemlich lang sein kann
                        bBalloon = TRUE;
                        /* no break */
                    case RES_JUMPEDITFLD:
                        sTxt = pFld->GetPar2();
                        break;

                    case RES_DBFLD:
                        sTxt = ((SwDBField*)pFld)->GetCntnt(TRUE);
                        break;

                    case RES_USERFLD:
                    case RES_HIDDENTXTFLD:
                        sTxt = pFld->GetPar1();
                        break;

                    case RES_DOCSTATFLD:
                        break;

                    case RES_MACROFLD:
                        sTxt = ((const SwMacroField*)pFld)->GetMacro();
                        break;

                    case RES_GETREFFLD:
                        sTxt = ((SwGetRefField*)pFld)->GetSetRefName();
                        break;
                    }

                    if( !sTxt.Len() )
                    {
                        aCntntAtPos.eCntntAtPos = SwContentAtPos::SW_REDLINE;
                        if( rSh.GetContentAtPos( aPos, aCntntAtPos, FALSE, &aFldRect ) )
                            ::lcl_GetRedlineHelp( *aCntntAtPos.aFnd.pRedl,
                                                    sTxt, bBalloon );
                    }
                }
            }
            if (sTxt.Len() )
            {
                if( bBalloon )
                    Help::ShowBalloon( this, rEvt.GetMousePosPixel(), sTxt );
                else
                {
                    // dann zeige die Hilfe mal an:
                    Rectangle aRect( aFldRect.SVRect() );
                    Point aPt( OutputToScreenPixel( LogicToPixel( aRect.TopLeft() )));
                    aRect.Left()   = aPt.X();
                    aRect.Top()    = aPt.Y();
                    aPt = OutputToScreenPixel( LogicToPixel( aRect.BottomRight() ));
                    aRect.Right()  = aPt.X();
                    aRect.Bottom() = aPt.Y();
                    Help::ShowQuickHelp( this, aRect, sTxt );
                }
            }

            bWeiter = FALSE;
        }
    }

/*
aktuelle Zeichenvorlage anzeigen?
    if( bWeiter && rEvt.GetMode() & ( HELPMODE_QUICK | HELPMODE_BALLOON ))
    {
        SwCharFmt* pChrFmt = rSh.GetCurCharFmt();

    }
*/
    if( bWeiter && pSdrView && rEvt.GetMode() & ( HELPMODE_QUICK | HELPMODE_BALLOON ))
    {
        SdrViewEvent aVEvt;
        SdrHitKind eHit = pSdrView->PickAnything(aPos, aVEvt);
        const SvxURLField *pField;
        SdrObject* pObj = NULL;

        if ((pField = aVEvt.pURLField) != 0)
        {
            // URL-Feld getroffen
            if (pField)
            {
                pObj = aVEvt.pObj;
                sTxt = pField->GetURL();

                bWeiter = FALSE;
            }
        }
        if (bWeiter && eHit == SDRHIT_TEXTEDIT)
        {
            // URL-Feld in zum Editieren ge”ffneten DrawText-Objekt suchen
            OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
            const SvxFieldItem* pFieldItem;
            const SdrTextObj* pTextObj = NULL;

            if (pSdrView->HasMarkedObj())
            {
                const SdrMarkList& rMarkList = pSdrView->GetMarkList();

                if (rMarkList.GetMarkCount() == 1)
                    pObj = rMarkList.GetMark(0)->GetObj();
            }

            if (pObj && pObj->ISA(SdrTextObj) && pOLV &&
                    (pFieldItem = pOLV->GetFieldUnderMousePointer()) != 0)
            {
                const SvxFieldData* pField = pFieldItem->GetField();

                if (pField && pField->ISA(SvxURLField))
                {
                    sTxt = ((const SvxURLField*) pField)->GetURL();
                    bWeiter = FALSE;
                }
            }
        }
        if (sTxt.Len() && pObj)
        {
            sTxt = URIHelper::removePassword( sTxt, INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_WITH_CHARSET );

            Rectangle aLogicPix = LogicToPixel(pObj->GetLogicRect());
            Rectangle aScreenRect(OutputToScreenPixel(aLogicPix.TopLeft()),
                                OutputToScreenPixel(aLogicPix.BottomRight()));

            if (bBalloon)
                Help::ShowBalloon(this, rEvt.GetMousePosPixel(), aScreenRect, sTxt);
            else
                Help::ShowQuickHelp(this, aScreenRect, sTxt);
        }
    }

    if( bWeiter )
        Window::RequestHelp( rEvt );
}

void  SwEditWin::Paint(const Rectangle& rRect)
{
#if defined(MYDEBUG)
    // StartUp-Statistik
    if ( pTickList )
    {
        SYSTICK( "Start SwEditWin::Paint" );
        READ_FIRST_TICKS()
        FLUSH_TICKS()
    }
#endif

    SwWrtShell* pWrtShell = GetView().GetWrtShellPtr();
    if(!pWrtShell)
        return;
    BOOL bPaintShadowCrsr = FALSE;
    if( pShadCrsr )
    {
        Rectangle aRect( pShadCrsr->GetRect());
        // liegt vollstaendig drin?
        if( rRect.IsInside( aRect ) )
            // dann aufheben
            delete pShadCrsr, pShadCrsr = 0;
        else if( rRect.IsOver( aRect ))
        {
            // liegt irgendwie drueber, dann ist alles ausserhalb geclippt
            // und wir muessen den "inneren Teil" am Ende vom Paint
            // wieder sichtbar machen. Sonst kommt es zu Paintfehlern!
            bPaintShadowCrsr = TRUE;
        }
    }

    SwDocShell* pDocShell = GetView().GetDocShell();
    SvInPlaceEnvironment *pIpEnv =  pDocShell ?
                                  pDocShell->GetIPEnv() : 0;
    if ( pIpEnv && pIpEnv->GetRectsChangedLockCount() )
        //Wir stehen in Groessenverhandlungen (MM), Paint verzoegern
        Invalidate( rRect );
    else if ( GetView().GetVisArea().GetWidth()  <= 0 ||
              GetView().GetVisArea().GetHeight() <= 0 )
        Invalidate( rRect );
    else
        pWrtShell->Paint( rRect );

    if( bPaintShadowCrsr )
        pShadCrsr->Paint();
}


/***********************************************************************

        $Log: not supported by cvs2svn $
        Revision 1.77  2000/09/18 16:05:23  willem.vandorp
        OpenOffice header added.

        Revision 1.76  2000/08/31 11:38:26  jp
        RequestHelp: get the correct string for the TableValue

        Revision 1.75  2000/08/15 18:58:30  jp
        Task #77160#: use URIHelper class instead of GetUrlNoPass

        Revision 1.74  2000/04/18 15:18:17  os
        UNICODE

        Revision 1.73  2000/03/03 15:16:59  os
        StarView remainders removed

        Revision 1.72  2000/02/11 14:44:54  hr
        #70473# changes for unicode ( patched by automated patchtool )

        Revision 1.71  1999/07/22 07:05:40  OS
        #67715# QuickHelp for RefMarks


      Rev 1.70   22 Jul 1999 09:05:40   OS
   #67715# QuickHelp for RefMarks

      Rev 1.69   14 Jun 1999 13:28:24   JP
   Task #66520#: show TOX-TypeName and TOX-Entry by RequestHelp

      Rev 1.68   02 Jun 1999 11:04:42   JP
   Task #66520#: Verzeichniseintraege ohne Bereich per TipHilfe anzeigen

      Rev 1.67   29 Jan 1999 11:48:08   MH
   add: header

      Rev 1.66   09 Oct 1998 17:01:28   JP
   Bug #57741#: neue ResourceIds

      Rev 1.65   17 Jul 1998 15:23:52   OM
   #53182# TipHilfe fuer Macrofelder

      Rev 1.64   09 Jun 1998 15:31:26   OM
   VC-Controls entfernt

      Rev 1.63   02 Apr 1998 11:38:54   OS
   SET_CURR_SHELL im RequestHelp

      Rev 1.62   25 Mar 1998 18:44:34   JP
   Bug #48368#: RequestHelp - falls Felder keine TipHilfe haben, dann frage mal nach Redlines

      Rev 1.61   13 Mar 1998 14:09:36   OM
   #48150# BubbleHelp fuer Eingabefeld

      Rev 1.60   04 Feb 1998 19:47:42   JP
   neu: SetRedlineComment - Kommentar am RedlineObject setzen

      Rev 1.59   23 Jan 1998 14:53:02   JP
   neu: Quick/Tip-Hilfe fuer Fuss-/Endnoten

      Rev 1.58   15 Jan 1998 13:16:14   JP
   neu: TipHilfe fuer Redlines

      Rev 1.57   07 Jan 1998 13:37:36   OS
   Hosentraeger und Guertel: im Paint Existenz der wrtsh an der view testen #46436#

      Rev 1.56   28 Nov 1997 18:43:00   MA
   includes

      Rev 1.55   25 Nov 1997 10:33:02   MA
   includes

      Rev 1.54   11 Nov 1997 14:04:06   MA
   precomp entfernt

      Rev 1.53   06 Nov 1997 20:45:48   JP
   RequestHelp: fuer !PRODUCT bei Balloon die akt. Attributierung anzeigen

      Rev 1.52   03 Nov 1997 16:13:46   JP
   neu: Optionen/-Page/Basic-Schnittst. fuer ShadowCursor

      Rev 1.51   24 Oct 1997 18:37:02   JP
   neu: ShadowCursor

      Rev 1.50   02 Oct 1997 15:22:32   OM
   Feldumstellung

      Rev 1.49   15 Sep 1997 09:58:48   OM
   Auf vorhandene SdrView pruefen

      Rev 1.48   12 Sep 1997 17:19:00   OM
   Quickhelp fuer Drawtext-Objekte

      Rev 1.47   11 Aug 1997 15:54:08   OM
   #42625# Kein Passwort in Tip-Hilfe anzeigen

      Rev 1.46   11 Aug 1997 12:20:24   MH
   chg: header, ::Paint nach edtwin2.cxx verschoben

      Rev 1.45   07 Aug 1997 15:00:16   OM
   Headerfile-Umstellung

      Rev 1.44   05 Aug 1997 12:27:56   MH
   chg: header

      Rev 1.43   03 Jul 1997 13:09:32   OV
   #41226# VCMouseButtonDown: bVCAction=FALSE, wenn kein VCControl getroffen

      Rev 1.42   02 May 1997 19:54:28   NF
   includes...

      Rev 1.41   11 Apr 1997 11:25:48   MA
   includes

      Rev 1.40   03 Apr 1997 17:22:28   TRI
   includes

      Rev 1.39   15 Feb 1997 14:54:54   JP
   TipHilfe: TabellenFormel/-Value anzeigen

      Rev 1.38   08 Nov 1996 11:40:22   HJS
   include w.g. positivdefine

      Rev 1.37   07 Nov 1996 14:32:58   MA
   immer Balloon fuer Notizen

      Rev 1.36   05 Nov 1996 15:34:42   JP
   GotoRefMark: Parameter erweitert fuer erweiterte RefMarks

      Rev 1.35   22 Oct 1996 14:03:08   JP
   RequestHelp: RefFelder umgestellt auf neu SS

      Rev 1.34   25 Sep 1996 14:11:00   OM
   Neue Datenbanktrenner

      Rev 1.33   02 Sep 1996 18:42:36   JP
   INetFeld entfernt

      Rev 1.32   28 Aug 1996 11:31:22   OS
   PCH-Header korrigiert

      Rev 1.31   28 Aug 1996 11:25:48   OS
   includes

      Rev 1.30   26 Aug 1996 12:35:16   OS
   wieder mit PCH

      Rev 1.29   14 Aug 1996 15:23:10   JP
   svdraw.hxx entfernt

      Rev 1.28   08 Aug 1996 10:03:16   JP
   GetFldAtPos ersetzt durch GetCntntAtPos

      Rev 1.27   07 Aug 1996 14:59:12   JP
   Umstellung fuer Upd. 330

      Rev 1.26   01 Jul 1996 15:27:56   HJS
   define raus

      Rev 1.25   13 Jun 1996 14:44:02   MA
   splitt sihxx

      Rev 1.24   24 May 1996 16:49:28   OM
   QuickHelp von ExpressionFields wandeln

      Rev 1.23   06 May 1996 09:26:10   OS
   kein PCH wg. positiv-define

      Rev 1.22   30 Apr 1996 17:51:14   HJS
   docfilt darf nicht ausdefined sein

      Rev 1.21   22 Mar 1996 15:16:50   HJS
   umstellung 311

**********************************************************************/

