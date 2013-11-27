/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <tools/ref.hxx>
#include <hintids.hxx>
#include <doc.hxx>
#ifdef DBG_UTIL
#include <stdio.h>
#endif
#include <vcl/help.hxx>
#include <svl/stritem.hxx>
#include <unotools/securityoptions.hxx>
#include <tools/urlobj.hxx>
#include <txtrfmrk.hxx>
#include <fmtrfmrk.hxx>
#include <editeng/flditem.hxx>
#include <svl/urihelper.hxx>
#include <svx/svdotext.hxx>
#include <editeng/outliner.hxx>
#include <svl/itemiter.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <edtwin.hxx>
#include <dpage.hxx>
#include <shellres.hxx>
#include <docufld.hxx>
#include <dbfld.hxx>
#include <reffld.hxx>
#include <cellatr.hxx>
#include <shdwcrsr.hxx>
#include <fmtcol.hxx>
#include <charfmt.hxx>
#include <fmtftn.hxx>
#include <redline.hxx>
#include <tox.hxx>
#include <txttxmrk.hxx>
#include <uitool.hxx>
#include <viewopt.hxx>
#include <docvw.hrc>
#include <utlui.hrc>
#include <PostItMgr.hxx>
#include <fmtfld.hxx>
#include <IDocumentMarkAccess.hxx>
#include <ndtxt.hxx>
#include <svx/svdlegacy.hxx>

/*--------------------------------------------------------------------
    Beschreibung:   KeyEvents
 --------------------------------------------------------------------*/
static void lcl_GetRedlineHelp( const SwRedline& rRedl, String& rTxt, sal_Bool bBalloon )
{
    sal_uInt16 nResId = 0;
    switch( rRedl.GetType() )
    {
    case nsRedlineType_t::REDLINE_INSERT:   nResId = STR_REDLINE_INSERT; break;
    case nsRedlineType_t::REDLINE_DELETE:   nResId = STR_REDLINE_DELETE; break;
    case nsRedlineType_t::REDLINE_FORMAT:   nResId = STR_REDLINE_FORMAT; break;
    case nsRedlineType_t::REDLINE_TABLE:        nResId = STR_REDLINE_TABLE; break;
    case nsRedlineType_t::REDLINE_FMTCOLL:  nResId = STR_REDLINE_FMTCOLL; break;
    }

    if( nResId )
    {
        rTxt = SW_RESSTR( nResId );
        rTxt.AppendAscii( RTL_CONSTASCII_STRINGPARAM(": " ));
        rTxt += rRedl.GetAuthorString();
        rTxt.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " - " ));
        rTxt += GetAppLangDateTimeString( rRedl.GetTimeStamp() );
        if( bBalloon && rRedl.GetComment().Len() )
            ( rTxt += '\n' ) += rRedl.GetComment();
    }
}


void SwEditWin::RequestHelp(const HelpEvent &rEvt)
{
    SwWrtShell &rSh = rView.GetWrtShell();
    sal_Bool bQuickBalloon = 0 != (rEvt.GetMode() & ( HELPMODE_QUICK | HELPMODE_BALLOON ));
    if(bQuickBalloon && !rSh.GetViewOptions()->IsShowContentTips())
        return;
    sal_Bool bWeiter = sal_True;
    SET_CURR_SHELL(&rSh);
    String sTxt;
    Point aPos( PixelToLogic( ScreenToOutputPixel( rEvt.GetMousePosPixel() ) ));
    sal_Bool bBalloon = static_cast< sal_Bool >(rEvt.GetMode() & HELPMODE_BALLOON);

    SdrView *pSdrView = rSh.GetDrawView();

    if( bQuickBalloon )
    {
        if( pSdrView )
        {
            SdrPageView* pPV = pSdrView->GetSdrPageView();
            SwDPage* pPage = pPV ? ((SwDPage*)&pPV->getSdrPageFromSdrPageView()) : 0;
            bWeiter = pPage && pPage->RequestHelp(this, pSdrView, rEvt);
        }
    }

    if( bWeiter && bQuickBalloon)
    {
        SwRect aFldRect;
        sal_uInt16 nStyle = 0; // style of quick help
        SwContentAtPos aCntntAtPos( SwContentAtPos::SW_FIELD |
                                    SwContentAtPos::SW_INETATTR |
                                    SwContentAtPos::SW_FTN |
                                    SwContentAtPos::SW_REDLINE |
                                    SwContentAtPos::SW_TOXMARK |
                                    SwContentAtPos::SW_REFMARK |
                                    SwContentAtPos::SW_SMARTTAG |
#ifdef DBG_UTIL
                                    SwContentAtPos::SW_TABLEBOXVALUE |
                                    ( bBalloon ? SwContentAtPos::SW_CURR_ATTRS : 0) |
#endif
                                    SwContentAtPos::SW_TABLEBOXFML );

        if( rSh.GetContentAtPos( aPos, aCntntAtPos, sal_False, &aFldRect ) )
        {
            switch( aCntntAtPos.eCntntAtPos )
            {
            case SwContentAtPos::SW_TABLEBOXFML:
                sTxt.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "= " ));
                sTxt += ((SwTblBoxFormula*)aCntntAtPos.aFnd.pAttr)->GetFormula();
                break;
#ifdef DBG_UTIL
            case SwContentAtPos::SW_TABLEBOXVALUE:
                {
                    sTxt = UniString(
                        ByteString::CreateFromDouble(
                        ((SwTblBoxValue*)aCntntAtPos.aFnd.pAttr)->GetValue()  )
                        , gsl_getSystemTextEncoding());
                }
                break;
            case SwContentAtPos::SW_CURR_ATTRS:
                sTxt = aCntntAtPos.sStr;
                break;
#endif

            case SwContentAtPos::SW_INETATTR:
                {
                    sTxt = ((SfxStringItem*)aCntntAtPos.aFnd.pAttr)->GetValue();
                    sTxt = URIHelper::removePassword( sTxt,
                        INetURLObject::WAS_ENCODED,
                        INetURLObject::DECODE_UNAMBIGUOUS);
                    //#i63832# remove the link target type
                    xub_StrLen nFound = sTxt.Search(cMarkSeperator);
                    if( nFound != STRING_NOTFOUND && (++nFound) < sTxt.Len() )
                    {
                        String sSuffix( sTxt.Copy(nFound) );
                        if( sSuffix.EqualsAscii( pMarkToTable ) ||
                            sSuffix.EqualsAscii( pMarkToFrame ) ||
                            sSuffix.EqualsAscii( pMarkToRegion ) ||
                            sSuffix.EqualsAscii( pMarkToOutline ) ||
                            sSuffix.EqualsAscii( pMarkToText ) ||
                            sSuffix.EqualsAscii( pMarkToGraphic ) ||
                            sSuffix.EqualsAscii( pMarkToOLE ))
                            sTxt = sTxt.Copy( 0, nFound - 1);
                    }
                    // special handling if target is a cross-reference bookmark
                    {
                        String sTmpSearchStr = sTxt.Copy( 1, sTxt.Len() );
                        IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
                        IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findBookmark( sTmpSearchStr );
                        if ( ppBkmk != pMarkAccess->getBookmarksEnd()
                             && IDocumentMarkAccess::GetType( *(ppBkmk->get()) ) == IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK )
                        {
                            SwTxtNode* pTxtNode = ppBkmk->get()->GetMarkStart().nNode.GetNode().GetTxtNode();
                            if ( pTxtNode )
                            {
                                sTxt = pTxtNode->GetExpandTxt( 0, pTxtNode->Len(), true, true );

                                if( sTxt.Len() )
                                {
                                    sTxt.EraseAllChars( 0xad );
                                    for( sal_Unicode* p = sTxt.GetBufferAccess(); *p; ++p )
                                    {
                                        if( *p < 0x20 )
                                            *p = 0x20;
                                        else if(*p == 0x2011)
                                            *p = '-';
                                    }
                                }
                            }
                        }
                    }

                    sal_Bool bExecHyperlinks = rView.GetDocShell()->IsReadOnly();
                    if ( !bExecHyperlinks )
                    {
                        SvtSecurityOptions aSecOpts;
                        bExecHyperlinks = !aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );

                        if ( !bExecHyperlinks )
                        {
                            sTxt.InsertAscii( ": ", 0 );
                            sTxt.Insert( ViewShell::GetShellRes()->aHyperlinkClick, 0 );
                        }
                    }
                    break;
                }
            case SwContentAtPos::SW_SMARTTAG:
                {
                    sTxt = SW_RESSTR(STR_SMARTTAG_CLICK);

                    KeyCode aCode( KEY_SPACE );
                    KeyCode aModifiedCode( KEY_SPACE, KEY_MOD1 );
                    String aModStr( aModifiedCode.GetName() );
                    aModStr.SearchAndReplace( aCode.GetName(), String() );
                    aModStr.SearchAndReplaceAllAscii( "+", String() );
                    sTxt.SearchAndReplaceAllAscii( "%s", aModStr );
                }
                break;

            case SwContentAtPos::SW_FTN:
                if( aCntntAtPos.pFndTxtAttr && aCntntAtPos.aFnd.pAttr )
                {
                    const SwFmtFtn* pFtn = (SwFmtFtn*)aCntntAtPos.aFnd.pAttr;
                    pFtn->GetFtnText( sTxt );
                    sTxt.Insert( SW_RESSTR( pFtn->IsEndNote()
                        ? STR_ENDNOTE : STR_FTNNOTE ), 0 );
                    if( aCntntAtPos.IsInRTLText() )
                        nStyle |= QUICKHELP_BIDI_RTL;
                }
                break;

            case SwContentAtPos::SW_REDLINE:
                lcl_GetRedlineHelp( *aCntntAtPos.aFnd.pRedl, sTxt, bBalloon );
                break;

            case SwContentAtPos::SW_TOXMARK:
                sTxt = aCntntAtPos.sStr;
                if( sTxt.Len() && aCntntAtPos.pFndTxtAttr )
                {
                    const SwTOXType* pTType = aCntntAtPos.pFndTxtAttr->
                        GetTOXMark().GetTOXType();
                    if( pTType && pTType->GetTypeName().Len() )
                    {
                        sTxt.InsertAscii( ": ", 0 );
                        sTxt.Insert( pTType->GetTypeName(), 0 );
                    }
                }
                break;

            case SwContentAtPos::SW_REFMARK:
                if(aCntntAtPos.aFnd.pAttr)
                {
                    sTxt = SW_RES(STR_CONTENT_TYPE_SINGLE_REFERENCE);
                    sTxt.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": "));
                    sTxt += ((const SwFmtRefMark*)aCntntAtPos.aFnd.pAttr)->GetRefName();
                }
                break;

            default:
                {
                    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
                    if(!pModOpt->IsHideFieldTips())
                    {
                        const SwField* pFld = aCntntAtPos.aFnd.pFld;
                        switch( pFld->Which() )
                        {
                        case RES_SETEXPFLD:
                        case RES_TABLEFLD:
                        case RES_GETEXPFLD:
                            {
                                sal_uInt16 nOldSubType = pFld->GetSubType();
                                ((SwField*)pFld)->SetSubType(nsSwExtendedSubType::SUB_CMD);
                                sTxt = pFld->ExpandField(true);
                                ((SwField*)pFld)->SetSubType(nOldSubType);
                            }
                            break;

                        case RES_POSTITFLD:
                            {
                                break;
                            }
                        case RES_INPUTFLD:  // BubbleHelp, da der Hinweis ggf ziemlich lang sein kann
                            bBalloon = sal_True;
                            /* no break */
                        case RES_JUMPEDITFLD:
                            sTxt = pFld->GetPar2();
                            break;

                        case RES_DBFLD:
                            sTxt = pFld->GetFieldName();
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
                            {
                                const SwGetRefField* pRefFld( dynamic_cast<const SwGetRefField*>(pFld) );
                                ASSERT( pRefFld,
                                    "<SwEditWin::RequestHelp(..)> - unexpected type of <pFld>" );
                                if ( pRefFld )
                                {
                                    if ( pRefFld->IsRefToHeadingCrossRefBookmark() ||
                                        pRefFld->IsRefToNumItemCrossRefBookmark() )
                                    {
                                        sTxt = pRefFld->GetExpandedTxtOfReferencedTxtNode();
                                        if ( sTxt.Len() > 80  )
                                        {
                                            sTxt.Erase( 80 );
                                            sTxt += '.';
                                            sTxt += '.';
                                            sTxt += '.';
                                        }
                                    }
                                    else
                                    {
                                        sTxt = ((SwGetRefField*)pFld)->GetSetRefName();
                                    }
                                }
                            }
                            break;
                        }
                    }

                    if( !sTxt.Len() )
                    {
                        aCntntAtPos.eCntntAtPos = SwContentAtPos::SW_REDLINE;
                        if( rSh.GetContentAtPos( aPos, aCntntAtPos, sal_False, &aFldRect ) )
                            lcl_GetRedlineHelp( *aCntntAtPos.aFnd.pRedl, sTxt, bBalloon );
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
                    Help::ShowQuickHelp( this, aRect, sTxt, nStyle );
                }
            }

            bWeiter = sal_False;
        }
        if( bWeiter )
        {
            sal_uInt8 nTabCols = rSh.WhichMouseTabCol(aPos);
            sal_uInt16 nTabRes = 0;
            switch(nTabCols)
            {
                case SW_TABCOL_HORI:
                case SW_TABCOL_VERT:
                    nTabRes = STR_TABLE_COL_ADJUST;
                    break;
                case SW_TABROW_HORI:
                case SW_TABROW_VERT:
                    nTabRes = STR_TABLE_ROW_ADJUST;
                    break;
                case SW_TABSEL_HORI:
                case SW_TABSEL_HORI_RTL:
                case SW_TABSEL_VERT:
                    nTabRes = STR_TABLE_SELECT_ALL;
                    break;
                case SW_TABROWSEL_HORI:
                case SW_TABROWSEL_HORI_RTL:
                case SW_TABROWSEL_VERT:
                    nTabRes = STR_TABLE_SELECT_ROW;
                    break;
                case SW_TABCOLSEL_HORI:
                case SW_TABCOLSEL_VERT:
                    nTabRes = STR_TABLE_SELECT_COL;
                    break;
            }
            if(nTabRes)
            {
                sTxt = SW_RESSTR(nTabRes);
                Size aTxtSize( GetTextWidth(sTxt), GetTextHeight());
                Rectangle aRect(rEvt.GetMousePosPixel(), aTxtSize);
                Help::ShowQuickHelp(this, aRect, sTxt);
            }
            bWeiter = sal_False;
        }
    }

/*
aktuelle Zeichenvorlage anzeigen?
    if( bWeiter && rEvt.GetMode() & ( HELPMODE_QUICK | HELPMODE_BALLOON ))
    {
        SwCharFmt* pChrFmt = rSh.GetCurCharFmt();

    }
*/
    if( bWeiter && pSdrView && bQuickBalloon)
    {
        SdrViewEvent aVEvt;
        SdrHitKind eHit = pSdrView->PickAnything(basegfx::B2DPoint(aPos.X(), aPos.Y()), aVEvt);
        const SvxURLField *pField;
        SdrObject* pObj = NULL;

        if(aVEvt.maURLField.Len())
        {
            // URLField hit
            pObj = aVEvt.mpObj;
            sTxt = aVEvt.maURLField;

                bWeiter = sal_False;
            }
        if (bWeiter && eHit == SDRHIT_TEXTEDIT)
        {
            // URL-Feld in zum Editieren ge?ffneten DrawText-Objekt suchen
            OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
            const SvxFieldItem* pFieldItem;

            pObj = pSdrView->getSelectedIfSingle();

            if (pObj && dynamic_cast< SdrTextObj* >(pObj) && pOLV &&
                    (pFieldItem = pOLV->GetFieldUnderMousePointer()) != 0)
            {
                pField = dynamic_cast<const SvxURLField*>(pFieldItem->GetField());

                if (pField )
                {
                    sTxt = ((const SvxURLField*) pField)->GetURL();
                    bWeiter = sal_False;
                }
            }
        }
        if (sTxt.Len() && pObj)
        {
            sTxt = URIHelper::removePassword( sTxt, INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS);

            Rectangle aLogicPix = LogicToPixel(sdr::legacy::GetLogicRect(*pObj));
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

void SwEditWin::PrePaint()
{
    SwWrtShell* pWrtShell = GetView().GetWrtShellPtr();

    if(pWrtShell)
    {
        pWrtShell->PrePaint();
    }
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
    sal_Bool bPaintShadowCrsr = sal_False;
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
            bPaintShadowCrsr = sal_True;
        }
    }
/*
    //TODO/LATER: what's the replacement for this? Do we need it?
    SwDocShell* pDocShell = GetView().GetDocShell();

  SvInPlaceEnvironment *pIpEnv =  pDocShell ?
                                  pDocShell->GetIPEnv() : 0;
    if ( pIpEnv && pIpEnv->GetRectsChangedLockCount() )
        //Wir stehen in Groessenverhandlungen (MM), Paint verzoegern
        Invalidate( rRect );
    else */
    if ( GetView().GetVisArea().GetWidth()  <= 0 ||
              GetView().GetVisArea().GetHeight() <= 0 )
        Invalidate( rRect );
    else
        pWrtShell->Paint( rRect );

    if( bPaintShadowCrsr )
        pShadCrsr->Paint();
}


