/*************************************************************************
 *
 *  $RCSfile: htmlnum.cxx,v $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _HTMLOUT_HXX //autogen
#include <svtools/htmlout.hxx>
#endif

#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif

#include "htmlnum.hxx"
#include "swcss1.hxx"
#include "swhtml.hxx"
#include "wrthtml.hxx"

// TODO: Unicode: Are these characters the correct ones?
#define HTML_BULLETCHAR_DISC    (0xf000 + 34)
#define HTML_BULLETCHAR_CIRCLE  (0xf000 + 38)
#define HTML_BULLETCHAR_SQUARE  (0xf000 + 36)


// <UL TYPE=...>
static HTMLOptionEnum __FAR_DATA aHTMLULTypeTable[] =
{
    { sHTML_ULTYPE_disc,    HTML_BULLETCHAR_DISC        },
    { sHTML_ULTYPE_circle,  HTML_BULLETCHAR_CIRCLE      },
    { sHTML_ULTYPE_square,  HTML_BULLETCHAR_SQUARE      },
    { 0,                    0                           }
};

/*  */

void SwHTMLNumRuleInfo::Set( const SwTxtNode& rTxtNd )
{
    if( rTxtNd.GetNum() )
    {
        pNumRule = (SwNumRule *)rTxtNd.GetNumRule();
        const SwNodeNum& rNum = *rTxtNd.GetNum();
        nDeep = pNumRule ? GetRealLevel( rNum.GetLevel() )+1 : 0;
        bNumbered = (rNum.GetLevel() & NO_NUMLEVEL) == 0;
        bRestart = rTxtNd.GetNum()->IsStart();
    }
    else
    {
        pNumRule = 0;
        nDeep = 0;
        bNumbered = bRestart = sal_False;
    }

#ifndef NUM_RELSPACE
    bUpdateWholeNum = sal_False;
#endif
}

/*  */

void SwHTMLParser::NewNumBulList( int nToken )
{
    SwHTMLNumRuleInfo& rInfo = GetNumInfo();

    // Erstmal einen neuen Absatz aufmachen
    sal_Bool bSpace = (rInfo.GetDepth() + nDefListDeep) == 0;
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( bSpace ? AM_SPACE : AM_NOSPACE, sal_False );
    else if( bSpace )
        AddParSpace();

    // Die Numerierung-Ebene erhoehen
    rInfo.IncDepth();
    sal_uInt8 nLevel = (sal_uInt8)( (rInfo.GetDepth() <= MAXLEVEL ? rInfo.GetDepth()
                                                        : MAXLEVEL) - 1 );

    // ggf. ein Regelwerk anlegen
    if( !rInfo.GetNumRule() )
    {
        sal_uInt16 nPos = pDoc->MakeNumRule( pDoc->GetUniqueNumRuleName() );
        rInfo.SetNumRule( pDoc->GetNumRuleTbl()[nPos] );
    }

    // das Format anpassen, falls es fuer den Level noch nicht
    // geschehen ist!
    sal_Bool bNewNumFmt = rInfo.GetNumRule()->GetNumFmt( nLevel ) == 0;
    sal_Bool bChangeNumFmt = sal_False;

    // das default Numerierungsformat erstellen
    SwNumFmt aNumFmt( rInfo.GetNumRule()->Get(nLevel) );
    rInfo.SetNodeStartValue( nLevel );
    if( bNewNumFmt )
    {
        sal_uInt16 nChrFmtPoolId = 0;
        if( HTML_ORDERLIST_ON == nToken )
        {
            aNumFmt.eType = SVX_NUM_ARABIC;
            nChrFmtPoolId = RES_POOLCHR_NUM_LEVEL;
        }
        else
        {
            // Wir setzen hier eine Zeichenvorlage, weil die UI das auch
            // so macht. Dadurch wurd immer auch eine 9pt-Schrift
            // eingestellt, was in Netscape nicht der Fall ist. Bisher hat
            // das noch niemanden gestoert.
            aNumFmt.SetBulletFont( &rInfo.GetNumRule()->GetDefBulletFont() );
            aNumFmt.eType = SVX_NUM_CHAR_SPECIAL;
            aNumFmt.SetBulletChar( cBulletChar );       // das Bulletzeichen !!
            nChrFmtPoolId = RES_POOLCHR_BUL_LEVEL;
        }

#ifdef NUM_RELSPACE
        sal_uInt16 nAbsLSpace = HTML_NUMBUL_MARGINLEFT;
        short nFirstLineIndent  = HTML_NUMBUL_INDENT;
        if( nLevel > 0 )
        {
            const SwNumFmt& rPrevNumFmt = rInfo.GetNumRule()->Get( nLevel-1 );
            nAbsLSpace += rPrevNumFmt.GetAbsLSpace();
            nFirstLineIndent = rPrevNumFmt.GetFirstLineOffset();
        }
        aNumFmt.SetAbsLSpace( nAbsLSpace );
        aNumFmt.SetFirstLineOffset( nFirstLineIndent );
#else
        aNumFmt.SetAbsLSpace( (nLevel+1) * HTML_NUMBUL_MARGINLEFT );
        aNumFmt.SetFirstLineOffset( HTML_NUMBUL_INDENT );
#endif
        aNumFmt.SetCharFmt( pCSS1Parser->GetCharFmtFromPool(nChrFmtPoolId) );
        bChangeNumFmt = sal_True;
    }
    else if( 1 != aNumFmt.GetStartValue() )
    {
        // Wenn die Ebene schon mal benutzt wurde, muss der Start-Wert
        // ggf. hart am Absatz gesetzt werden.
        rInfo.SetNodeStartValue( nLevel, 1 );
    }

    // und es ggf. durch die Optionen veraendern
    String aId, aStyle, aClass, aBulletSrc;
    SwVertOrient eVertOri = VERT_NONE;
    sal_uInt16 nWidth=USHRT_MAX, nHeight=USHRT_MAX;
    const HTMLOptions *pOptions = GetOptions();
    for( sal_uInt16 i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
        case HTML_O_ID:
            aId = pOption->GetString();
            break;
        case HTML_O_TYPE:
            if( bNewNumFmt && pOption->GetString().Len() )
            {
                switch( nToken )
                {
                case HTML_ORDERLIST_ON:
                    bChangeNumFmt = sal_True;
                    switch( pOption->GetString().GetChar(0) )
                    {
                    case 'A':   aNumFmt.eType = SVX_NUM_CHARS_UPPER_LETTER; break;
                    case 'a':   aNumFmt.eType = SVX_NUM_CHARS_LOWER_LETTER; break;
                    case 'I':   aNumFmt.eType = SVX_NUM_ROMAN_UPPER;        break;
                    case 'i':   aNumFmt.eType = SVX_NUM_ROMAN_LOWER;        break;
                    default:    bChangeNumFmt = sal_False;
                    }
                    break;

                case HTML_UNORDERLIST_ON:
                    aNumFmt.SetBulletChar( (sal_Unicode)pOption->GetEnum(
                                    aHTMLULTypeTable,aNumFmt.GetBulletChar() ) );
                    bChangeNumFmt = sal_True;
                    break;
                }
            }
            break;
        case HTML_O_START:
            {
                sal_uInt16 nStart = (sal_uInt16)pOption->GetNumber();
                if( bNewNumFmt )
                {
                    aNumFmt.SetStartValue( nStart );
                    bChangeNumFmt = sal_True;
                }
                else
                {
                    rInfo.SetNodeStartValue( nLevel, nStart );
                }
            }
            break;
        case HTML_O_STYLE:
            aStyle = pOption->GetString();
            break;
        case HTML_O_CLASS:
            aClass = pOption->GetString();
            break;
        case HTML_O_SRC:
            if( bNewNumFmt )
            {
                aBulletSrc = pOption->GetString();
                if( !InternalImgToPrivateURL(aBulletSrc) )
                    aBulletSrc = INetURLObject::RelToAbs( aBulletSrc );
            }
            break;
        case HTML_O_WIDTH:
            nWidth = (sal_uInt16)pOption->GetNumber();
            break;
        case HTML_O_HEIGHT:
            nHeight = (sal_uInt16)pOption->GetNumber();
            break;
        case HTML_O_ALIGN:
            eVertOri =
                (SwVertOrient)pOption->GetEnum( aHTMLImgVAlignTable,
                                                eVertOri );
            break;
        }
    }

    if( aBulletSrc.Len() )
    {
        // Eine Bullet-Liste mit Grafiken
        aNumFmt.eType = SVX_NUM_BITMAP;

        // Die Grafik als Brush anlegen
        SvxBrushItem aBrushItem;
        aBrushItem.SetGraphicLink( aBulletSrc );
        aBrushItem.SetGraphicPos( GPOS_AREA );

        // Die Groesse nur beachten, wenn Breite und Hoehe vorhanden sind
        Size aTwipSz( nWidth, nHeight), *pTwipSz=0;
        if( nWidth!=USHRT_MAX && nHeight!=USHRT_MAX )
        {
            aTwipSz =
                Application::GetDefaultDevice()->PixelToLogic( aTwipSz,
                                                    MapMode(MAP_TWIP) );
            pTwipSz = &aTwipSz;
        }

        // Die Ausrichtung auch nur beachten, wenn eine Ausrichtung
        // angegeben wurde
        SwFmtVertOrient aVertOri( 0, eVertOri );
        SwFmtVertOrient *pVertOri = VERT_NONE!=eVertOri ? &aVertOri : 0;

        aNumFmt.SetGrfBrush( &aBrushItem, pTwipSz, pVertOri );

        // Und noch die Grafik merken, um sie in den Absaetzen nicht
        // einzufuegen
        aBulletGrfs[nLevel] = aBulletSrc;
        bChangeNumFmt = sal_True;
    }
    else
        aBulletGrfs[nLevel].Erase();

    // den aktuellen Absatz erst einmal nicht numerieren
    SetNodeNum( nLevel | NO_NUMLEVEL );

    // einen neuen Kontext anlegen
    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( nToken );

#ifndef NUM_RELSPACE
    // darin auch die Raender merken
    sal_uInt16 nLeft=0, nRight=0;
    short nIndent=0;
    GetMarginsFromContext( nLeft, nRight, nIndent );
    nLeft += HTML_NUMBUL_MARGINLEFT;
    pCntxt->SetMargins( nLeft, nRight, nIndent );
#endif

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo ) )
        {
#ifdef NUM_RELSPACE
            if( bNewNumFmt )
            {
                if( aPropInfo.bLeftMargin )
                {
                    // Der Der Default-Einzug wurde schon eingefuegt.
                    sal_uInt16 nAbsLSpace =
                        aNumFmt.GetAbsLSpace() - HTML_NUMBUL_MARGINLEFT;
                    if( aPropInfo.nLeftMargin < 0 &&
                        nAbsLSpace < -aPropInfo.nLeftMargin )
                        nAbsLSpace = 0U;
                    else if( aPropInfo.nLeftMargin > USHRT_MAX ||
                             (long)nAbsLSpace +
                                            aPropInfo.nLeftMargin > USHRT_MAX )
                        nAbsLSpace = USHRT_MAX;
                    else
                        nAbsLSpace = nAbsLSpace + (sal_uInt16)aPropInfo.nLeftMargin;

                    aNumFmt.SetAbsLSpace( nAbsLSpace );
                    bChangeNumFmt = sal_True;
                }
                if( aPropInfo.bTextIndent )
                {
                    short nTextIndent =
                        ((const SvxLRSpaceItem &)aItemSet.Get( RES_LR_SPACE ))
                                                        .GetTxtFirstLineOfst();
                    aNumFmt.SetFirstLineOffset( nTextIndent );
                    bChangeNumFmt = sal_True;
                }
            }
            aPropInfo.bLeftMargin = aPropInfo.bTextIndent = sal_False;
            if( !aPropInfo.bRightMargin )
                aItemSet.ClearItem( RES_LR_SPACE );
#endif
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    if( bChangeNumFmt )
    {
        rInfo.GetNumRule()->Set( nLevel, aNumFmt );
        pDoc->ChgNumRuleFmts( *rInfo.GetNumRule() );
    }

    PushContext( pCntxt );

    // die Attribute der neuen Vorlage setzen
    SetTxtCollAttrs( pCntxt );
}

void SwHTMLParser::EndNumBulList( int nToken )
{
    SwHTMLNumRuleInfo& rInfo = GetNumInfo();

    // Ein neuer Absatz muss aufgemacht werden, wenn
    // - der aktuelle nicht leer ist, also Text oder absatzgebundene Objekte
    //   enthaelt.
    // - der aktuelle Absatz numeriert ist.
    sal_Bool bAppend = pPam->GetPoint()->nContent.GetIndex() > 0;
    if( !bAppend )
    {
        SwTxtNode* pTxtNode = pPam->GetNode()->GetTxtNode();
        bAppend = (pTxtNode && pTxtNode->GetNum() &&
                    !(NO_NUMLEVEL & pTxtNode->GetNum()->GetLevel())) ||
                  HasCurrentParaFlys();
    }

    sal_Bool bSpace = (rInfo.GetDepth() + nDefListDeep) == 1;
    if( bAppend )
        AppendTxtNode( bSpace ? AM_SPACE : AM_NOSPACE, sal_False );
    else if( bSpace )
        AddParSpace();

    // den aktuellen Kontext vom Stack holen
    _HTMLAttrContext *pCntxt = nToken!=0 ? PopContext( nToken & ~1 ) : 0;

    // Keine Liste aufgrund eines Tokens beenden, wenn der Kontext
    // nie angelgt wurde oder nicht beendet werden darf.
    if( rInfo.GetDepth()>0 && (!nToken || pCntxt) )
    {
        rInfo.DecDepth();
        if( !rInfo.GetDepth() )     // wars der letze Level ?
        {
            // Die noch nicht angepassten Formate werden jetzt noch
            // angepasst, damit es sich besser Editieren laesst.
            const SwNumFmt *pRefNumFmt = 0;
            sal_Bool bChanged = sal_False;
            for( sal_uInt16 i=0; i<MAXLEVEL; i++ )
            {
                const SwNumFmt *pNumFmt = rInfo.GetNumRule()->GetNumFmt(i);
                if( pNumFmt )
                {
                    pRefNumFmt = pNumFmt;
                }
                else if( pRefNumFmt )
                {
                    SwNumFmt aNumFmt( rInfo.GetNumRule()->Get(i) );
                    aNumFmt.eType = pRefNumFmt->eType != SVX_NUM_BITMAP
                                        ? pRefNumFmt->eType : SVX_NUM_CHAR_SPECIAL;
                    if( SVX_NUM_CHAR_SPECIAL == aNumFmt.eType )
                    {
                        aNumFmt.SetBulletFont(
                                &rInfo.GetNumRule()->GetDefBulletFont() );
                        aNumFmt.SetBulletChar( cBulletChar );
                    }
                    aNumFmt.SetAbsLSpace( (i+1) * HTML_NUMBUL_MARGINLEFT );
                    aNumFmt.SetFirstLineOffset( HTML_NUMBUL_INDENT );
                    aNumFmt.SetCharFmt( pRefNumFmt->GetCharFmt() );
                    rInfo.GetNumRule()->Set( i, aNumFmt );
                    bChanged = sal_True;
                }
            }
            if( bChanged )
                pDoc->ChgNumRuleFmts( *rInfo.GetNumRule() );

            // Beim letzen Append wurde das NumRule-Item und das
            // NodeNum-Objekt mit kopiert. Beides muessen wir noch
            // loeschen. Das ResetAttr loescht das NodeNum-Objekt mit!
            pPam->GetNode()->GetTxtNode()->ResetAttr( RES_PARATR_NUMRULE );

#ifndef NUM_RELSPACE
            // Die Numerierung komplett beenden.
            if( pTable )
                UpdateNumRuleInTable();
#endif
            rInfo.Clear();
        }
        else
        {
            // den naechsten Absatz erstmal nicht numerieren
            SetNodeNum( rInfo.GetLevel() | NO_NUMLEVEL );
        }
    }

    // und noch Attribute beenden
    sal_Bool bSetAttrs = sal_False;
    if( pCntxt )
    {
        EndContext( pCntxt );
        delete pCntxt;
        bSetAttrs = sal_True;
    }

    if( nToken )
        SetTxtCollAttrs();

    if( bSetAttrs )
        SetAttr();  // Absatz-Atts wegen JavaScript moeglichst schnell setzen

}

/*  */

void SwHTMLParser::NewNumBulListItem( int nToken )
{
    sal_uInt8 nLevel = GetNumInfo().GetLevel();
    String aId, aStyle, aClass;
    sal_uInt16 nStart = HTML_LISTHEADER_ON != nToken
                        ? GetNumInfo().GetNodeStartValue( nLevel )
                        : USHRT_MAX;
    if( USHRT_MAX != nStart )
        GetNumInfo().SetNodeStartValue( nLevel );

    const HTMLOptions *pOptions = GetOptions();
    for( sal_uInt16 i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        switch( pOption->GetToken() )
        {
            case HTML_O_VALUE:
                nStart = (sal_uInt16)pOption->GetNumber();
                break;
            case HTML_O_ID:
                aId = pOption->GetString();
                break;
            case HTML_O_STYLE:
                aStyle = pOption->GetString();
                break;
            case HTML_O_CLASS:
                aClass = pOption->GetString();
                break;
        }
    }

    // einen neuen Absatz aufmachen
    if( pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_NOSPACE, sal_False );
    bNoParSpace = sal_False;    // In <LI> wird kein Abstand eingefuegt!

    if( HTML_LISTHEADER_ON==nToken )
        nLevel = nLevel | NO_NUMLEVEL;

    _HTMLAttrContext *pCntxt = new _HTMLAttrContext( nToken );

    String aNumRuleName;
    if( GetNumInfo().GetNumRule() )
    {
        aNumRuleName = GetNumInfo().GetNumRule()->GetName();
    }
    else
    {
        aNumRuleName = pDoc->GetUniqueNumRuleName();
        SwNumRule aNumRule( aNumRuleName );
        SwNumFmt aNumFmt( aNumRule.Get( 0 ) );
        aNumFmt.SetBulletFont( &SwNumRule::GetDefBulletFont() );
        aNumFmt.eType = SVX_NUM_CHAR_SPECIAL;
        aNumFmt.SetBulletChar( cBulletChar );   // das Bulletzeichen !!
        aNumFmt.SetCharFmt( pCSS1Parser->GetCharFmtFromPool(RES_POOLCHR_BUL_LEVEL) );
        aNumFmt.SetLSpace( (sal_uInt16)(-HTML_NUMBUL_INDENT) );
        aNumFmt.SetFirstLineOffset( HTML_NUMBUL_INDENT );
        aNumRule.Set( 0, aNumFmt );

        pDoc->MakeNumRule( aNumRuleName, &aNumRule );

#ifndef NUM_RELSPACE
        // Hier muessen wir hart attributieren
        sal_uInt16 nLeft=0, nRight=0;
        short nIndent=0;
        GetMarginsFromContext( nLeft, nRight, nIndent );
        nLeft += (sal_uInt16)(-HTML_NUMBUL_INDENT);
        pCntxt->SetMargins( nLeft, nRight, nIndent );
#endif

        ASSERT( !nOpenParaToken,
                "Jetzt geht ein offenes Absatz-Element verloren" );
        // Wir tun so, als ob wir in einem Absatz sind. Dann wird
        // beim naechsten Absatz wenigstens die Numerierung
        // weggeschmissen, die nach dem naechsten AppendTxtNode uebernommen
        // wird.
        nOpenParaToken = nToken;
    }

    SwTxtNode* pTxtNode = pPam->GetNode()->GetTxtNode();
    ((SwCntntNode *)pTxtNode)->SetAttr( SwNumRuleItem(aNumRuleName) );
    pTxtNode->UpdateNum( SwNodeNum( nLevel, nStart ) );
#ifndef NUM_RELSPACE
    pTxtNode->SetNumLSpace( GetNumInfo().GetNumRule()!=0 );
#endif
    if( GetNumInfo().GetNumRule() )
        GetNumInfo().GetNumRule()->SetInvalidRule( sal_True );

    // Styles parsen
    if( HasStyleOptions( aStyle, aId, aClass ) )
    {
        SfxItemSet aItemSet( pDoc->GetAttrPool(), pCSS1Parser->GetWhichMap() );
        SvxCSS1PropertyInfo aPropInfo;

        if( ParseStyleOptions( aStyle, aId, aClass, aItemSet, aPropInfo ) )
        {
            DoPositioning( aItemSet, aPropInfo, pCntxt );
            InsertAttrs( aItemSet, aPropInfo, pCntxt );
        }
    }

    PushContext( pCntxt );

    // die neue Vorlage setzen
    SetTxtCollAttrs( pCntxt );

    // Laufbalkenanzeige aktualisieren
    ShowStatline();
}

void SwHTMLParser::EndNumBulListItem( int nToken, sal_Bool bSetColl,
                                      sal_Bool bLastPara )
{
    // einen neuen Absatz aufmachen
    if( !nToken && pPam->GetPoint()->nContent.GetIndex() )
        AppendTxtNode( AM_NOSPACE );

    // Kontext zu dem Token suchen und vom Stack holen
    _HTMLAttrContext *pCntxt = 0;
    sal_uInt16 nPos = aContexts.Count();
    nToken &= ~1;
    while( !pCntxt && nPos>nContextStMin )
    {
        sal_uInt16 nCntxtToken = aContexts[--nPos]->GetToken();
        switch( nCntxtToken )
        {
        case HTML_LI_ON:
        case HTML_LISTHEADER_ON:
            if( !nToken || nToken == nCntxtToken  )
            {
                pCntxt = aContexts[nPos];
                aContexts.Remove( nPos, 1 );
            }
            break;
        case HTML_ORDERLIST_ON:
        case HTML_UNORDERLIST_ON:
        case HTML_MENULIST_ON:
        case HTML_DIRLIST_ON:
            // keine LI/LH ausserhalb der aktuellen Liste betrachten
            nPos = nContextStMin;
            break;
        }
    }

    // und noch Attribute beenden
    if( pCntxt )
    {
        EndContext( pCntxt );
        SetAttr();  // Absatz-Atts wegen JavaScript moeglichst schnell setzen
        delete pCntxt;
    }

    // und die bisherige Vorlage setzen
    if( bSetColl )
        SetTxtCollAttrs();
}

/*  */

void SwHTMLParser::SetNodeNum( sal_uInt8 nLevel )
{
    SwTxtNode* pTxtNode = pPam->GetNode()->GetTxtNode();
    ASSERT( pTxtNode, "Kein Text-Node an PaM-Position" );

    ASSERT( GetNumInfo().GetNumRule(), "Kein Numerierungs-Regel" );
    const String& rName = GetNumInfo().GetNumRule()->GetName();
    ((SwCntntNode *)pTxtNode)->SetAttr( SwNumRuleItem(rName) );

    pTxtNode->UpdateNum( SwNodeNum( nLevel ) );
#ifndef NUM_RELSPACE
    pTxtNode->SetNumLSpace( sal_True );
#endif

    // NumRule invalidieren, weil sie durch ein EndAction bereits
    // auf valid geschaltet worden sein kann.
    GetNumInfo().GetNumRule()->SetInvalidRule( sal_False );
}


#ifndef NUM_RELSPACE
void SwHTMLParser::UpdateNumRuleInTable()
{
    // Wenn wir in einer Tabelle sind, muss die Numerierung wegen
    // GetMixMaxSize gliech updatet werden. Wenn die Numerierung
    // in einen Rahmen rein oder aus einem Rahmen heraus geschoben
    // muss sie komplett updatet werden. Sonst genuegt es, sie
    // innerhalb der aktuellen Zelle zu updaten.

    ASSERT( GetNumInfo().GetNumRule(), "UpdateNumRuleInTable: Keine NumRule" );
    ASSERT( pTable, "UpdateNumRuleInTable: Keine Tabele, Aufruf unneotig" );

    sal_uInt32 nPos = ULONG_MAX;
    if( !GetNumInfo().IsUpdateWholeNum() )
    {
        const SwStartNode *pSttNd = pPam->GetNode()->FindTableBoxStartNode();
        ASSERT( pSttNd || pPam->GetNode()->FindFlyStartNode(),
                "UpdateNumRuleInTable: Doch keine Tabelle?" );
        if( pSttNd )
            nPos = pSttNd->GetIndex();
    }
    pDoc->UpdateNumRule( GetNumInfo().GetNumRule()->GetName(), nPos );
}
#endif

/*  */

void SwHTMLWriter::FillNextNumInfo()
{
    pNextNumRuleInfo = 0;

    sal_uInt32 nPos = pCurPam->GetPoint()->nNode.GetIndex() + 1;

    sal_Bool bDone = sal_False;
    sal_Bool bTable = sal_False;
    do
    {
        const SwNode* pNd = pDoc->GetNodes()[nPos];
        if( pNd->IsTxtNode() )
        {
            // Der naechste wird als naechstes ausgegeben.
            pNextNumRuleInfo = new SwHTMLNumRuleInfo( *pNd->GetTxtNode() );

            // Vor einer Tabelle behalten wir erst einmal die alte Ebene bei,
            // wenn die gleiche Numerierung hinter der Tabelle
            // fortgesetzt wird und dort nicht von vorne numeriert
            // wird. Die Tabelle wird ann beim Import so weit eingeruckt,
            // wie es der Num-Ebene entspricht.
            if( bTable &&
                pNextNumRuleInfo->GetNumRule()==GetNumInfo().GetNumRule() &&
                !pNextNumRuleInfo->IsRestart() )
            {
                pNextNumRuleInfo->SetDepth( GetNumInfo().GetDepth() );
            }
        }
        else if( pNd->IsTableNode() )
        {
            // Eine Tabelle wird uebersprungen, also den Node
            // hinter der Tabelle betrachten.
            nPos = pNd->EndOfSectionIndex() + 1;
            bTable = sal_True;
        }
        else
        {
            // In allen anderen Faellen ist die Numerierung erstmal
            // zu Ende.
            pNextNumRuleInfo = new SwHTMLNumRuleInfo;
        }
    }
    while( !pNextNumRuleInfo );
}

void SwHTMLWriter::ClearNextNumInfo()
{
    delete pNextNumRuleInfo;
    pNextNumRuleInfo = 0;
}

Writer& OutHTML_NumBulListStart( SwHTMLWriter& rWrt,
                                 const SwHTMLNumRuleInfo& rInfo )
{
    SwHTMLNumRuleInfo& rPrevInfo = rWrt.GetNumInfo();
    sal_Bool bSameRule = rPrevInfo.GetNumRule() == rInfo.GetNumRule();
    if( bSameRule && rPrevInfo.GetDepth() >= rInfo.GetDepth() &&
        !rInfo.IsRestart() )
    {
        return rWrt;
    }

    DBG_ASSERT( rWrt.nLastParaToken == 0,
                "<PRE> wurde nicht vor <OL> beendet." );
    sal_uInt16 nPrevDepth =
        (bSameRule && !rInfo.IsRestart()) ? rPrevInfo.GetDepth() : 0;

    for( sal_uInt16 i=nPrevDepth; i<rInfo.GetDepth(); i++ )
    {
        rWrt.OutNewLine(); // <OL>/<UL> in eine neue Zeile

        rWrt.aBulletGrfs[i].Erase();
        ByteString sOut( '<' );
        const SwNumFmt& rNumFmt = rInfo.GetNumRule()->Get( i );
        SvxExtNumType eType = rNumFmt.eType;
        if( SVX_NUM_CHAR_SPECIAL == eType )
        {
            // Aufzaehlungs-Liste: <OL>
            sOut += sHTML_unorderlist;

            // den Typ ueber das Bullet-Zeichen bestimmen
            const sal_Char *pStr = 0;
            switch( rNumFmt.GetBulletChar() )
            {
            case HTML_BULLETCHAR_DISC:
                pStr = sHTML_ULTYPE_disc;
                break;
            case HTML_BULLETCHAR_CIRCLE:
                pStr = sHTML_ULTYPE_circle;
                break;
            case HTML_BULLETCHAR_SQUARE:
                pStr = sHTML_ULTYPE_square;
                break;
            }

            if( pStr )
                (((sOut += ' ') += sHTML_O_type) += '=') += pStr;
        }
        else if( SVX_NUM_BITMAP == eType )
        {
            // Aufzaehlungs-Liste: <OL>
            sOut += sHTML_unorderlist;
            rWrt.Strm() << sOut.GetBuffer();
            sOut.Erase();

            OutHTML_BulletImage( rWrt,
                                    0,
                                    rNumFmt.GetGrfBrush(),
                                    rWrt.aBulletGrfs[i],
                                    rNumFmt.GetGrfSize(),
                                    rNumFmt.GetGrfOrient() );
        }
        else
        {
            // Numerierungs-Liste: <UL>
            sOut += sHTML_orderlist;

            // den Typ ueber das Format bestimmen
            sal_Char cType = 0;
            switch( eType )
            {
            case SVX_NUM_CHARS_UPPER_LETTER:    cType = 'A'; break;
            case SVX_NUM_CHARS_LOWER_LETTER:    cType = 'a'; break;
            case SVX_NUM_ROMAN_UPPER:           cType = 'I'; break;
            case SVX_NUM_ROMAN_LOWER:           cType = 'i'; break;
            }
            if( cType )
                (((sOut += ' ') += sHTML_O_type) += '=') += cType;

            // und evtl. den Startwert mit ausgeben
            if( rNumFmt.GetStartValue() != 1 )
            {
                (((sOut += ' ') += sHTML_O_start) += '=')
                    += ByteString::CreateFromInt32( rNumFmt.GetStartValue() );
            }
        }

        if( sOut.Len() )
            rWrt.Strm() << sOut.GetBuffer();

#ifdef NUM_RELSPACE
        if( rWrt.bCfgOutStyles )
            OutCSS1_NumBulListStyleOpt( rWrt, *rInfo.GetNumRule(), i );
#endif

        rWrt.Strm() << '>';

        rWrt.IncIndentLevel(); // Inhalt von <OL> einruecken
    }

    return rWrt;
}

Writer& OutHTML_NumBulListEnd( SwHTMLWriter& rWrt,
                               const SwHTMLNumRuleInfo& rNextInfo )
{
    SwHTMLNumRuleInfo& rInfo = rWrt.GetNumInfo();
    sal_Bool bSameRule = rNextInfo.GetNumRule() == rInfo.GetNumRule();
    if( bSameRule && rNextInfo.GetDepth() >= rInfo.GetDepth() &&
        !rNextInfo.IsRestart() )
    {
        return rWrt;
    }

    DBG_ASSERT( rWrt.nLastParaToken == 0,
                "<PRE> wurde nicht vor </OL> beendet." );
    sal_uInt16 nNextDepth =
        (bSameRule && !rNextInfo.IsRestart()) ? rNextInfo.GetDepth() : 0;

    // MIB 23.7.97: Die Schleife muss doch rueckwaerts durchlaufen
    // werden, weil die Reihenfolge von </OL>/</UL> stimmen muss
    for( sal_uInt16 i=rInfo.GetDepth(); i>nNextDepth; i-- )
    {
        rWrt.DecIndentLevel(); // Inhalt von <OL> einruecken
        if( rWrt.bLFPossible )
            rWrt.OutNewLine(); // </OL>/</UL> in eine neue Zeile

        // es wird also eine Liste angefangen oder beendet:
        SvxExtNumType eType = rInfo.GetNumRule()->Get( i-1 ).eType;
        const sal_Char *pStr;
        if( SVX_NUM_CHAR_SPECIAL == eType || SVX_NUM_BITMAP == eType)
            pStr = sHTML_unorderlist;
        else
            pStr = sHTML_orderlist;
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), pStr, sal_False );
        rWrt.bLFPossible = sal_True;
    }

    return rWrt;
}


/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.21  2000/09/18 16:04:45  willem.vandorp
      OpenOffice header added.

      Revision 1.20  2000/06/26 09:52:17  jp
      must change: GetAppWindow->GetDefaultDevice

      Revision 1.19  2000/04/10 12:20:56  mib
      unicode

      Revision 1.18  2000/03/21 15:06:18  os
      UNOIII

      Revision 1.17  2000/02/11 14:37:16  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.16  1999/11/19 16:40:20  os
      modules renamed

      Revision 1.15  1999/09/17 12:13:32  mib
      support of multiple and non system text encodings

      Revision 1.14  1999/03/26 10:37:28  MIB
      #63049#: Sofortige Numerierungs-Aktualisierung in Tabellen ist jetzt unnoetig


      Rev 1.13   26 Mar 1999 11:37:28   MIB
   #63049#: Sofortige Numerierungs-Aktualisierung in Tabellen ist jetzt unnoetig

      Rev 1.12   23 Mar 1999 15:28:16   MIB
   #63049#: Relative Einzuege in Numerierungen

      Rev 1.11   17 Mar 1999 16:47:10   MIB
   #63049#: Numerierungen mit relativen Abstaenden

      Rev 1.10   27 Jan 1999 09:43:54   OS
   #56371# TF_ONE51

      Rev 1.9   06 Jan 1999 10:33:40   MIB
   #60311#: In Listen fuer nicht-numerierte Absaetze keine <PRE> ausgeben

      Rev 1.8   17 Nov 1998 10:44:30   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.7   05 Nov 1998 12:34:30   MIB
   #59042#: Fuer nicht verwendete Numerierungs-Ebenen sinnvolle Defaults setzen

      Rev 1.6   21 Apr 1998 13:46:58   MIB
   fix: Keine ::com::sun::star::text::Bookmark fuer abs-pos Objekte mit ID einfuegen

      Rev 1.5   03 Apr 1998 12:21:48   MIB
   Export des Rahmen-Namens als ID

      Rev 1.4   25 Mar 1998 12:09:24   MIB
   unneotige defines weg

      Rev 1.3   02 Mar 1998 18:32:16   MIB
   fix #47671#: Erste Numerierung auf einer Ebene gewinnt, Startwert

      Rev 1.2   20 Feb 1998 19:01:50   MA
   header

      Rev 1.1   10 Feb 1998 09:51:24   MIB
   fix: Fuer Absatz-Abstand am Start-/Ende von Listen auch OL/UL/DL beachten

      Rev 1.0   19 Jan 1998 16:16:26   MIB
   Initial revision.


*************************************************************************/

