/*************************************************************************
 *
 *  $RCSfile: htmldraw.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:55 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#define _EEITEMID_HXX
#include <svx/outliner.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif
#ifndef _SVX_FILLITEM_HXX //autogen
#include <svx/xfillit.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _HTMLOUT_HXX //autogen
#include <svtools/htmlout.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _SVDPOOL_HXX //autogen
#include <svx/svdpool.hxx>
#endif


#ifndef _CHARATR_HXX
#include "charatr.hxx"
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#include "ndtxt.hxx"
#include "doc.hxx"
#include "dcontact.hxx"
#include "poolfmt.hxx"
#include "swcss1.hxx"
#include "swhtml.hxx"
#include "wrthtml.hxx"


const sal_uInt32 HTML_FRMOPTS_MARQUEE   =
    HTML_FRMOPT_ALIGN |
    HTML_FRMOPT_SPACE;

const sal_uInt32 HTML_FRMOPTS_MARQUEE_CSS1  =
    HTML_FRMOPT_S_ALIGN |
    HTML_FRMOPT_S_SPACE;

static HTMLOptionEnum __FAR_DATA aHTMLMarqBehaviorTable[] =
{
    { sHTML_BEHAV_scroll,       SDRTEXTANI_SCROLL       },
    { sHTML_BEHAV_alternate,    SDRTEXTANI_ALTERNATE    },
    { sHTML_BEHAV_slide,        SDRTEXTANI_SLIDE        },
    { 0,                        0                       }
};

static HTMLOptionEnum __FAR_DATA aHTMLMarqDirectionTable[] =
{
    { sHTML_AL_left,            SDRTEXTANI_LEFT         },
    { sHTML_AL_right,           SDRTEXTANI_RIGHT        },
    { 0,                        0                       }
};

/*  */
void SwHTMLParser::InsertDrawObject( SdrObject* pNewDrawObj,
                                     const Size& rPixSpace,
                                     SwVertOrient eVertOri,
                                     SwHoriOrient eHoriOri,
                                     SfxItemSet& rCSS1ItemSet,
                                     SvxCSS1PropertyInfo& rCSS1PropInfo,
                                     sal_Bool bHidden )
{
    // immer ueber dem Text stehen
    pNewDrawObj->SetLayer( pDoc->GetHeavenId() );

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
            nLeftSpace = aLRItem.GetLeft();
            rCSS1PropInfo.bLeftMargin = sal_False;
        }
        if( rCSS1PropInfo.bRightMargin )
        {
            nRightSpace = aLRItem.GetRight();
            rCSS1PropInfo.bRightMargin = sal_False;
        }
        rCSS1ItemSet.ClearItem( RES_LR_SPACE );
    }
    if( nLeftSpace || nRightSpace )
    {
        SvxLRSpaceItem aLRItem;
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
        SvxULSpaceItem aULItem;
        aULItem.SetUpper( nUpperSpace );
        aULItem.SetLower( nLowerSpace );
        aFrmSet.Put( aULItem );
    }

    SwFmtAnchor aAnchor( FLY_IN_CNTNT );
    if( SVX_CSS1_POS_ABSOLUTE == rCSS1PropInfo.ePosition &&
        SVX_CSS1_LTYPE_TWIP == rCSS1PropInfo.eLeftType &&
        SVX_CSS1_LTYPE_TWIP == rCSS1PropInfo.eTopType )
    {
        const SwStartNode *pFlySttNd =
            pDoc->GetNodes()[pPam->GetPoint()->nNode]->FindFlyStartNode();

        if( pFlySttNd )
        {
            aAnchor.SetType( FLY_AT_FLY );
            SwPosition aPos( *pFlySttNd );
            aAnchor.SetAnchor( &aPos );
        }
        else
        {
            aAnchor.SetType( FLY_PAGE );
        }
        pNewDrawObj->SetRelativePos( Point(rCSS1PropInfo.nLeft + nLeftSpace,
                                           rCSS1PropInfo.nTop + nUpperSpace) );
        aFrmSet.Put( SwFmtSurround(SURROUND_THROUGHT) );
    }
    else if( SVX_ADJUST_LEFT == rCSS1PropInfo.eFloat ||
             HORI_LEFT == eHoriOri )
    {
        aAnchor.SetType( FLY_AT_CNTNT );
        aFrmSet.Put( SwFmtSurround(bHidden ? SURROUND_THROUGHT
                                             : SURROUND_RIGHT) );
        pNewDrawObj->SetRelativePos( Point(nLeftSpace, nUpperSpace) );
    }
    else if( VERT_NONE != eVertOri )
    {
        aFrmSet.Put( SwFmtVertOrient( 0, eVertOri ) );
    }

    if( FLY_PAGE == aAnchor.GetAnchorId() )
        aAnchor.SetPageNum( 1 );
    else if( FLY_AT_FLY != aAnchor.GetAnchorId() )
        aAnchor.SetAnchor( pPam->GetPoint() );
    aFrmSet.Put( aAnchor );

    pDoc->Insert( *pPam, *pNewDrawObj, &aFrmSet );
}

/*  */

static void PutEEPoolItem( SfxItemSet &rEEItemSet,
                           const SfxPoolItem& rSwItem )
{

    sal_uInt16 nEEWhich = 0;

    switch( rSwItem.Which() )
    {
    case RES_CHRATR_COLOR:      nEEWhich = EE_CHAR_COLOR; break;
    case RES_CHRATR_CROSSEDOUT: nEEWhich = EE_CHAR_STRIKEOUT; break;
    case RES_CHRATR_ESCAPEMENT: nEEWhich = EE_CHAR_ESCAPEMENT; break;
    case RES_CHRATR_FONT:       nEEWhich = EE_CHAR_FONTINFO; break;
    case RES_CHRATR_FONTSIZE:   nEEWhich = EE_CHAR_FONTHEIGHT; break;
    case RES_CHRATR_KERNING:    nEEWhich = EE_CHAR_KERNING; break;
    case RES_CHRATR_POSTURE:    nEEWhich = EE_CHAR_ITALIC; break;
    case RES_CHRATR_UNDERLINE:  nEEWhich = EE_CHAR_UNDERLINE; break;
    case RES_CHRATR_WEIGHT:     nEEWhich = EE_CHAR_WEIGHT; break;
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

    ASSERT( !pMarquee, "Marquee in Marquee???" );
    aContents.Erase();

    String aId, aStyle, aClass;

    long nWidth=0, nHeight=0;
    sal_Bool bPrcWidth = sal_False, bDirection = sal_False, bBGColor = sal_False;
    Size aSpace( 0, 0 );
    SwVertOrient eVertOri = VERT_TOP;
    SwHoriOrient eHoriOri = HORI_NONE;
    SdrTextAniKind eAniKind = SDRTEXTANI_SCROLL;
    SdrTextAniDirection eAniDir = SDRTEXTANI_LEFT;
    sal_uInt16 nCount = 0, nDelay = 60;
    sal_Int16 nAmount = -6;
    Color aBGColor;

    const HTMLOptions *pOptions = GetOptions();
    sal_uInt16 nArrLen = pOptions->Count();
    for ( sal_uInt16 i=0; i<nArrLen; i++ )
    {
        const HTMLOption *pOption = (*pOptions)[i];
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
                                                      eAniKind );
                break;

            case HTML_O_BGCOLOR:
                pOption->GetColor( aBGColor );
                bBGColor = sal_True;
                break;

            case HTML_O_DIRECTION:
                eAniDir =
                    (SdrTextAniDirection)pOption->GetEnum( aHTMLMarqDirectionTable,
                                                      eAniDir );
                bDirection = sal_True;
                break;

            case HTML_O_LOOP:
                if( pOption->GetString().
                        EqualsIgnoreCaseAscii(sHTML_LOOP_infinite) )
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
                    (SwVertOrient)pOption->GetEnum( aHTMLImgVAlignTable,
                                                    VERT_TOP );
                eHoriOri =
                    (SwHoriOrient)pOption->GetEnum( aHTMLImgHAlignTable,
                                                    HORI_NONE );
                break;
        }
    }

    // Ein DrawTxtobj anlegen
    SdrModel* pModel = pDoc->MakeDrawModel();
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
        RES_CHRATR_BACKGROUND,                     0
    };
    const SwTxtNode *pTxtNd = pDoc->GetNodes()[pPam->GetPoint()->nNode]
                                  ->GetTxtNode();
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

    pMarquee->SetAttributes( aItemSet, sal_False );

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
    ASSERT( pMarquee && OBJ_TEXT==pMarquee->GetObjIdentifier(),
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
    ASSERT( pMarquee && OBJ_TEXT==pMarquee->GetObjIdentifier(),
            "kein Marquee oder falscher Typ" );

    // das akteulle Textstueck an den Text anhaengen
    aContents += aToken;
}

void SwHTMLParser::ResizeDrawObject( SdrObject* pObj, SwTwips nWidth )
{
    ASSERT( OBJ_TEXT==pObj->GetObjIdentifier(),
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
    // die Edit ::com::sun::star::script::Engine-Attribute aus dem Objekt holen
    SfxItemSet aObjItemSet( *pObj->GetItemPool(), EE_CHAR_START,
                                                  EE_CHAR_END );
    pObj->TakeAttributes( aObjItemSet, sal_False, sal_False );

    // ueber die Edit ::com::sun::star::script::Engine-Attribute iterieren und die Attribute
    // in SW-Attrs wandeln bzw. default setzen
    SfxWhichIter aIter( aObjItemSet );
    sal_uInt16 nEEWhich = aIter.FirstWhich();
    while( nEEWhich )
    {
        const SfxPoolItem *pEEItem;
        sal_Bool bSet = SFX_ITEM_SET == aObjItemSet.GetItemState( nEEWhich, sal_False,
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
            case EE_CHAR_FONTHEIGHT:    nSwWhich = RES_CHRATR_FONTSIZE;     break;
            case EE_CHAR_KERNING:       nSwWhich = RES_CHRATR_KERNING;      break;
            case EE_CHAR_ITALIC:        nSwWhich = RES_CHRATR_POSTURE;      break;
            case EE_CHAR_UNDERLINE:     nSwWhich = RES_CHRATR_UNDERLINE;    break;
            case EE_CHAR_WEIGHT:        nSwWhich = RES_CHRATR_WEIGHT;       break;
            }

            if( nSwWhich )
            {
                // wenn das Item nicht gesetzt ist nehmen wir ggf. das
                // Default-Item
                if( !bSet )
                    pEEItem = &aObjItemSet.GetPool()->GetDefaultItem(nEEWhich);

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

    SdrModel* pModel = rWrt.pDoc->GetDrawModel();
    ASSERT( pModel, "Da gibt's ein Draw-Obj ohne ein Draw-Model zu haben?" );
    const SdrTextObj *pTextObj = (const SdrTextObj *)&rSdrObject;

    // Gibt es ueberhaupt auszugebenden Text
    const OutlinerParaObject *pOutlinerParaObj =
        pTextObj->GetOutlinerParaObject();
    if( !pOutlinerParaObj )
        return rWrt;

    ByteString sOut( '<' );
    sOut += sHTML_marquee;

    // Die Attribute des Objektd holen
    sal_uInt16 aWhichMap[5] =   { XATTR_FILL_FIRST,   XATTR_FILL_LAST,
                              SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                              0 };
    SfxItemSet aItemSet( *pTextObj->GetItemPool(), aWhichMap );
    pTextObj->TakeAttributes( aItemSet, sal_False, sal_False );

    // BEHAVIOUR
    SdrTextAniKind eAniKind = pTextObj->GetTextAniKind();
    ASSERT( SDRTEXTANI_SCROLL==eAniKind ||
            SDRTEXTANI_ALTERNATE==eAniKind ||
            SDRTEXTANI_SLIDE==eAniKind,
            "Text-Draw-Objekt nicht fuer Marquee geeignet" )

    const sal_Char *pStr = 0;
    switch( eAniKind )
    {
    case SDRTEXTANI_SCROLL:     pStr = sHTML_BEHAV_scroll;      break;
    case SDRTEXTANI_SLIDE:      pStr = sHTML_BEHAV_slide;       break;
    case SDRTEXTANI_ALTERNATE:  pStr = sHTML_BEHAV_alternate;   break;
    }

    if( pStr )
        (((sOut += ' ') += sHTML_O_behavior) += '=') += pStr;

    // DIRECTION
    pStr = 0;
    SdrTextAniDirection eAniDir = pTextObj->GetTextAniDirection();
    switch( eAniDir )
    {
    case SDRTEXTANI_LEFT:       pStr = sHTML_AL_left;       break;
    case SDRTEXTANI_RIGHT:      pStr = sHTML_AL_right;      break;
    }

    if( pStr )
        (((sOut += ' ') += sHTML_O_direction) += '=') += pStr;

    // LOOP
    sal_Int32 nCount =
        ((const SdrTextAniCountItem&)aItemSet.Get( SDRATTR_TEXT_ANICOUNT ))
                                             .GetValue();
    if( 0==nCount )
        nCount = SDRTEXTANI_SLIDE==eAniKind ? 1 : -1;
    (((sOut += ' ') += sHTML_O_loop) += '=')
        += ByteString::CreateFromInt32( nCount );

    // SCROLLDELAY
    sal_uInt16 nDelay =
        ((const SdrTextAniDelayItem&)aItemSet.Get( SDRATTR_TEXT_ANIDELAY ))
                                            .GetValue();
    (((sOut += ' ') += sHTML_O_scrolldelay) += '=')
        += ByteString::CreateFromInt32( nDelay );

    // SCROLLAMOUNT
    sal_Int16 nAmount =
        ((const SdrTextAniAmountItem&)aItemSet.Get( SDRATTR_TEXT_ANIAMOUNT ))
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
        (((sOut += ' ') += sHTML_O_scrollamount) += '=')
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
            (((sOut += ' ') += sHTML_O_width) += '=')
                += ByteString::CreateFromInt32( aPixelSz.Width() );

        if( aPixelSz.Height() )
            (((sOut += ' ') += sHTML_O_height) += '=')
                += ByteString::CreateFromInt32( aPixelSz.Height() );
    }

    // BGCOLOR
    XFillStyle eFillStyle =
        ((const XFillStyleItem&)aItemSet.Get(XATTR_FILLSTYLE)).GetValue();
    if( XFILL_SOLID==eFillStyle )
    {
        const Color& rFillColor =
            ((const XFillColorItem&)aItemSet.Get(XATTR_FILLCOLOR)).GetValue();

        ((sOut += ' ') += sHTML_O_bgcolor) += '=';
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
#if SUPD>601
    Outliner aOutliner(0, OUTLINERMODE_TEXTOBJECT);
#else
    Outliner aOutliner;
#endif
    aOutliner.SetUpdateMode( sal_False );
    aOutliner.SetText( *pOutlinerParaObj );
#if SUPD>601
    String aText( aOutliner.GetText( aOutliner.GetParagraph(0),
                                     aOutliner.GetParagraphCount() ) );
#else
    String aText( aOutliner.GetText( aOutliner.First(),
                                     aOutliner.GetParagraphCount() ) );
#endif
    HTMLOutFuncs::Out_String( rWrt.Strm(), aText,
                                rHTMLWrt.eDestEnc );

    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_marquee, sal_False );

    if( aEndTags.Len() )
        rWrt.Strm() << aEndTags.GetBuffer();

    return rWrt;
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/html/htmldraw.cxx,v 1.1.1.1 2000-09-18 17:14:55 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.45  2000/09/18 16:04:44  willem.vandorp
      OpenOffice header added.

      Revision 1.44  2000/08/24 07:53:38  os
      Outliner changes

      Revision 1.43  2000/06/26 09:51:57  jp
      must change: GetAppWindow->GetDefaultDevice

      Revision 1.42  2000/04/10 12:20:55  mib
      unicode

      Revision 1.41  2000/03/21 15:06:17  os
      UNOIII

      Revision 1.40  2000/03/03 15:21:00  os
      StarView remainders removed

      Revision 1.39  2000/02/11 14:37:07  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.38  1999/11/19 16:40:20  os
      modules renamed

      Revision 1.37  1999/09/21 09:48:50  mib
      multiple text encodings

      Revision 1.36  1999/09/17 12:12:52  mib
      support of multiple and non system text encodings

      Revision 1.35  1999/06/10 08:33:54  JP
      have to change: no AppWin from SfxApp


      Rev 1.34   10 Jun 1999 10:33:54   JP
   have to change: no AppWin from SfxApp

      Rev 1.33   09 Jun 1999 19:36:36   JP
   have to change: no cast from GetpApp to SfxApp/OffApp, SfxShell only subclass of SfxApp

      Rev 1.32   27 Jan 1999 19:00:04   JP
   Task #61014#: FindSdrObject/FindContactObject als Methoden vom SwFrmFmt

      Rev 1.31   27 Jan 1999 09:43:28   OS
   #56371# TF_ONE51

      Rev 1.30   11 Dec 1998 09:34:56   JP
   Bug #58217#: MarqueeTextObjecte erkennen

      Rev 1.29   20 May 1998 15:22:18   MIB
   Zeichen-Objekte ueber Schnittstelle am Dok einfuegen

      Rev 1.28   02 Apr 1998 19:44:24   MIB
   Frm-Flags: sal_uInt16 -> sal_uInt32

      Rev 1.27   27 Mar 1998 17:24:34   MIB
   direkte absolute Positionierung und Groessen-Export von Controls und Marquee

      Rev 1.26   27 Mar 1998 10:03:38   MIB
   direkte Positionierung von Grafiken etc.

      Rev 1.25   20 Mar 1998 10:49:50   MIB
   abs.-pos. fuer Zeichen-Objektei, Optimierungen

      Rev 1.24   14 Mar 1998 19:02:34   MIB
   Export-Umstellung fuer am-zeichen gebundene Rahmen und abs.-pos.

      Rev 1.23   20 Feb 1998 19:04:32   MA
   header

      Rev 1.22   16 Feb 1998 16:06:18   MIB
   fix #45523#: Hoehe von Laufschriften als Mindesthoehe interpretieren

      Rev 1.21   16 Feb 1998 12:31:08   MIB
   DokInfo-Feld jetzt auch fixed

      Rev 1.20   26 Nov 1997 19:09:24   MA
   includes

      Rev 1.19   03 Nov 1997 14:08:52   MA
   precomp entfernt

      Rev 1.18   19 Sep 1997 08:38:00   MIB
   fix #41185#: Laufschrift an Tabellenzellen-Breite anpassen

      Rev 1.17   10 Sep 1997 15:22:06   MIB
   Styles fuer Marquee

      Rev 1.16   09 Sep 1997 14:13:24   MIB
   Ueberall Browse-View-Breite statt Seitenbreite verwenden

      Rev 1.15   29 Aug 1997 16:49:50   OS
   DLL-Umstellung

      Rev 1.14   18 Aug 1997 11:36:14   OS
   includes

      Rev 1.13   15 Aug 1997 12:47:42   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.12   12 Aug 1997 13:47:02   OS
   Header-Umstellung

      Rev 1.11   07 Aug 1997 15:08:28   OM
   Headerfile-Umstellung

      Rev 1.10   05 Aug 1997 14:56:36   TJ
   include

      Rev 1.9   28 Jul 1997 12:39:00   MIB
   fix #37237#: Bei Laufschrift auch Attrs aus Absatz-Vorlage uebernehmen

      Rev 1.8   29 Apr 1997 11:21:48   MH
   add: header

      Rev 1.7   10 Apr 1997 16:45:50   TRI
   includes

      Rev 1.6   10 Apr 1997 13:15:46   MH
   add: header

      Rev 1.5   09 Jan 1997 11:20:44   MIB
   fix #34862#: -Angaben groesser 100 bei MARQUEE abfangen

      Rev 1.4   22 Nov 1996 09:37:50   MIB
   fix #33414#: MARQUEE in Tabellen an Textbreite anpassen, Farbe aus Standard-Vor

      Rev 1.3   07 Nov 1996 19:06:58   JP
   NoTxtNode hat Altern.Text, Namen am HREF

      Rev 1.2   07 Nov 1996 16:24:30   MIB
   Laufschrift exportieren

      Rev 1.1   06 Nov 1996 11:50:24   MIB
   neue defaults fuer Alternate-Marquee, Anpassung an die 344

      Rev 1.0   05 Nov 1996 12:03:58   MIB
   Initial revision.

*************************************************************************/

