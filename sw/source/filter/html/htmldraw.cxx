/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include "hintids.hxx"
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <editeng/eeitem.hxx>

#ifndef _OUTLINER_HXX //autogen
#define _EEITEMID_HXX
#include <editeng/outliner.hxx>
#endif
#include <svx/xfillit.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <svx/svdpool.hxx>


#include "charatr.hxx"
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <fmtsrnd.hxx>
#include "ndtxt.hxx"
#include "doc.hxx"
#include "dcontact.hxx"
#include "poolfmt.hxx"
#include "swcss1.hxx"
#include "swhtml.hxx"
#include "wrthtml.hxx"

using namespace ::com::sun::star;


const sal_uInt32 HTML_FRMOPTS_MARQUEE   =
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_SPACE;

const sal_uInt32 HTML_FRMOPTS_MARQUEE_CSS1  =
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SPACE;

static HTMLOptionEnum aHTMLMarqBehaviorTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_BEHAV_scroll,     SDRTEXTANI_SCROLL       },
    { OOO_STRING_SVTOOLS_HTML_BEHAV_alternate,  SDRTEXTANI_ALTERNATE    },
    { OOO_STRING_SVTOOLS_HTML_BEHAV_slide,      SDRTEXTANI_SLIDE        },
    { 0,                        0                       }
};

static HTMLOptionEnum aHTMLMarqDirectionTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_AL_left,          SDRTEXTANI_LEFT         },
    { OOO_STRING_SVTOOLS_HTML_AL_right,         SDRTEXTANI_RIGHT        },
    { 0,                        0                       }
};

/*  */
void SwHTMLParser::InsertDrawObject( SdrObject* pNewDrawObj,
                                     const Size& rPixSpace,
                                     sal_Int16 eVertOri,
                                     sal_Int16 eHoriOri,
                                     SfxItemSet& rCSS1ItemSet,
                                     SvxCSS1PropertyInfo& rCSS1PropInfo,
                                     sal_Bool bHidden )
{
    // always on top of text.
    // but in invisible layer. <ConnectToLayout> will move the object
    // to the visible layer.
    pNewDrawObj->SetLayer( pDoc->GetInvisibleHeavenId() );

    SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                        RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    if( !IsNewDoc() )
        Reader::ResetFrmFmtAttrs( aFrmSet );

    sal_uInt16 nLeftSpace = 0, nRightSpace = 0, nUpperSpace = 0, nLowerSpace = 0;
    if( (rPixSpace.Width() || rPixSpace.Height()) && Application::GetDefaultDevice() )
    {
        Size aTwipSpc( rPixSpace.Width(), rPixSpace.Height() );
        aTwipSpc =
            Application::GetDefaultDevice()->PixelToLogic( aTwipSpc,
                                                MapMode(MAP_TWIP) );
        nLeftSpace = nRightSpace = (sal_uInt16)aTwipSpc.Width();
        nUpperSpace = nLowerSpace = (sal_uInt16)aTwipSpc.Height();
    }

    // linken/rechten Rand setzen
    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_LR_SPACE, sal_True, &pItem ) )
    {
        // Ggf. den Erstzeilen-Einzug noch plaetten
        const SvxLRSpaceItem *pLRItem = (const SvxLRSpaceItem *)pItem;
        SvxLRSpaceItem aLRItem( *pLRItem );
        aLRItem.SetTxtFirstLineOfst( 0 );
        if( rCSS1PropInfo.bLeftMargin )
        {
            nLeftSpace = static_cast< sal_uInt16 >(aLRItem.GetLeft());
            rCSS1PropInfo.bLeftMargin = sal_False;
        }
        if( rCSS1PropInfo.bRightMargin )
        {
            nRightSpace = static_cast< sal_uInt16 >(aLRItem.GetRight());
            rCSS1PropInfo.bRightMargin = sal_False;
        }
        rCSS1ItemSet.ClearItem( RES_LR_SPACE );
    }
    if( nLeftSpace || nRightSpace )
    {
        SvxLRSpaceItem aLRItem( RES_LR_SPACE );
        aLRItem.SetLeft( nLeftSpace );
        aLRItem.SetRight( nRightSpace );
        aFrmSet.Put( aLRItem );
    }

    // oberen/unteren Rand setzen
    if( SFX_ITEM_SET==rCSS1ItemSet.GetItemState( RES_UL_SPACE, sal_True, &pItem ) )
    {
        // Ggf. den Erstzeilen-Einzug noch plaetten
        const SvxULSpaceItem *pULItem = (const SvxULSpaceItem *)pItem;
        if( rCSS1PropInfo.bTopMargin )
        {
            nUpperSpace = pULItem->GetUpper();
            rCSS1PropInfo.bTopMargin = sal_False;
        }
        if( rCSS1PropInfo.bBottomMargin )
        {
            nLowerSpace = pULItem->GetLower();
            rCSS1PropInfo.bBottomMargin = sal_False;
        }

        rCSS1ItemSet.ClearItem( RES_UL_SPACE );
    }
    if( nUpperSpace || nLowerSpace )
    {
        SvxULSpaceItem aULItem( RES_UL_SPACE );
        aULItem.SetUpper( nUpperSpace );
        aULItem.SetLower( nLowerSpace );
        aFrmSet.Put( aULItem );
    }

    SwFmtAnchor aAnchor( FLY_AS_CHAR );
    if( SVX_CSS1_POS_ABSOLUTE == rCSS1PropInfo.ePosition &&
        SVX_CSS1_LTYPE_TWIP == rCSS1PropInfo.eLeftType &&
        SVX_CSS1_LTYPE_TWIP == rCSS1PropInfo.eTopType )
    {
        const SwStartNode *pFlySttNd =
            pPam->GetPoint()->nNode.GetNode().FindFlyStartNode();

        if( pFlySttNd )
        {
            aAnchor.SetType( FLY_AT_FLY );
            SwPosition aPos( *pFlySttNd );
            aAnchor.SetAnchor( &aPos );
        }
        else
        {
            aAnchor.SetType( FLY_AT_PAGE );
        }
        // #i26791# - direct positioning for <SwDoc::Insert(..)>
        pNewDrawObj->SetRelativePos( Point(rCSS1PropInfo.nLeft + nLeftSpace,
                                           rCSS1PropInfo.nTop + nUpperSpace) );
        aFrmSet.Put( SwFmtSurround(SURROUND_THROUGHT) );
    }
    else if( SVX_ADJUST_LEFT == rCSS1PropInfo.eFloat ||
             text::HoriOrientation::LEFT == eHoriOri )
    {
        aAnchor.SetType( FLY_AT_PARA );
        aFrmSet.Put( SwFmtSurround(bHidden ? SURROUND_THROUGHT
                                             : SURROUND_RIGHT) );
        // #i26791# - direct positioning for <SwDoc::Insert(..)>
        pNewDrawObj->SetRelativePos( Point(nLeftSpace, nUpperSpace) );
    }
    else if( text::VertOrientation::NONE != eVertOri )
    {
        aFrmSet.Put( SwFmtVertOrient( 0, eVertOri ) );
    }

    if (FLY_AT_PAGE == aAnchor.GetAnchorId())
    {
        aAnchor.SetPageNum( 1 );
    }
    else if( FLY_AT_FLY != aAnchor.GetAnchorId() )
    {
        aAnchor.SetAnchor( pPam->GetPoint() );
    }
    aFrmSet.Put( aAnchor );

    pDoc->Insert( *pPam, *pNewDrawObj, &aFrmSet, NULL );
}

/*  */

static void PutEEPoolItem( SfxItemSet &rEEItemSet,
                           const SfxPoolItem& rSwItem )
{

    sal_uInt16 nEEWhich = 0;

    switch( rSwItem.Which() )
    {
    case RES_CHRATR_COLOR:          nEEWhich = EE_CHAR_COLOR; break;
    case RES_CHRATR_CROSSEDOUT:     nEEWhich = EE_CHAR_STRIKEOUT; break;
    case RES_CHRATR_ESCAPEMENT:     nEEWhich = EE_CHAR_ESCAPEMENT; break;
    case RES_CHRATR_FONT:           nEEWhich = EE_CHAR_FONTINFO; break;
    case RES_CHRATR_CJK_FONT:       nEEWhich = EE_CHAR_FONTINFO_CJK; break;
    case RES_CHRATR_CTL_FONT:       nEEWhich = EE_CHAR_FONTINFO_CTL; break;
    case RES_CHRATR_FONTSIZE:       nEEWhich = EE_CHAR_FONTHEIGHT; break;
    case RES_CHRATR_CJK_FONTSIZE:   nEEWhich = EE_CHAR_FONTHEIGHT_CJK; break;
    case RES_CHRATR_CTL_FONTSIZE:   nEEWhich = EE_CHAR_FONTHEIGHT_CTL; break;
    case RES_CHRATR_KERNING:        nEEWhich = EE_CHAR_KERNING; break;
    case RES_CHRATR_POSTURE:        nEEWhich = EE_CHAR_ITALIC; break;
    case RES_CHRATR_CJK_POSTURE:    nEEWhich = EE_CHAR_ITALIC_CJK; break;
    case RES_CHRATR_CTL_POSTURE:    nEEWhich = EE_CHAR_ITALIC_CTL; break;
    case RES_CHRATR_UNDERLINE:      nEEWhich = EE_CHAR_UNDERLINE; break;
    case RES_CHRATR_WEIGHT:         nEEWhich = EE_CHAR_WEIGHT; break;
    case RES_CHRATR_CJK_WEIGHT:     nEEWhich = EE_CHAR_WEIGHT_CJK; break;
    case RES_CHRATR_CTL_WEIGHT:     nEEWhich = EE_CHAR_WEIGHT_CTL; break;
    case RES_BACKGROUND:
    case RES_CHRATR_BACKGROUND:
        {
            const SvxBrushItem& rBrushItem = (const SvxBrushItem&)rSwItem;
            rEEItemSet.Put( XFillStyleItem(XFILL_SOLID) );
            rEEItemSet.Put( XFillColorItem(aEmptyStr,
                            rBrushItem.GetColor()) );
        }
        break;
    }

    if( nEEWhich )
    {
        SfxPoolItem *pEEItem = rSwItem.Clone();
        pEEItem->SetWhich( nEEWhich );
        rEEItemSet.Put( *pEEItem );
        delete pEEItem;
    }
}

void SwHTMLParser::NewMarquee( HTMLTable *pCurTable )
{

    OSL_ENSURE( !pMarquee, "Marquee in Marquee???" );
    aContents.Erase();

    String aId, aStyle, aClass;

    long nWidth=0, nHeight=0;
    sal_Bool bPrcWidth = sal_False, bDirection = sal_False, bBGColor = sal_False;
    Size aSpace( 0, 0 );
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;
    SdrTextAniKind eAniKind = SDRTEXTANI_SCROLL;
    SdrTextAniDirection eAniDir = SDRTEXTANI_LEFT;
    sal_uInt16 nCount = 0, nDelay = 60;
    sal_Int16 nAmount = -6;
    Color aBGColor;

    const HTMLOptions *pHTMLOptions = GetOptions();
    sal_uInt16 nArrLen = pHTMLOptions->Count();
    for ( sal_uInt16 i=0; i<nArrLen; i++ )
    {
        const HTMLOption *pOption = (*pHTMLOptions)[i];
        switch( pOption->GetToken() )
        {
            case HTML_O_ID:
                aId = pOption->GetString();
                break;
            case HTML_O_STYLE:
                aStyle = pOption->GetString();
                break;
            case HTML_O_CLASS:
                aClass = pOption->GetString();
                break;

            case HTML_O_BEHAVIOR:
                eAniKind =
                    (SdrTextAniKind)pOption->GetEnum( aHTMLMarqBehaviorTable,
                                                      static_cast< sal_uInt16 >(eAniKind) );
                break;

            case HTML_O_BGCOLOR:
                pOption->GetColor( aBGColor );
                bBGColor = sal_True;
                break;

            case HTML_O_DIRECTION:
                eAniDir =
                    (SdrTextAniDirection)pOption->GetEnum( aHTMLMarqDirectionTable,
                                                      static_cast< sal_uInt16 >(eAniDir) );
                bDirection = sal_True;
                break;

            case HTML_O_LOOP:
                if( pOption->GetString().
                        EqualsIgnoreCaseAscii(OOO_STRING_SVTOOLS_HTML_LOOP_infinite) )
                {
                    nCount = 0;
                }
                else
                {
                    sal_uInt32 nLoop = pOption->GetSNumber();
                    nCount = (sal_uInt16)(nLoop>0 ? nLoop : 0 );
                }
                break;

            case HTML_O_SCROLLAMOUNT:
                nAmount = -((sal_Int16)pOption->GetNumber());
                break;

            case HTML_O_SCROLLDELAY:
                nDelay = (sal_uInt16)pOption->GetNumber();
                break;

            case HTML_O_WIDTH:
                // erstmal nur als Pixelwerte merken!
                nWidth = pOption->GetNumber();
                bPrcWidth = pOption->GetString().Search('%') != STRING_NOTFOUND;
                if( bPrcWidth && nWidth>100 )
                    nWidth = 100;
                break;

            case HTML_O_HEIGHT:
                // erstmal nur als Pixelwerte merken!
                nHeight = pOption->GetNumber();
                if( pOption->GetString().Search('%') != STRING_NOTFOUND )
                    nHeight = 0;
                break;

            case HTML_O_HSPACE:
                // erstmal nur als Pixelwerte merken!
                aSpace.Height() = pOption->GetNumber();
                break;

            case HTML_O_VSPACE:
                // erstmal nur als Pixelwerte merken!
                aSpace.Width() = pOption->GetNumber();
                break;

            case HTML_O_ALIGN:
                eVertOri =
                    pOption->GetEnum( aHTMLImgVAlignTable,
                                                    text::VertOrientation::TOP );
                eHoriOri =
                    pOption->GetEnum( aHTMLImgHAlignTable,
                                                    text::HoriOrientation::NONE );
                break;
        }
    }

    // Ein DrawTxtobj anlegen
    // #i52858# - method name changed
    SdrModel* pModel = pDoc->GetOrCreateDrawModel();

    SdrPage* pPg = pModel->GetPage( 0 );
    pMarquee = SdrObjFactory::MakeNewObject( SdrInventor,
                                             OBJ_TEXT, pPg, pModel );
    if( !pMarquee )
        return;

    pPg->InsertObject( pMarquee );

    if( aId.Len() )
        InsertBookmark( aId );

    // (Nur) Alternate leueft per Default von links nach rechts
    if( SDRTEXTANI_ALTERNATE==eAniKind && !bDirection )
        eAniDir = SDRTEXTANI_RIGHT;

    // die fuer das Scrollen benoetigten Attribute umsetzen
    sal_uInt16 aWhichMap[7] =   { XATTR_FILL_FIRST,   XATTR_FILL_LAST,
                              SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                              EE_CHAR_START,      EE_CHAR_END,
                              0 };
    SfxItemSet aItemSet( pModel->GetItemPool(), aWhichMap );
    aItemSet.Put( SdrTextAutoGrowWidthItem( sal_False ) );
    aItemSet.Put( SdrTextAutoGrowHeightItem( sal_True ) );
    aItemSet.Put( SdrTextAniKindItem( eAniKind ) );
    aItemSet.Put( SdrTextAniDirectionItem( eAniDir ) );
    aItemSet.Put( SdrTextAniCountItem( nCount ) );
    aItemSet.Put( SdrTextAniDelayItem( nDelay ) );
    aItemSet.Put( SdrTextAniAmountItem( nAmount ) );
    if( SDRTEXTANI_ALTERNATE==eAniKind )
    {
        // (Nur) Alternate startet und stoppt per default Inside
        aItemSet.Put( SdrTextAniStartInsideItem(sal_True) );
        aItemSet.Put( SdrTextAniStopInsideItem(sal_True) );
        if( SDRTEXTANI_LEFT==eAniDir )
            aItemSet.Put( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );
    }

    // die Default-Farbe (aus der Standard-Vorlage) setzen, damit ueberhaupt
    // eine sinnvolle Farbe gesetzt ist.
    const Color& rDfltColor =
        pCSS1Parser->GetTxtCollFromPool( RES_POOLCOLL_STANDARD )
            ->GetColor().GetValue();
    aItemSet.Put( SvxColorItem( rDfltColor, EE_CHAR_COLOR ) );

    // Die Attribute der aktuellen Absatzvorlage setzen
    sal_uInt16 nWhichIds[] =
    {
        RES_CHRATR_COLOR,   RES_CHRATR_CROSSEDOUT, RES_CHRATR_ESCAPEMENT,
        RES_CHRATR_FONT,    RES_CHRATR_FONTSIZE,   RES_CHRATR_KERNING,
        RES_CHRATR_POSTURE, RES_CHRATR_UNDERLINE,  RES_CHRATR_WEIGHT,
        RES_CHRATR_BACKGROUND,
        RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONTSIZE,
        RES_CHRATR_CJK_POSTURE, RES_CHRATR_CJK_WEIGHT,
        RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONTSIZE,
        RES_CHRATR_CTL_POSTURE, RES_CHRATR_CTL_WEIGHT,
        0
    };
    SwTxtNode const*const pTxtNd =
        pPam->GetPoint()->nNode.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        const SfxItemSet& rItemSet = pTxtNd->GetAnyFmtColl().GetAttrSet();
        const SfxPoolItem *pItem;
        for( sal_uInt16 i=0; nWhichIds[i]; i++ )
        {
            if( SFX_ITEM_SET == rItemSet.GetItemState( nWhichIds[i], sal_True, &pItem ) )
                PutEEPoolItem( aItemSet, *pItem );
        }
    }

    // die Attribute der Umgebung am Draw-Objekt setzen
    _HTMLAttr** pTbl = (_HTMLAttr**)&aAttrTab;
    for( sal_uInt16 nCnt = sizeof( _HTMLAttrTable ) / sizeof( _HTMLAttr* );
         nCnt--; ++pTbl )
    {
        _HTMLAttr *pAttr = *pTbl;
        if( pAttr )
            PutEEPoolItem( aItemSet, pAttr->GetItem() );
    }

    if( bBGColor )
    {
        aItemSet.Put( XFillStyleItem(XFILL_SOLID) );
        aItemSet.Put( XFillColorItem(aEmptyStr, aBGColor) );
    }

    // Styles parsen (funktioniert hier nur fuer Attribute, die auch
    // am Zeichen-Objekt gesetzt werden koennen)
    SfxItemSet aStyleItemSet( pDoc->GetAttrPool(),
                              pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass )  &&
        ParseStyleOptions( aStyle, aId, aClass, aStyleItemSet, aPropInfo ) )
    {
        SfxItemIter aIter( aStyleItemSet );

        const SfxPoolItem *pItem = aIter.FirstItem();
        while( pItem )
        {
            PutEEPoolItem( aItemSet, *pItem );
            pItem = aIter.NextItem();
        }
    }

    // jetzt noch die Groesse setzen
    Size aTwipSz( bPrcWidth ? 0 : nWidth, nHeight );
    if( (aTwipSz.Width() || aTwipSz.Height()) && Application::GetDefaultDevice() )
    {
        aTwipSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aTwipSz, MapMode( MAP_TWIP ) );
    }

    if( SVX_CSS1_LTYPE_TWIP== aPropInfo.eWidthType )
    {
        aTwipSz.Width() = aPropInfo.nWidth;
        nWidth = 1; // != 0;
        bPrcWidth = sal_False;
    }
    if( SVX_CSS1_LTYPE_TWIP== aPropInfo.eHeightType )
        aTwipSz.Height() = aPropInfo.nHeight;

    bFixMarqueeWidth = sal_False;
    if( !nWidth || bPrcWidth )
    {
        if( pTable )
        {
            if( !pCurTable )
            {
                // Die Laufschrift steht in einer Tabelle, aber nicht
                // in einer Zelle. Da jetzt keine vernuenftige Zuordung
                // zu einer Zelle moeglich ist, passen wir hir die
                // Breite dem Inhalt der Laufschrift an.
                bFixMarqueeWidth = sal_True;
            }
            else if( !nWidth )
            {
                // Da wir wissen, in welcher Zelle die Laufschrift ist,
                // koennen wir die Breite auch anpassen. Keine Breitenangabe
                // wird wie 100% behandelt.
                nWidth = 100;
                bPrcWidth = sal_True;
            }
            aTwipSz.Width() = MINLAY;
        }
        else
        {
            long nBrowseWidth = GetCurrentBrowseWidth();
            aTwipSz.Width() = !nWidth ? nBrowseWidth
                                      : (nWidth*nBrowseWidth) / 100;
        }
    }

    // Die Hoehe ist nur eine Mindest-Hoehe
    if( aTwipSz.Height() < MINFLY )
        aTwipSz.Height() = MINFLY;
    aItemSet.Put( SdrTextMinFrameHeightItem( aTwipSz.Height() ) );

    pMarquee->SetMergedItemSetAndBroadcast(aItemSet);

    if( aTwipSz.Width() < MINFLY )
        aTwipSz.Width() = MINFLY;
    pMarquee->SetLogicRect( Rectangle( 0, 0, aTwipSz.Width(), aTwipSz.Height() ) );

    // und das Objekt in das Dok einfuegen
    InsertDrawObject( pMarquee, aSpace, eVertOri, eHoriOri, aStyleItemSet,
                      aPropInfo );

    // Das Zeichen-Objekt der Tabelle bekanntmachen. Ist ein bisserl
    // umstaendlich, weil noch ueber den Parser gegangen wird, obwohl die
    // Tabelle bekannt ist, aber anderenfalls muesste man die Tabelle
    // oeffentlich machen, und das ist auch nicht schoen. Das globale
    // pTable kann uebrigens auch nicht verwendet werden, denn die
    // Laufschrift kann sich auch mal in einer Sub-Tabelle befinden.
    if( pCurTable && bPrcWidth)
        RegisterDrawObjectToTable( pCurTable, pMarquee, (sal_uInt8)nWidth );
}

void SwHTMLParser::EndMarquee()
{
    OSL_ENSURE( pMarquee && OBJ_TEXT==pMarquee->GetObjIdentifier(),
            "kein Marquee oder falscher Typ" );

    if( bFixMarqueeWidth )
    {
        // Da es keine fixe Hoehe gibt, das Text-Objekt erstmal breiter
        // als den Text machen, damit nicht umgebrochen wird.
        const Rectangle& rOldRect = pMarquee->GetLogicRect();
        pMarquee->SetLogicRect( Rectangle( rOldRect.TopLeft(),
                                           Size( USHRT_MAX, 240 ) ) );
    }

    // den gesammelten Text einfuegen
    ((SdrTextObj*)pMarquee)->SetText( aContents );
    pMarquee->SetMergedItemSetAndBroadcast( pMarquee->GetMergedItemSet() );

    if( bFixMarqueeWidth )
    {
        // die Groesse dem Text anpassen.
        ((SdrTextObj*)pMarquee)->FitFrameToTextSize();
    }

    aContents.Erase();
    pMarquee = 0;
}

void SwHTMLParser::InsertMarqueeText()
{
    OSL_ENSURE( pMarquee && OBJ_TEXT==pMarquee->GetObjIdentifier(),
            "kein Marquee oder falscher Typ" );

    // das akteulle Textstueck an den Text anhaengen
    aContents += aToken;
}

void SwHTMLParser::ResizeDrawObject( SdrObject* pObj, SwTwips nWidth )
{
    OSL_ENSURE( OBJ_TEXT==pObj->GetObjIdentifier(),
            "kein Marquee oder falscher Typ" );

    if( OBJ_TEXT!=pObj->GetObjIdentifier() )
        return;

    // die alte Groesse
    const Rectangle& rOldRect = pObj->GetLogicRect();
    Size aNewSz( nWidth, rOldRect.GetSize().Height() );
    pObj->SetLogicRect( Rectangle( rOldRect.TopLeft(), aNewSz ) );
}

/*  */

const SdrObject *SwHTMLWriter::GetMarqueeTextObj( const SwDrawFrmFmt& rFmt )
{
    const SdrObject* pObj = rFmt.FindSdrObject();
    return (pObj && ::IsMarqueeTextObj( *pObj )) ? pObj : 0;
}

void SwHTMLWriter::GetEEAttrsFromDrwObj( SfxItemSet& rItemSet,
                                         const SdrObject *pObj,
                                         sal_Bool bSetDefaults )
{
    // die Edit script::Engine-Attribute aus dem Objekt holen
    SfxItemSet rObjItemSet = pObj->GetMergedItemSet();

    // ueber die Edit script::Engine-Attribute iterieren und die Attribute
    // in SW-Attrs wandeln bzw. default setzen
    SfxWhichIter aIter( rObjItemSet );
    sal_uInt16 nEEWhich = aIter.FirstWhich();
    while( nEEWhich )
    {
        const SfxPoolItem *pEEItem;
        sal_Bool bSet = SFX_ITEM_SET == rObjItemSet.GetItemState( nEEWhich, sal_False,
                                                              &pEEItem );

        if( bSet || bSetDefaults )
        {
            sal_uInt16 nSwWhich = 0;
            switch( nEEWhich )
            {
            case EE_CHAR_COLOR:         nSwWhich = RES_CHRATR_COLOR;        break;
            case EE_CHAR_STRIKEOUT:     nSwWhich = RES_CHRATR_CROSSEDOUT;   break;
            case EE_CHAR_ESCAPEMENT:    nSwWhich = RES_CHRATR_ESCAPEMENT;   break;
            case EE_CHAR_FONTINFO:      nSwWhich = RES_CHRATR_FONT;         break;
            case EE_CHAR_FONTINFO_CJK:  nSwWhich = RES_CHRATR_CJK_FONT;     break;
            case EE_CHAR_FONTINFO_CTL:  nSwWhich = RES_CHRATR_CTL_FONT;     break;
            case EE_CHAR_FONTHEIGHT:    nSwWhich = RES_CHRATR_FONTSIZE;     break;
            case EE_CHAR_FONTHEIGHT_CJK:nSwWhich = RES_CHRATR_CJK_FONTSIZE; break;
            case EE_CHAR_FONTHEIGHT_CTL:nSwWhich = RES_CHRATR_CTL_FONTSIZE; break;
            case EE_CHAR_KERNING:       nSwWhich = RES_CHRATR_KERNING;      break;
            case EE_CHAR_ITALIC:        nSwWhich = RES_CHRATR_POSTURE;      break;
            case EE_CHAR_ITALIC_CJK:    nSwWhich = RES_CHRATR_CJK_POSTURE;  break;
            case EE_CHAR_ITALIC_CTL:    nSwWhich = RES_CHRATR_CTL_POSTURE;  break;
            case EE_CHAR_UNDERLINE:     nSwWhich = RES_CHRATR_UNDERLINE;    break;
            case EE_CHAR_WEIGHT:        nSwWhich = RES_CHRATR_WEIGHT;       break;
            case EE_CHAR_WEIGHT_CJK:    nSwWhich = RES_CHRATR_CJK_WEIGHT;   break;
            case EE_CHAR_WEIGHT_CTL:    nSwWhich = RES_CHRATR_CTL_WEIGHT;   break;
            }

            if( nSwWhich )
            {
                // wenn das Item nicht gesetzt ist nehmen wir ggf. das
                // Default-Item
                if( !bSet )
                    pEEItem = &rObjItemSet.GetPool()->GetDefaultItem(nEEWhich);

                // jetzt Clonen wir das Item mit der Which-Id des Writers
                SfxPoolItem *pSwItem = pEEItem->Clone();
                pSwItem->SetWhich( nSwWhich );
                rItemSet.Put( *pSwItem );
                delete pSwItem;
            }
        }

        nEEWhich = aIter.NextWhich();
    }
}


Writer& OutHTML_DrawFrmFmtAsMarquee( Writer& rWrt,
                                     const SwDrawFrmFmt& rFmt,
                                     const SdrObject& rSdrObject )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    OSL_ENSURE( rWrt.pDoc->GetDrawModel(), "Da gibt's ein Draw-Obj ohne ein Draw-Model zu haben?" );
    const SdrTextObj *pTextObj = (const SdrTextObj *)&rSdrObject;

    // Gibt es ueberhaupt auszugebenden Text
    const OutlinerParaObject *pOutlinerParaObj =
        pTextObj->GetOutlinerParaObject();
    if( !pOutlinerParaObj )
        return rWrt;

    ByteString sOut( '<' );
    sOut += OOO_STRING_SVTOOLS_HTML_marquee;

    // Die Attribute des Objektd holen
    const SfxItemSet& rItemSet = pTextObj->GetMergedItemSet();

    // BEHAVIOUR
    SdrTextAniKind eAniKind = pTextObj->GetTextAniKind();
    OSL_ENSURE( SDRTEXTANI_SCROLL==eAniKind ||
            SDRTEXTANI_ALTERNATE==eAniKind ||
            SDRTEXTANI_SLIDE==eAniKind,
            "Text-Draw-Objekt nicht fuer Marquee geeignet" );

    const sal_Char *pStr = 0;
    switch( eAniKind )
    {
    case SDRTEXTANI_SCROLL:     pStr = OOO_STRING_SVTOOLS_HTML_BEHAV_scroll;        break;
    case SDRTEXTANI_SLIDE:      pStr = OOO_STRING_SVTOOLS_HTML_BEHAV_slide;     break;
    case SDRTEXTANI_ALTERNATE:  pStr = OOO_STRING_SVTOOLS_HTML_BEHAV_alternate; break;
    default:
        ;
    }

    if( pStr )
        (((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_behavior) += '=') += pStr;

    // DIRECTION
    pStr = 0;
    SdrTextAniDirection eAniDir = pTextObj->GetTextAniDirection();
    switch( eAniDir )
    {
    case SDRTEXTANI_LEFT:       pStr = OOO_STRING_SVTOOLS_HTML_AL_left;     break;
    case SDRTEXTANI_RIGHT:      pStr = OOO_STRING_SVTOOLS_HTML_AL_right;        break;
    default:
        ;
    }

    if( pStr )
        (((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_direction) += '=') += pStr;

    // LOOP
    sal_Int32 nCount =
        ((const SdrTextAniCountItem&)rItemSet.Get( SDRATTR_TEXT_ANICOUNT ))
                                             .GetValue();
    if( 0==nCount )
        nCount = SDRTEXTANI_SLIDE==eAniKind ? 1 : -1;
    (((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_loop) += '=')
        += ByteString::CreateFromInt32( nCount );

    // SCROLLDELAY
    sal_uInt16 nDelay =
        ((const SdrTextAniDelayItem&)rItemSet.Get( SDRATTR_TEXT_ANIDELAY ))
                                            .GetValue();
    (((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_scrolldelay) += '=')
        += ByteString::CreateFromInt32( nDelay );

    // SCROLLAMOUNT
    sal_Int16 nAmount =
        ((const SdrTextAniAmountItem&)rItemSet.Get( SDRATTR_TEXT_ANIAMOUNT ))
                                             .GetValue();
    if( nAmount < 0 )
    {
        nAmount = -nAmount;
    }
    else if( nAmount && Application::GetDefaultDevice() )
    {
        nAmount = (sal_uInt16)(Application::GetDefaultDevice()
                            ->LogicToPixel( Size(nAmount,0),
                                            MapMode(MAP_TWIP) ).Width());
    }
    if( nAmount )
        (((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_scrollamount) += '=')
            += ByteString::CreateFromInt32( nAmount );

    Size aTwipSz( pTextObj->GetLogicRect().GetSize() );
    if( pTextObj->IsAutoGrowWidth() )
        aTwipSz.Width() = 0;
    // Die Hoehe ist bei MS eine Mindesthoehe, also geben wir auch die
    // Mindestheoehe aus, wenn es sie gibt. Da eine Mindesthoehe MINFLY
    // mit hoher Wahrscheinlichkeit vom Import kommt, wird sie nicht mit
    // ausgegeben. Falsch machen kann man da nichst, denn jeder Font ist
    // hoeher.
    if( pTextObj->IsAutoGrowHeight() )
    {
        aTwipSz.Height() = pTextObj->GetMinTextFrameHeight();
        if( MINFLY==aTwipSz.Height() )
            aTwipSz.Height() = 0;
    }

    if( (aTwipSz.Width() || aTwipSz.Height()) &&
        Application::GetDefaultDevice() )
    {
        Size aPixelSz =
            Application::GetDefaultDevice()->LogicToPixel( aTwipSz,
                                                MapMode(MAP_TWIP) );
        if( !aPixelSz.Width() && aTwipSz.Width() )
            aPixelSz.Width() = 1;
        if( !aPixelSz.Height() && aTwipSz.Height() )
            aPixelSz.Height() = 1;

        if( aPixelSz.Width() )
            (((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_width) += '=')
                += ByteString::CreateFromInt32( aPixelSz.Width() );

        if( aPixelSz.Height() )
            (((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_height) += '=')
                += ByteString::CreateFromInt32( aPixelSz.Height() );
    }

    // BGCOLOR
    XFillStyle eFillStyle =
        ((const XFillStyleItem&)rItemSet.Get(XATTR_FILLSTYLE)).GetValue();
    if( XFILL_SOLID==eFillStyle )
    {
        const Color& rFillColor =
            ((const XFillColorItem&)rItemSet.Get(XATTR_FILLCOLOR)).GetColorValue();

        ((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_bgcolor) += '=';
        rWrt.Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_Color( rWrt.Strm(), rFillColor, rHTMLWrt.eDestEnc );
        sOut.Erase();
    }

    if( sOut.Len() )
        rWrt.Strm() << sOut.GetBuffer();

    // und nun noch ALIGN, HSPACE und VSPACE
    ByteString aEndTags;
    sal_uInt32 nFrmFlags = HTML_FRMOPTS_MARQUEE;
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_DRAW ) )
        nFrmFlags |= HTML_FRMOPTS_MARQUEE_CSS1;
    rHTMLWrt.OutFrmFmtOptions( rFmt, aEmptyStr, aEndTags, nFrmFlags );
    if( rHTMLWrt.IsHTMLMode( HTMLMODE_ABS_POS_DRAW ) )
        rHTMLWrt.OutCSS1_FrmFmtOptions( rFmt, nFrmFlags, &rSdrObject );


    rWrt.Strm() << '>';

    // Was jetzt kommt ist das Gegenstueck zu SdrTextObjectt::SetText()
    Outliner aOutliner(0, OUTLINERMODE_TEXTOBJECT);
    aOutliner.SetUpdateMode( sal_False );
    aOutliner.SetText( *pOutlinerParaObj );
    String aText( aOutliner.GetText( aOutliner.GetParagraph(0),
                                     aOutliner.GetParagraphCount() ) );
    HTMLOutFuncs::Out_String( rWrt.Strm(), aText,
                                rHTMLWrt.eDestEnc, &rHTMLWrt.aNonConvertableCharacters );

    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), OOO_STRING_SVTOOLS_HTML_marquee, sal_False );

    if( aEndTags.Len() )
        rWrt.Strm() << aEndTags.GetBuffer();

    return rWrt;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
