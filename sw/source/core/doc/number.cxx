/*************************************************************************
 *
 *  $RCSfile: number.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:16 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#include <string.h>

#ifndef _SV_FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_NUMITEM_HXX //autogen
#include <svx/numitem.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif


USHORT SwNumRule::nRefCount = 0;
SwNumFmt* SwNumRule::aBaseFmts[ RULE_END ][ MAXLEVEL ] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0 };

Font* SwNumRule::pDefBulletFont = 0;
sal_Char* SwNumRule::pDefOutlineName = "Outline";

USHORT SwNumRule::aDefNumIndents[ MAXLEVEL ] = {
//cm:   0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
        283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
};

#if defined UNX

#if defined GCC
extern const sal_Char __FAR_DATA sBulletFntName[];
const sal_Char __FAR_DATA sBulletFntName[] = "starbats";
#else
extern const sal_Char __FAR_DATA sBulletFntName[] = "starbats";
#endif

#else
extern const sal_Char __FAR_DATA sBulletFntName[] = "StarBats";
#endif


// Methoden fuer die Klassen aus NUMRULE.HXX

SwNumFmt::SwNumFmt()
    : SwClient( 0 ),
    pBulletFont( 0 ),
    cBullet( cBulletChar ),
    eNumAdjust( SVX_ADJUST_LEFT ),
    nLSpace( 0 ),
    nAbsLSpace( 0 ),
    nFirstLineOffset( 0 ),
    nCharTextOffset( 0 ),
    nStart( 0 ),
    pGrfBrush( 0 ), pVertOrient( 0 )
{
    nInclUpperLevel = MAXLEVEL;
    bRelLSpace = FALSE;
}


SwNumFmt::SwNumFmt( const SwNumFmt& rNumFmt )
    : SwClient( rNumFmt.pRegisteredIn ), SwNumType( rNumFmt ),
    cBullet( rNumFmt.cBullet ),
    pBulletFont( 0 ),
    eNumAdjust( rNumFmt.eNumAdjust ),
    nLSpace( rNumFmt.nLSpace ),
    nAbsLSpace( rNumFmt.nAbsLSpace ),
    nFirstLineOffset( rNumFmt.nFirstLineOffset ),
    nCharTextOffset( rNumFmt.nCharTextOffset ),
    nStart( rNumFmt.nStart ),
    aPrefix( rNumFmt.aPrefix ),
    aPostfix( rNumFmt.aPostfix ),
    pGrfBrush( 0 ), pVertOrient( 0 )
{
    SetBulletFont(rNumFmt.GetBulletFont());
    nInclUpperLevel = rNumFmt.nInclUpperLevel;
    bRelLSpace = rNumFmt.bRelLSpace;

    SetGrfBrush( rNumFmt.GetGrfBrush(), &rNumFmt.GetGrfSize(),
                rNumFmt.GetGrfOrient() );
}


SwNumFmt::~SwNumFmt()
{
    delete pBulletFont;
    delete pGrfBrush;
    delete pVertOrient;
}


void SwNumFmt::SetBulletFont(const Font* pFont)
{
    if( pBulletFont )
    {
        if( pFont )
            *pBulletFont = *pFont;
        else
            delete pBulletFont, pBulletFont = 0;
    }
    else if( pFont )
        pBulletFont = new Font( *pFont );
}


void SwNumFmt::SetGrfBrush( const SvxBrushItem* pGrfBr, const Size* pSz,
                            const SwFmtVertOrient* pVOrient )
{
    if( pGrfBr )
    {
        if( pGrfBrush )
        {
            if( !( *pGrfBrush == *pGrfBr ) )
            {
                delete pGrfBrush;
                pGrfBrush = (SvxBrushItem*)pGrfBr->Clone();
            }
        }
        else
            pGrfBrush = (SvxBrushItem*)pGrfBr->Clone();

        if( pVertOrient != pVOrient )
        {
            if(pVertOrient)
                delete pVertOrient;

            pVertOrient = pVOrient ? (SwFmtVertOrient*)pVOrient->Clone() : 0;
        }
        if( pSz )
            aGrfSize = *pSz;
        else
            aGrfSize.Width() = aGrfSize.Height() = 0;

        pGrfBrush->SetDoneLink( STATIC_LINK( this, SwNumFmt, GraphicArrived) );
    }
    else
    {
        delete pGrfBrush, pGrfBrush = 0;
        delete pVertOrient, pVertOrient = 0;
        aGrfSize.Width() = aGrfSize.Height() = 0;
    }
}


SwNumFmt& SwNumFmt::operator=( const SwNumFmt& rNumFmt )
{
    cBullet = rNumFmt.cBullet;
    eType = rNumFmt.eType;
    nInclUpperLevel = rNumFmt.nInclUpperLevel;
    bRelLSpace = rNumFmt.bRelLSpace;

    eNumAdjust = rNumFmt.eNumAdjust;
    nLSpace = rNumFmt.nLSpace;
    nAbsLSpace = rNumFmt.nAbsLSpace;
    nFirstLineOffset = rNumFmt.nFirstLineOffset;
    nCharTextOffset = rNumFmt.nCharTextOffset;
    nStart = rNumFmt.nStart;
    aPrefix = rNumFmt.aPrefix;
    aPostfix = rNumFmt.aPostfix;

    SetBulletFont(rNumFmt.GetBulletFont());
    if( rNumFmt.GetRegisteredIn() )
        rNumFmt.pRegisteredIn->Add( this );
    else if( GetRegisteredIn() )
        pRegisteredIn->Remove( this );

    SetGrfBrush( rNumFmt.GetGrfBrush(), &rNumFmt.GetGrfSize(),
                rNumFmt.GetGrfOrient() );

    return *this;
}


BOOL SwNumFmt::operator==( const SwNumFmt& rNumFmt ) const
{
    BOOL bRet = (
        eType == rNumFmt.eType &&
        nInclUpperLevel == rNumFmt.nInclUpperLevel &&
        bRelLSpace == rNumFmt.bRelLSpace &&
        eNumAdjust == rNumFmt.eNumAdjust &&
        nLSpace == rNumFmt.nLSpace &&
        nAbsLSpace == rNumFmt.nAbsLSpace &&
        nFirstLineOffset == rNumFmt.nFirstLineOffset &&
        nCharTextOffset == rNumFmt.nCharTextOffset &&
        nStart == rNumFmt.nStart &&
        aPrefix == rNumFmt.aPrefix &&
        aPostfix == rNumFmt.aPostfix &&
        pRegisteredIn == rNumFmt.pRegisteredIn);

        if( bRet )
        {
            switch( eType )
            {
            case SVX_NUM_CHAR_SPECIAL:

                if( cBullet != rNumFmt.cBullet ||
                    ( pBulletFont
                        ? ( !rNumFmt.GetBulletFont() ||
                            *pBulletFont != *rNumFmt.GetBulletFont() )
                        : 0 != rNumFmt.GetBulletFont() ) )
                    bRet = FALSE;
                break;

            case SVX_NUM_BITMAP:

                if( aGrfSize != rNumFmt.aGrfSize ||
                    ( pGrfBrush
                        ? ( !rNumFmt.pGrfBrush ||
                                *pGrfBrush != *rNumFmt.pGrfBrush )
                        : 0 != rNumFmt.pGrfBrush ) ||
                    ( pVertOrient
                        ? ( !rNumFmt.pVertOrient ||
                                *pVertOrient != *rNumFmt.pVertOrient )
                        : 0 != rNumFmt.pVertOrient ) )
                    bRet = FALSE;
                break;
            }
        }
    return bRet;
}


void SwNumFmt::SetCharFmt( SwCharFmt* pChFmt )
{
    if( pChFmt )
        pChFmt->Add( this );
    else if( GetRegisteredIn() )
        pRegisteredIn->Remove( this );
}


void SwNumFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    // dann suche mal in dem Doc nach dem NumRules-Object, in dem dieses
    // NumFormat gesetzt ist. Das Format muss es nicht geben!
    const SwCharFmt* pFmt = 0;
    switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
    {
    case RES_ATTRSET_CHG:
    case RES_FMT_CHG:
        pFmt = GetCharFmt();
        break;
    }

    if( pFmt && !pFmt->GetDoc()->IsInDtor() )
        UpdateNumNodes( (SwDoc*)pFmt->GetDoc() );
    else
        SwClient::Modify( pOld, pNew );
}


void SwNumFmt::SetGraphic( const String& rName )
{
    if( pGrfBrush && *pGrfBrush->GetGraphicLink() == rName )
        return ;

    delete pGrfBrush;

    pGrfBrush = new SvxBrushItem( rName, aEmptyStr, GPOS_AREA );
    pGrfBrush->SetDoneLink( STATIC_LINK( this, SwNumFmt, GraphicArrived) );
    if( !pVertOrient )
        pVertOrient = new SwFmtVertOrient( 0, VERT_TOP );

    aGrfSize.Width() = aGrfSize.Height() = 0;
}


IMPL_STATIC_LINK( SwNumFmt, GraphicArrived, void *, EMPTYARG )
{
    // ggfs. die GrfSize setzen:
    if( !pThis->aGrfSize.Width() || !pThis->aGrfSize.Height() )
    {
        const Graphic* pGrf = pThis->pGrfBrush->GetGraphic();
        if( pGrf )
            pThis->aGrfSize = ::GetGraphicSizeTwip( *pGrf, 0 );
    }

    if( pThis->GetCharFmt() )
        pThis->UpdateNumNodes( (SwDoc*)pThis->GetCharFmt()->GetDoc() );

    return 0;
}

inline void lcl_SetRuleChgd( SwTxtNode& rNd, BYTE nLevel )
{
    if( rNd.GetNum() &&
        (~NO_NUMLEVEL & rNd.GetNum()->GetLevel() ) == nLevel )
        rNd.NumRuleChgd();
}

void SwNumFmt::UpdateNumNodes( SwDoc* pDoc )
{
    BOOL bDocIsModified = pDoc->IsModified();
    BOOL bFnd = FALSE;
    const SwNumRule* pRule;
    for( USHORT n = pDoc->GetNumRuleTbl().Count(); !bFnd && n; )
    {
        pRule = pDoc->GetNumRuleTbl()[ --n ];
        for( BYTE i = 0; i < MAXLEVEL; ++i )
            if( pRule->GetNumFmt( i ) == this )
            {
                const String& rRuleNm = pRule->GetName();

                SwModify* pMod;
                const SfxPoolItem* pItem;
                USHORT k, nMaxItems = pDoc->GetAttrPool().GetItemCount(
                                                    RES_PARATR_NUMRULE );
                for( k = 0; k < nMaxItems; ++k )
                    if( 0 != (pItem = pDoc->GetAttrPool().GetItem(
                        RES_PARATR_NUMRULE, k ) ) &&
                        0 != ( pMod = (SwModify*)((SwNumRuleItem*)pItem)->
                                GetDefinedIn()) &&
                        ((SwNumRuleItem*)pItem)->GetValue() == rRuleNm )
                    {
                        if( pMod->IsA( TYPE( SwFmt )) )
                        {
                            SwNumRuleInfo aInfo( rRuleNm );
                            pMod->GetInfo( aInfo );

                            for( ULONG nFirst = 0, nLast = aInfo.GetList().Count();
                                nFirst < nLast; ++nFirst )
                                lcl_SetRuleChgd(
                                    *aInfo.GetList().GetObject( nFirst ), i );
                        }
                        else if( ((SwTxtNode*)pMod)->GetNodes().IsDocNodes() )
                            lcl_SetRuleChgd( *(SwTxtNode*)pMod, i );
                    }
                bFnd = TRUE;
                break;
            }
    }

    if( !bFnd )
    {
        pRule = pDoc->GetOutlineNumRule();
        for( BYTE i = 0; i < MAXLEVEL; ++i )
            if( pRule->GetNumFmt( i ) == this )
            {
                ULONG nStt = pDoc->GetNodes().GetEndOfContent().StartOfSectionIndex();
                const SwTxtFmtColls& rColls = *pDoc->GetTxtFmtColls();
                for( USHORT n = 1; n < rColls.Count(); ++n )
                {
                    const SwTxtFmtColl* pColl = rColls[ n ];
                    if( i == pColl->GetOutlineLevel() )
                    {
                        SwClientIter aIter( *(SwTxtFmtColl*)pColl );
                        for( SwTxtNode* pNd = (SwTxtNode*)aIter.First( TYPE( SwTxtNode ));
                                pNd; pNd = (SwTxtNode*)aIter.Next() )
                            if( pNd->GetNodes().IsDocNodes() &&
                                                    nStt < pNd->GetIndex() &&
                                pNd->GetOutlineNum() && i == (~NO_NUMLEVEL &
                                pNd->GetOutlineNum()->GetLevel() ) )
                                    pNd->NumRuleChgd();
                        break;
                    }
                }

                bFnd = TRUE;
                break;
            }
    }

    if( bFnd && !bDocIsModified )
        pDoc->ResetModified();
}

// Graphic ggfs. reinswappen
const Graphic* SwNumFmt::GetGraphic() const
{
    const Graphic* pGrf = 0;
    if( pGrfBrush && GetCharFmt() )
        pGrf = pGrfBrush->GetGraphic( GetCharFmt()->GetDoc()->GetDocShell() );
    return pGrf;
}


/*  */

void SwNumType::GetRomanStr( ULONG nNo, String& rStr ) const
{
    nNo %= 4000;            // mehr kann nicht dargestellt werden
//      i, ii, iii, iv, v, vi, vii, vii, viii, ix
//                          (Dummy),1000,500,100,50,10,5,1
    const sal_Char *cRomanArr = SVX_NUM_ROMAN_UPPER == eType
                        ? "MDCLXVI--"   // +2 Dummy-Eintraege !!
                        : "mdclxvi--";  // +2 Dummy-Eintraege !!

    USHORT nMask = 1000;
    while( nMask )
    {
        BYTE nZahl = BYTE(nNo / nMask);
        BYTE nDiff = 1;
        nNo %= nMask;

        if( 5 < nZahl )
        {
            if( nZahl < 9 )
                rStr += *(cRomanArr-1);
            ++nDiff;
            nZahl -= 5;
        }
        switch( nZahl )
        {
        case 3:     { rStr += *cRomanArr; }
        case 2:     { rStr += *cRomanArr; }
        case 1:     { rStr += *cRomanArr; }
                    break;

        case 4:     {
                        rStr += *cRomanArr;
                        rStr += *(cRomanArr-nDiff);
                    }
                    break;
        case 5:     { rStr += *(cRomanArr-nDiff); }
                    break;
        }

        nMask /= 10;            // zur naechsten Dekade
        cRomanArr += 2;
    }
}


void SwNumType::GetCharStr( ULONG nNo, String& rStr ) const
{
    ASSERT( nNo, "0 ist eine ungueltige Nummer !!" );

    const ULONG coDiff = 'Z' - 'A' +1;
    char cAdd = (SVX_NUM_CHARS_UPPER_LETTER == eType ? 'A' : 'a') - 1;
    ULONG nCalc;

    do {
        nCalc = nNo % coDiff;
        if( !nCalc )
            nCalc = coDiff;
        rStr.Insert( (sal_Unicode)(cAdd + nCalc ), 0 );
        nNo -= nCalc;
        if( nNo )
            nNo /= coDiff;
    } while( nNo );
}

void SwNumType::GetCharStrN( ULONG nNo, String& rStr ) const
{
    ASSERT( nNo, "0 ist eine ungueltige Nummer !!" );

    const ULONG coDiff = 'Z' - 'A' +1;
    sal_Unicode cChar = --nNo % coDiff;
    if( SVX_NUM_CHARS_UPPER_LETTER_N == eType )
        cChar += 'A';
    else
        cChar += 'a';

    rStr.Fill( (nNo / coDiff) + 1, cChar );
}


String SwNumType::GetNumStr( ULONG nNo ) const
{
    String aTmpStr;
    if( nNo )
    {
        switch( eType )
        {
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER:
            GetCharStr( nNo, aTmpStr );
            break;

        case SVX_NUM_CHARS_UPPER_LETTER_N:
        case SVX_NUM_CHARS_LOWER_LETTER_N:
            GetCharStrN( nNo, aTmpStr );
            break;

        case SVX_NUM_ROMAN_UPPER:
        case SVX_NUM_ROMAN_LOWER:
            GetRomanStr( nNo, aTmpStr );
            break;

        case SVX_NUM_CHAR_SPECIAL:      //JP 06.12.99: this types dont have
        case SVX_NUM_BITMAP:            // any number, so return emptystr
        case SVX_NUM_NUMBER_NONE:       //  Bug: 70527
            break;

//      case ARABIC:    ist jetzt default
        default:
            aTmpStr = String::CreateFromInt32( nNo );
            break;
        }
    }
    else
        aTmpStr = '0';
    return aTmpStr;
}

/*  */

BOOL SwNodeNum::operator==( const SwNodeNum& rNum ) const
{
    return nMyLevel == rNum.nMyLevel &&
           nSetValue == rNum.nSetValue &&
           bStartNum == rNum.bStartNum &&
           ( nMyLevel >= MAXLEVEL ||
             0 == memcmp( nLevelVal, rNum.nLevelVal,
                        sizeof( USHORT ) * (nMyLevel+1) ));
}

SwNumRule::SwNumRule( const String& rNm, SwNumRuleType eType, BOOL bAutoFlg )
    : eRuleType( eType ),
    sName( rNm ),
    bAutoRuleFlag( bAutoFlg ),
    bInvalidRuleFlag( TRUE ),
    bContinusNum( FALSE ),
    bAbsSpaces( FALSE ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX )
{
    if( !nRefCount++ )          // zum erstmal, also initialisiern
    {
        SwNumFmt* pFmt;
        // Nummerierung:
        for( int n = 0; n < MAXLEVEL; ++n )
        {
            pFmt = new SwNumFmt;
            pFmt->SetUpperLevel( 1 );
//          pFmt->SetRelLSpace( TRUE );
            pFmt->SetRelLSpace( FALSE );
            pFmt->SetStartValue( 1 );
            pFmt->SetLSpace( lNumIndent );
            pFmt->SetAbsLSpace( SwNumRule::GetNumIndent( n ) );
            pFmt->SetFirstLineOffset( lNumFirstLineOffset );
            pFmt->SetPostfix( aDotStr );
            SwNumRule::aBaseFmts[ NUM_RULE ][ n ] = pFmt;
        }

        // Gliederung:
        for( n = 0; n < MAXLEVEL; ++n )
        {
            pFmt = new SwNumFmt;
//JP 18.01.96: heute soll es mal wieder vollstaendig numeriert werden
//JP 10.03.96: und nun mal wieder nicht
            pFmt->eType = SVX_NUM_NUMBER_NONE;
//            pFmt->eType = ARABIC;
            pFmt->SetStartValue( 1 );
            SwNumRule::aBaseFmts[ OUTLINE_RULE ][ n ] = pFmt;
        }
    }
    memset( aFmts, 0, sizeof( aFmts ));
    ASSERT( sName.Len(), "NumRule ohne Namen!" );
}

SwNumRule::SwNumRule( const SwNumRule& rNumRule )
    : eRuleType( rNumRule.eRuleType ),
    sName( rNumRule.sName ),
    bAutoRuleFlag( rNumRule.bAutoRuleFlag ),
    bInvalidRuleFlag( TRUE ),
    bContinusNum( rNumRule.bContinusNum ),
    bAbsSpaces( rNumRule.bAbsSpaces ),
    nPoolFmtId( rNumRule.GetPoolFmtId() ),
    nPoolHelpId( rNumRule.GetPoolHelpId() ),
    nPoolHlpFileId( rNumRule.GetPoolHlpFileId() )
{
    ++nRefCount;
    memset( aFmts, 0, sizeof( aFmts ));
    for( USHORT n = 0; n < MAXLEVEL; ++n )
        if( rNumRule.aFmts[ n ] )
            Set( n, *rNumRule.aFmts[ n ] );
}

SwNumRule::~SwNumRule()
{
    for( USHORT n = 0; n < MAXLEVEL; ++n )
        delete aFmts[ n ];

    if( !--nRefCount )          // der letzte macht die Tuer zu
    {
            // Nummerierung:
            SwNumFmt** ppFmts = (SwNumFmt**)SwNumRule::aBaseFmts;
            for( int n = 0; n < MAXLEVEL; ++n, ++ppFmts )
                delete *ppFmts, *ppFmts = 0;

            // Gliederung:
            for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
                delete *ppFmts, *ppFmts = 0;
    }
}


void SwNumRule::_MakeDefBulletFont()
{
    pDefBulletFont = new Font( String::CreateFromAscii( sBulletFntName ),
                                aEmptyStr, Size( 0, 14 ) );
    pDefBulletFont->SetCharSet( RTL_TEXTENCODING_SYMBOL );
    pDefBulletFont->SetFamily( FAMILY_DONTKNOW );
    pDefBulletFont->SetPitch( PITCH_DONTKNOW );
    pDefBulletFont->SetWeight( WEIGHT_DONTKNOW );
    pDefBulletFont->SetTransparent( TRUE );
}


void SwNumRule::CheckCharFmts( SwDoc* pDoc )
{
    SwCharFmt* pFmt;
    for( BYTE n = 0; n < MAXLEVEL; ++n )
        if( aFmts[ n ] && 0 != ( pFmt = aFmts[ n ]->GetCharFmt() ) &&
            pFmt->GetDoc() != pDoc )
        {
            // dann kopieren!
            SwNumFmt* pNew = new SwNumFmt( *aFmts[ n ] );
            pNew->SetCharFmt( pDoc->CopyCharFmt( *pFmt ) );
            delete aFmts[ n ];
            aFmts[ n ] = pNew;
        }
}

    // setzt Num oder NoNum fuer den Level am TextNode UND setzt die
    // richtige Attributierung

BOOL SwNumRule::IsRuleLSpace( SwTxtNode& rNd ) const
{
    const SfxPoolItem* pItem;
    BYTE nLvl;
    const SwAttrSet* pASet = rNd.GetpSwAttrSet();
    BOOL bRet = rNd.GetNum() && pASet && SFX_ITEM_SET == pASet->GetItemState(
                RES_LR_SPACE, FALSE, &pItem ) && ( nLvl = (~NO_NUMLEVEL &
                rNd.GetNum()->GetLevel() )) < MAXLEVEL &&
                Get( nLvl ).GetAbsLSpace() == ((SvxLRSpaceItem*)pItem)->GetTxtLeft();

    return bRet;
}

SwNumRule& SwNumRule::operator=( const SwNumRule& rNumRule )
{
    if( this != &rNumRule )
    {
        for( USHORT n = 0; n < MAXLEVEL; ++n )
            Set( n, rNumRule.aFmts[ n ] );

        eRuleType = rNumRule.eRuleType;
        sName = rNumRule.sName;
        bAutoRuleFlag = rNumRule.bAutoRuleFlag;
        bInvalidRuleFlag = TRUE;
        bContinusNum = rNumRule.bContinusNum;
        bAbsSpaces = rNumRule.bAbsSpaces;
        nPoolFmtId = rNumRule.GetPoolFmtId();
        nPoolHelpId = rNumRule.GetPoolHelpId();
        nPoolHlpFileId = rNumRule.GetPoolHlpFileId();
    }
    return *this;
}


BOOL SwNumRule::operator==( const SwNumRule& rRule ) const
{
    BOOL bRet = eRuleType == rRule.eRuleType &&
                sName == rRule.sName &&
                bAutoRuleFlag == rRule.bAutoRuleFlag &&
                bContinusNum == rRule.bContinusNum &&
                bAbsSpaces == rRule.bAbsSpaces &&
                nPoolFmtId == rRule.GetPoolFmtId() &&
                nPoolHelpId == rRule.GetPoolHelpId() &&
                nPoolHlpFileId == rRule.GetPoolHlpFileId();
    if( bRet )
    {
        for( BYTE n = 0; n < MAXLEVEL; ++n )
            if( !( rRule.Get( n ) == Get( n ) ))
            {
                bRet = FALSE;
                break;
            }
    }
    return bRet;
}


void SwNumRule::Set( USHORT i, const SwNumFmt& rNumFmt )
{
    if( !aFmts[ i ] || !(rNumFmt == Get( i )) )
    {
        delete aFmts[ i ];
        aFmts[ i ] = new SwNumFmt( rNumFmt );
        bInvalidRuleFlag = TRUE;
    }
}

void SwNumRule::Set( USHORT i, const SwNumFmt* pNumFmt )
{
    SwNumFmt* pOld = aFmts[ i ];
    if( !pOld )
    {
        if( pNumFmt )
        {
            aFmts[ i ] = new SwNumFmt( *pNumFmt );
            bInvalidRuleFlag = TRUE;
        }
    }
    else if( !pNumFmt )
        delete pOld, aFmts[ i ] = 0, bInvalidRuleFlag = TRUE;
    else if( *pOld != *pNumFmt )
        *pOld = *pNumFmt, bInvalidRuleFlag = TRUE;
}


String SwNumRule::MakeNumString( const SwNodeNum& rNum, BOOL bInclStrings,
                                BOOL bOnlyArabic ) const
{
    String aStr;
    if( NO_NUM > rNum.GetLevel() && !( NO_NUMLEVEL & rNum.GetLevel() ) )
    {
        const SwNumFmt& rMyNFmt = Get( rNum.GetLevel() );
        if( SVX_NUM_NUMBER_NONE != rMyNFmt.eType )
        {
            BYTE i = rNum.GetLevel();

            if( !IsContinusNum() &&
                rMyNFmt.IsInclUpperLevel() )        // nur der eigene Level ?
            {
                BYTE n = rMyNFmt.GetUpperLevel();
                if( 1 < n )
                {
                    if( i+1 >= n )
                        i -= n - 1;
                    else
                        i = 0;
                }
            }

            for( ; i <= rNum.GetLevel(); ++i )
            {
                const SwNumFmt& rNFmt = Get( i );
                if( SVX_NUM_NUMBER_NONE == rNFmt.eType )
                {
    // Soll aus 1.1.1 --> 2. NoNum --> 1..1 oder 1.1 ??
    //                 if( i != rNum.nMyLevel )
    //                    aStr += aDotStr;
                    continue;
                }

                if( rNum.GetLevelVal()[ i ] )
                {
                    if( bOnlyArabic )
                        aStr += String::CreateFromInt32( rNum.GetLevelVal()[ i ] );
                    else
                        aStr += rNFmt.GetNumStr( rNum.GetLevelVal()[ i ] );
                }
                else
                    aStr += '0';        // alle 0-Level sind eine 0
                if( i != rNum.GetLevel() && aStr.Len() )
                    aStr += aDotStr;
            }
        }

        //JP 14.12.99: the type dont have any number, so dont append
        //              the Post-/Prefix String
        if( bInclStrings && !bOnlyArabic &&
            SVX_NUM_CHAR_SPECIAL != rMyNFmt.eType &&
            SVX_NUM_BITMAP != rMyNFmt.eType )
        {
            aStr.Insert( rMyNFmt.GetPrefix(), 0 );
            aStr += rMyNFmt.GetPostfix();
        }
    }
    return aStr;
}

//  ----- Copy-Methode vom SwNumRule ------

    // eine Art Copy-Constructor, damit die Num-Formate auch an den
    // richtigen CharFormaten eines Dokumentes haengen !!
    // (Kopiert die NumFormate und returnt sich selbst)

SwNumRule& SwNumRule::CopyNumRule( SwDoc* pDoc, const SwNumRule& rNumRule )
{
    for( USHORT n = 0; n < MAXLEVEL; ++n )
    {
        Set( n, rNumRule.aFmts[ n ] );
        if( aFmts[ n ] && aFmts[ n ]->GetCharFmt() &&
            USHRT_MAX == pDoc->GetCharFmts()->GetPos( aFmts[n]->GetCharFmt() ))
            // ueber unterschiedliche Dokumente kopieren, dann
            // kopiere das entsprechende Char-Format ins neue
            // Dokument.
            aFmts[n]->SetCharFmt( pDoc->CopyCharFmt( *aFmts[n]->
                                        GetCharFmt() ) );
    }
    eRuleType = rNumRule.eRuleType;
    sName = rNumRule.sName;
    bAutoRuleFlag = rNumRule.bAutoRuleFlag;
    nPoolFmtId = rNumRule.GetPoolFmtId();
    nPoolHelpId = rNumRule.GetPoolHelpId();
    nPoolHlpFileId = rNumRule.GetPoolHlpFileId();
    bInvalidRuleFlag = TRUE;
    return *this;
}
/* -----------------30.10.98 08:40-------------------
 *
 * --------------------------------------------------*/
SwNumFmt::SwNumFmt(const SvxNumberFormat& rNumFmt, SwDoc* pDoc) :
    pBulletFont( 0 ),
    pGrfBrush( 0 ),
    pVertOrient( 0 )
{
    cBullet = rNumFmt.GetBulletChar();
    eType = rNumFmt.GetNumType();
    nInclUpperLevel = rNumFmt.GetIncludeUpperLevels();
    bRelLSpace = FALSE;

    eNumAdjust = rNumFmt.GetNumAdjust();
    nLSpace = rNumFmt.GetLSpace();
    nAbsLSpace = rNumFmt.GetAbsLSpace();
    nFirstLineOffset = rNumFmt.GetFirstLineOffset();
    nCharTextOffset = rNumFmt.GetCharTextDistance();
    nStart = rNumFmt.GetStart();
    aPrefix = rNumFmt.GetPrefix();
    aPostfix = rNumFmt.GetSuffix();

    SetBulletFont(rNumFmt.GetBulletFont());
    if( rNumFmt.GetCharFmt().Len() )
    {
        SwCharFmt* pCFmt = pDoc->FindCharFmtByName( rNumFmt.GetCharFmt() );
        if( !pCFmt )
        {
            USHORT nId = pDoc->GetPoolId( rNumFmt.GetCharFmt(),
                                            GET_POOLID_CHRFMT );
            pCFmt = nId != USHRT_MAX
                        ? pDoc->GetCharFmtFromPool( nId )
                        : pDoc->MakeCharFmt( rNumFmt.GetCharFmt(), 0 );
        }
        pCFmt->Add( this );
    }
    else if( GetRegisteredIn() )
        pRegisteredIn->Remove( this );

    SwVertOrient eOrient = (SwVertOrient)(USHORT) rNumFmt.GetVertOrient();
    SwFmtVertOrient aOrient(0, eOrient);
    Size aTmpSize(rNumFmt.GetGraphicSize());
    SetGrfBrush( rNumFmt.GetBrush(), &aTmpSize,
                &aOrient );

}
/* -----------------30.10.98 12:36-------------------
 *
 * --------------------------------------------------*/
SvxNumberFormat SwNumFmt::MakeSvxFormat() const
{
    SvxNumberFormat aRet(eType);
    aRet.SetNumAdjust(eNumAdjust);
    aRet.SetIncludeUpperLevels(nInclUpperLevel);
    aRet.SetStart(nStart);
    aRet.SetBulletChar(cBullet);
    aRet.SetFirstLineOffset(nFirstLineOffset);
    aRet.SetAbsLSpace(nAbsLSpace);
    aRet.SetLSpace(nLSpace);
    aRet.SetPrefix(aPrefix);
    aRet.SetSuffix(aPostfix);
    if( GetCharFmt() )
        aRet.SetCharFmt( GetCharFmt()->GetName() );
    aRet.SetCharTextDistance(nCharTextOffset);

    SvxFrameVertOrient eOrient = SVX_VERT_NONE;
    if(pVertOrient)
        eOrient = (SvxFrameVertOrient)(USHORT)pVertOrient->GetVertOrient();
    aRet.SetGraphicBrush( pGrfBrush, &aGrfSize, &eOrient);

    aRet.SetBulletFont(pBulletFont);
    return aRet;
}

/* -----------------30.10.98 08:33-------------------
 *
 * --------------------------------------------------*/
void SwNumRule::SetSvxRule(const SvxNumRule& rNumRule, SwDoc* pDoc)
{
    for( USHORT n = 0; n < MAXLEVEL; ++n )
    {
        const SvxNumberFormat* pSvxFmt = rNumRule.Get(n);
        delete aFmts[n];
        aFmts[n] = pSvxFmt ? new SwNumFmt(*pSvxFmt, pDoc) : 0;
    }

//  eRuleType = rNumRule.eRuleType;
//  sName = rNumRule.sName;
//  bAutoRuleFlag = rNumRule.bAutoRuleFlag;
    bInvalidRuleFlag = TRUE;
    bContinusNum = rNumRule.IsContinuousNumbering();
//!!!   bAbsSpaces = rNumRule.IsAbsSpaces();
}
/* -----------------30.10.98 08:33-------------------
 *
 * --------------------------------------------------*/
SvxNumRule SwNumRule::MakeSvxNumRule() const
{
    SvxNumRule aRule(NUM_CONTINUOUS|NUM_CHAR_TEXT_DISTANCE|NUM_CHAR_STYLE|
                        NUM_ENABLE_LINKED_BMP|NUM_ENABLE_EMBEDDED_BMP,
                        MAXLEVEL,
                        eRuleType ==
                            NUM_RULE ?
                                SVX_RULETYPE_NUMBERING :
                                    SVX_RULETYPE_OUTLINE_NUMBERING );
    aRule.SetContinuousNumbering(bContinusNum);
//!!!   aRule.SetAbsSpaces( bAbsSpaces );
    for( USHORT n = 0; n < MAXLEVEL; ++n )
    {
        const SwNumFmt rNumFmt = Get(n);
        SvxNumberFormat aSvxFormat = rNumFmt.MakeSvxFormat();
        aRule.SetLevel(n, aSvxFormat, aFmts[n] != 0);
    }
    return aRule;
}



