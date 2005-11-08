/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: number.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-08 17:17:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


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
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

#include <hash_map>

USHORT SwNumRule::nRefCount = 0;
SwNumFmt* SwNumRule::aBaseFmts[ RULE_END ][ MAXLEVEL ] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0 };

Font* SwNumRule::pDefBulletFont = 0;
sal_Char* SwNumRule::pDefOutlineName = "Outline";

// #i30312#
USHORT SwNumRule::aDefNumIndents[ MAXLEVEL ] = {
//inch:   0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
        1440/4, 1440/2, 1440*3/4, 1440, 1440*5/4, 1440*3/2, 1440*7/4, 1440*2,
        1440*9/4, 1440*5/2
};

#if defined( UNX ) && defined( GCC )
extern const sal_Char __FAR_DATA sBulletFntName[];
const sal_Char __FAR_DATA sBulletFntName[] = "StarSymbol";
#else
extern const sal_Char __FAR_DATA sBulletFntName[] = "StarSymbol";
#endif


BYTE GetRealLevel( const BYTE nLvl )
{
    return nLvl & (NO_NUMLEVEL - 1);
}

BOOL IsNum( BYTE nLvl )
{
    return 0 == (nLvl & NO_NUMLEVEL);
}

BOOL IsShowNum( BYTE nLvl )
{
    return IsNum(nLvl) && nLvl != NO_NUMBERING;
}

void SetNoNum( BYTE * nLvl, BOOL nVal )
{
    if (nVal)
        *nLvl |= NO_NUMLEVEL;
    else
        *nLvl &= ~NO_NUMLEVEL;
}

void SetLevel(BYTE * nLvl, BYTE nNewLevel)
{
    if (IsNum(*nLvl))
        *nLvl = nNewLevel;
    else
        *nLvl = nNewLevel | NO_NUMLEVEL;
}

const SwNumFmt& SwNumRule::Get( USHORT i ) const
{
    ASSERT_ID( i < MAXLEVEL && eRuleType < RULE_END, ERR_NUMLEVEL);
    return aFmts[ i ] ? *aFmts[ i ]
                      : *aBaseFmts[ eRuleType ][ i ];
}

const SwNumFmt* SwNumRule::GetNumFmt( USHORT i ) const
{
    ASSERT_ID( i < MAXLEVEL && eRuleType < RULE_END, ERR_NUMLEVEL);
    return aFmts[ i ];
}

void SwNumRule::SetName(const String & rName)
{

    if (pNumRuleMap)
    {
        pNumRuleMap->erase(sName);
        (*pNumRuleMap)[rName] = this;
    }

    sName = rName;
}

void SwNumRule::SetList(SwTxtNodeTable * _pList)
{
    if (pList)
        delete pList;

    pList = _pList;
}

void SwNumRule::SetNumRuleMap(std::hash_map<String, SwNumRule *, StringHash> *
                              _pNumRuleMap)
{
    pNumRuleMap = _pNumRuleMap;
}

const Font& SwNumRule::GetDefBulletFont()
{
    if( !pDefBulletFont )
        SwNumRule::_MakeDefBulletFont();
    return *pDefBulletFont;
}

USHORT SwNumRule::GetNumIndent( BYTE nLvl )
{
    ASSERT( MAXLEVEL > nLvl, "NumLevel is out of range" );
    return aDefNumIndents[ nLvl ];
}

USHORT SwNumRule::GetBullIndent( BYTE nLvl )
{
    ASSERT( MAXLEVEL > nLvl, "NumLevel is out of range" );
    return aDefNumIndents[ nLvl ];
}



static void lcl_SetRuleChgd( SwTxtNode& rNd, BYTE nLevel )
{
    if( rNd.GetLevel() == nLevel )
        rNd.NumRuleChgd();
}
/* -----------------------------22.02.01 13:41--------------------------------

 ---------------------------------------------------------------------------*/
SwNumFmt::SwNumFmt() :
    SwClient( 0 ),
    SvxNumberFormat(SVX_NUM_ARABIC),
    pVertOrient(new SwFmtVertOrient( 0, VERT_NONE))
{
}
/* -----------------------------22.02.01 13:42--------------------------------

 ---------------------------------------------------------------------------*/
SwNumFmt::SwNumFmt( const SwNumFmt& rFmt) :
    SwClient( rFmt.pRegisteredIn ),
    SvxNumberFormat(rFmt),
    pVertOrient(new SwFmtVertOrient( 0, (SwVertOrient)rFmt.GetVertOrient()))
{
    SvxFrameVertOrient eVertOrient = rFmt.GetVertOrient();
    SetGraphicBrush( rFmt.GetBrush(), &rFmt.GetGraphicSize(),
                                                &eVertOrient);
}
/* -----------------------------22.02.01 13:58--------------------------------

 ---------------------------------------------------------------------------*/
SwNumFmt::SwNumFmt(const SvxNumberFormat& rNumFmt, SwDoc* pDoc) :
    SvxNumberFormat(rNumFmt),
    pVertOrient(new SwFmtVertOrient( 0, (SwVertOrient)rNumFmt.GetVertOrient()))
{
    SvxFrameVertOrient eVertOrient = rNumFmt.GetVertOrient();
    SetGraphicBrush( rNumFmt.GetBrush(), &rNumFmt.GetGraphicSize(),
                                                &eVertOrient);
    const String& rCharStyleName = rNumFmt.SvxNumberFormat::GetCharFmtName();
    if( rCharStyleName.Len() )
    {
        SwCharFmt* pCFmt = pDoc->FindCharFmtByName( rCharStyleName );
        if( !pCFmt )
        {
            USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName( rCharStyleName,
                                            GET_POOLID_CHRFMT );
            pCFmt = nId != USHRT_MAX
                        ? pDoc->GetCharFmtFromPool( nId )
                        : pDoc->MakeCharFmt( rCharStyleName, 0 );
        }
        pCFmt->Add( this );
    }
    else if( GetRegisteredIn() )
        pRegisteredIn->Remove( this );

}
/* -----------------------------22.02.01 13:42--------------------------------

 ---------------------------------------------------------------------------*/
SwNumFmt::~SwNumFmt()
{
    delete pVertOrient;
}
/* -----------------------------02.07.01 15:37--------------------------------

 ---------------------------------------------------------------------------*/
void SwNumFmt::NotifyGraphicArrived()
{
    if( GetCharFmt() )
        UpdateNumNodes( (SwDoc*)GetCharFmt()->GetDoc() );
}

// #i22362#
BOOL SwNumFmt::IsEnumeration() const
{
    // --> FME 2004-08-12 #i30655# native numbering did not work any longer
    // using this code. Therefore HBRINKM and I agreed upon defining
    // IsEnumeration() as !IsItemize()
    return !IsItemize();
    // <--

    /*
    BOOL bResult;

    switch(GetNumberingType())
    {
    case SVX_NUM_CHARS_UPPER_LETTER:
    case SVX_NUM_CHARS_LOWER_LETTER:
    case SVX_NUM_ROMAN_UPPER:
    case SVX_NUM_ROMAN_LOWER:
    case SVX_NUM_ARABIC:
    case SVX_NUM_PAGEDESC:
    case SVX_NUM_CHARS_UPPER_LETTER_N:
    case SVX_NUM_CHARS_LOWER_LETTER_N:
        bResult = TRUE;

        break;

    default:
        bResult = FALSE;
    }

    return bResult;
     */
}

// #i29560#
BOOL SwNumFmt::IsItemize() const
{
    BOOL bResult;

    switch(GetNumberingType())
    {
    case SVX_NUM_CHAR_SPECIAL:
    case SVX_NUM_BITMAP:
        bResult = TRUE;

        break;

    default:
        bResult = FALSE;
    }

    return bResult;

}


/* -----------------------------23.02.01 09:28--------------------------------

 ---------------------------------------------------------------------------*/
SwNumFmt& SwNumFmt::operator=( const SwNumFmt& rNumFmt)
{
    SvxNumberFormat::operator=(rNumFmt);
    if( rNumFmt.GetRegisteredIn() )
        rNumFmt.pRegisteredIn->Add( this );
    else if( GetRegisteredIn() )
        pRegisteredIn->Remove( this );
    return *this;
}
/* -----------------------------23.02.01 09:28--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwNumFmt::operator==( const SwNumFmt& rNumFmt) const
{
    BOOL bRet = SvxNumberFormat::operator==(rNumFmt) &&
        pRegisteredIn == rNumFmt.pRegisteredIn;
    return bRet;
}

/* -----------------------------22.02.01 13:42--------------------------------

 ---------------------------------------------------------------------------*/
const Graphic* SwNumFmt::GetGraphic() const
{
    const Graphic* pGrf = 0;
    if( GetBrush() && GetCharFmt() )
        pGrf = GetBrush()->GetGraphic( GetCharFmt()->GetDoc()->GetDocShell() );
    return pGrf;
}
/* -----------------------------22.02.01 13:44--------------------------------

 ---------------------------------------------------------------------------*/
void SwNumFmt::SetCharFmt( SwCharFmt* pChFmt)
{
    if( pChFmt )
        pChFmt->Add( this );
    else if( GetRegisteredIn() )
        pRegisteredIn->Remove( this );
}
/* -----------------------------22.02.01 13:45--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------23.02.01 11:08--------------------------------

 ---------------------------------------------------------------------------*/
void SwNumFmt::SetCharFmtName(const String& rSet)
{
    SvxNumberFormat::SetCharFmtName(rSet);
}
/* -----------------------------22.02.01 13:47--------------------------------

 ---------------------------------------------------------------------------*/
const String&   SwNumFmt::GetCharFmtName() const
{
    if((SwCharFmt*)pRegisteredIn)
        return ((SwCharFmt*)pRegisteredIn)->GetName();
    else
        return aEmptyStr;
}
/* -----------------------------22.02.01 16:05--------------------------------

 ---------------------------------------------------------------------------*/
void    SwNumFmt::SetGraphicBrush( const SvxBrushItem* pBrushItem, const Size* pSize,
    const SvxFrameVertOrient* pOrient)
{
    if(pOrient)
        pVertOrient->SetVertOrient( (SwVertOrient)*pOrient );
    SvxNumberFormat::SetGraphicBrush( pBrushItem, pSize, pOrient);
}
/* -----------------------------22.02.01 16:05--------------------------------

 ---------------------------------------------------------------------------*/
void    SwNumFmt::SetVertOrient(SvxFrameVertOrient eSet)
{
    SvxNumberFormat::SetVertOrient(eSet);
}
/* -----------------------------22.02.01 16:05--------------------------------

 ---------------------------------------------------------------------------*/
SvxFrameVertOrient  SwNumFmt::GetVertOrient() const
{
    return SvxNumberFormat::GetVertOrient();
}
/* -----------------------------22.02.01 13:54--------------------------------

 ---------------------------------------------------------------------------*/
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

    if( bFnd && !bDocIsModified )
        pDoc->ResetModified();
}
/* -----------------------------31.05.01 16:08--------------------------------

 ---------------------------------------------------------------------------*/
const SwFmtVertOrient*      SwNumFmt::GetGraphicOrientation() const
{
    SvxFrameVertOrient  eOrient = SvxNumberFormat::GetVertOrient();
    if(SVX_VERT_NONE == eOrient)
        return 0;
    else
    {
        pVertOrient->SetVertOrient((SwVertOrient)eOrient);
        return pVertOrient;
    }
}

#ifndef PRODUCT
long int SwNumRule::nInstances = 0;
#endif

SwNumRule::SwNumRule( const String& rNm, SwNumRuleType eType, BOOL bAutoFlg )
    : eRuleType( eType ),
    sName( rNm ),
      pList(0),
      aMarkedLevels( MAXLEVEL ), // #i27615#
      pNumRuleMap(0),
    bAutoRuleFlag( bAutoFlg ),
    bInvalidRuleFlag( TRUE ),
    bContinusNum( FALSE ),
    bAbsSpaces( FALSE ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX ),
    // --> OD 2005-10-21 - initialize member <mbCountPhantoms>
    mbCountPhantoms( true )
    // <--
{
#ifndef PRODUCT
    nSerial = nInstances++;
#endif

    if( !nRefCount++ )          // zum erstmal, also initialisiern
    {
        SwNumFmt* pFmt;
        int n;

        // Nummerierung:
        for( n = 0; n < MAXLEVEL; ++n )
        {
            pFmt = new SwNumFmt;
            pFmt->SetIncludeUpperLevels( 1 );
            pFmt->SetStart( 1 );
            pFmt->SetLSpace( lNumIndent );
            pFmt->SetAbsLSpace( lNumIndent + SwNumRule::GetNumIndent( n ) );
            pFmt->SetFirstLineOffset( lNumFirstLineOffset );
            pFmt->SetSuffix( aDotStr );
            pFmt->SetBulletChar( GetBulletChar(n));
            SwNumRule::aBaseFmts[ NUM_RULE ][ n ] = pFmt;
        }

        // Gliederung:
        for( n = 0; n < MAXLEVEL; ++n )
        {
            pFmt = new SwNumFmt;
            pFmt->SetNumberingType(SVX_NUM_NUMBER_NONE);
            pFmt->SetIncludeUpperLevels( MAXLEVEL );
            pFmt->SetStart( 1 );
            pFmt->SetCharTextDistance( lOutlineMinTextDistance );
            pFmt->SetBulletChar( GetBulletChar(n));
            SwNumRule::aBaseFmts[ OUTLINE_RULE ][ n ] = pFmt;
        }
    }
    memset( aFmts, 0, sizeof( aFmts ));
    ASSERT( sName.Len(), "NumRule ohne Namen!" );
}

SwNumRule::SwNumRule( const SwNumRule& rNumRule )
    : eRuleType( rNumRule.eRuleType ),
    sName( rNumRule.sName ),
      pList(0),
    aMarkedLevels( MAXLEVEL ), // #i27615#
      pNumRuleMap(0),
    bAutoRuleFlag( rNumRule.bAutoRuleFlag ),
    bInvalidRuleFlag( TRUE ),
    bContinusNum( rNumRule.bContinusNum ),
    bAbsSpaces( rNumRule.bAbsSpaces ),
    nPoolFmtId( rNumRule.GetPoolFmtId() ),
    nPoolHelpId( rNumRule.GetPoolHelpId() ),
    nPoolHlpFileId( rNumRule.GetPoolHlpFileId() ),
    // --> OD 2005-10-21 - initialize member <mbCountPhantoms>
    mbCountPhantoms( true )
    // <--
{
#ifndef PRODUCT
    nSerial = nInstances++;
#endif

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

    if (pNumRuleMap)
    {
        pNumRuleMap->erase(GetName());
    }

    if( !--nRefCount )          // der letzte macht die Tuer zu
    {
            // Nummerierung:
            SwNumFmt** ppFmts = (SwNumFmt**)SwNumRule::aBaseFmts;
            int n;

            for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
                delete *ppFmts, *ppFmts = 0;

            // Gliederung:
            for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
                delete *ppFmts, *ppFmts = 0;
    }

    tPamAndNums::iterator aIt;

    for(aIt = aNumberRanges.begin(); aIt != aNumberRanges.end(); aIt++)
    {
        delete (*aIt).first;
        delete (*aIt).second;
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

    if (rNum.IsCounted())
        aStr = MakeNumString(rNum.GetNumberVector(),
                             bInclStrings, bOnlyArabic);

    return aStr;
}

String SwNumRule::MakeNumString( const SwNodeNum::tNumberVector & rNumVector,
                                 const BOOL bInclStrings,
                                 const BOOL bOnlyArabic,
                                 const unsigned int _nRestrictToThisLevel ) const
{
    String aStr;

    unsigned int nLevel = rNumVector.size() - 1;
    // --> OD 2005-10-17 #126238#
    if ( nLevel > _nRestrictToThisLevel )
    {
        nLevel = _nRestrictToThisLevel;
    }
    // <--

    if (nLevel >= 0 && nLevel < MAXLEVEL)
    {
        const SwNumFmt& rMyNFmt = Get( nLevel );
        if( SVX_NUM_NUMBER_NONE != rMyNFmt.GetNumberingType() )
        {
            BYTE i = nLevel;

            if( !IsContinusNum() &&
                rMyNFmt.GetIncludeUpperLevels() )  // nur der eigene Level ?
            {
                BYTE n = rMyNFmt.GetIncludeUpperLevels();
                if( 1 < n )
                {
                    if( i+1 >= n )
                        i -= n - 1;
                    else
                        i = 0;
                }
            }

            for( ; i <= nLevel; ++i )
            {
                const SwNumFmt& rNFmt = Get( i );
                if( SVX_NUM_NUMBER_NONE == rNFmt.GetNumberingType() )
                {
                    // Soll aus 1.1.1 --> 2. NoNum --> 1..1 oder 1.1 ??
                    //                 if( i != rNum.nMyLevel )
                    //                    aStr += aDotStr;
                    continue;
                }

                if( rNumVector[ i ] )
                {
                    if( bOnlyArabic )
                        aStr += String::CreateFromInt32( rNumVector[ i ] );
                    else
                        aStr += rNFmt.GetNumStr( rNumVector[ i ] );
                }
                else
                    aStr += '0';        // alle 0-Level sind eine 0
                if( i != nLevel && aStr.Len() )
                    aStr += aDotStr;
            }

            //JP 14.12.99: the type dont have any number, so dont append
            //              the Post-/Prefix String
            if( bInclStrings && !bOnlyArabic &&
                SVX_NUM_CHAR_SPECIAL != rMyNFmt.GetNumberingType() &&
                SVX_NUM_BITMAP != rMyNFmt.GetNumberingType() )
            {
                aStr.Insert( rMyNFmt.GetPrefix(), 0 );
                aStr += rMyNFmt.GetSuffix();
            }
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
        SwNumFmt aNumFmt = Get(n);
        if(aNumFmt.GetCharFmt())
            aNumFmt.SetCharFmtName(aNumFmt.GetCharFmt()->GetName());
        aRule.SetLevel(n, aNumFmt, aFmts[n] != 0);
    }
    return aRule;
}

void SwNumRule::SetInvalidRule(BOOL bFlag)
{
    if (bFlag)
    {
        if (pList != NULL)
        {
            delete pList;
            pList = 0;
        }

        tPamAndNums::iterator aIt;

        for (aIt = aNumberRanges.begin(); aIt != aNumberRanges.end(); aIt++)
            (*aIt).second->InvalidateTree();
    }

    bInvalidRuleFlag = bFlag;
}

/* #109308# */
void SwNumRule::SetNumAdjust(SvxAdjust eNumAdjust)
{
    for (int i = 0; i < MAXLEVEL; i++)
        aFmts[i]->SetNumAdjust(eNumAdjust);
}

// #i27615#
SwBitArray SwNumRule::SetLevelMarked(BYTE nLvl, BOOL bVal)
{
    SwBitArray aTmpMarkedLevels(aMarkedLevels);

    aMarkedLevels.Set(nLvl, bVal);

    aTmpMarkedLevels = aTmpMarkedLevels ^ aMarkedLevels;

    return aTmpMarkedLevels;
}

// #i23725#, #i23726#
void SwNumRule::Indent(short nAmount, int nLevel, int nReferenceLevel,
                       BOOL bRelative, BOOL bFirstLine, BOOL bCheckGtZero)
{
    int nStartLevel = 0;
    int nEndLevel = MAXLEVEL - 1;
    BOOL bGotInvalid = FALSE;

    if (nLevel >= 0)
        nStartLevel = nEndLevel = nLevel;

    int i;
    short nRealAmount =  nAmount;

    if (! bRelative)
    {
        if (bFirstLine)
        {
            if (nReferenceLevel >= 0)
                nAmount -= Get(nReferenceLevel).GetFirstLineOffset();
            else
                nAmount -= Get(nStartLevel).GetFirstLineOffset();
        }

        BOOL bFirst = TRUE;

        if (nReferenceLevel >= 0)
            nRealAmount = nAmount - Get(nReferenceLevel).GetAbsLSpace();
        else
            for (i = nStartLevel; i < nEndLevel + 1; i++)
            {
                short nTmp = nAmount - Get(i).GetAbsLSpace();

                if (bFirst || nTmp > nRealAmount)
                {
                    nRealAmount = nTmp;
                    bFirst = FALSE;
                }
            }
    }

    if (nRealAmount < 0)
        for (i = nStartLevel; i < nEndLevel + 1; i++)
            if (Get(i).GetAbsLSpace() + nRealAmount < 0)
                nRealAmount = -Get(i).GetAbsLSpace();

    for (i = nStartLevel; i < nEndLevel + 1; i++)
    {
        short nNew = Get(i).GetAbsLSpace() + nRealAmount;

        if (bCheckGtZero && nNew < 0)
            nNew = 0;

        SwNumFmt aTmpNumFmt(Get(i));
        aTmpNumFmt.SetAbsLSpace(nNew);

        Set(i, aTmpNumFmt);

        bGotInvalid = TRUE;
    }

    if (bGotInvalid)
        SetInvalidRule(bGotInvalid);
}

sal_Unicode GetBulletChar(BYTE nLevel)
{
    static sal_Unicode nLevelChars[MAXLEVEL] =
        { 0x25cf, 0x25cb, 0x25a0, 0x25cf, 0x25cb,
          0x25a0, 0x25cf, 0x25cb, 0x25a0, 0x25cf };

    if (nLevel > MAXLEVEL)
        nLevel = MAXLEVEL;

    return nLevelChars[nLevel];
}

void SwNumRule::NewNumberRange(const SwPaM & rPam)
{
    SwNodeNum * pNum = new SwNodeNum();
    // --> OD 2005-10-21 - apply numbering rule to number tree root node for
    // correct creation of phantoms.
    pNum->SetNumRule( this );
    // <--

    SwPaM * pPam = new SwPaM(*rPam.Start(), *rPam.End());
    tPamAndNum aPamAndNum(pPam, pNum);

    aNumberRanges.push_back(aPamAndNum);
}

void SwNumRule::AddNumber(SwNodeNum * pNum, unsigned int nLevel)
{
    tPamAndNums::iterator aIt;

    SwPosition aPos(pNum->GetPosition());

    for (aIt = aNumberRanges.begin(); aIt != aNumberRanges.end(); aIt++)
    {
        SwPosition * pStart = (*aIt).first->Start();
        SwPosition * pEnd = (*aIt).first->End();
        SwNodes * pRangeNodes = &(pStart->nNode.GetNode().GetNodes());
        SwNodes * pNodes = &(aPos.nNode.GetNode().GetNodes());

        if (pRangeNodes == pNodes && *pStart <= aPos && aPos <= *pEnd)
        {
            pNum->SetNumRule(this);
            (*aIt).second->AddChild(pNum, nLevel);
        }
    }
}

void SwNumRule::Validate()
{
    tPamAndNums::iterator aIt;

    for (aIt = aNumberRanges.begin(); aIt != aNumberRanges.end(); aIt++)
        (*aIt).second->NotifyInvalidChildren();

    SetInvalidRule(FALSE);
}

bool SwNumRule::IsCountPhantoms() const
{
    return mbCountPhantoms;
}

void SwNumRule::SetCountPhantoms(bool bCountPhantoms)
{
    mbCountPhantoms = bCountPhantoms;
}

String SwNumRule::ToString() const
{
    String aResult("[ ", RTL_TEXTENCODING_ASCII_US);

    aResult += GetName();
    aResult += String("\n", RTL_TEXTENCODING_ASCII_US);

    tPamAndNums::const_iterator aIt;
    for (aIt = aNumberRanges.begin(); aIt != aNumberRanges.end(); aIt++)
        aResult += aIt->second->print();

    aResult += String("]\n", RTL_TEXTENCODING_ASCII_US);

    return aResult;
}
