/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: number.cxx,v $
 *
 *  $Revision: 1.48 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 15:55:49 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


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
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
// --> OD 2006-06-28 #b6440955#
// Needed to load default bullet list configuration
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
// <--
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif

#include <hash_map>

using namespace ::com::sun::star;


USHORT SwNumRule::nRefCount = 0;
SwNumFmt* SwNumRule::aBaseFmts[ RULE_END ][ MAXLEVEL ] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
// --> OD 2008-02-11 #newlistlevelattrs#
SwNumFmt* SwNumRule::aLabelAlignmentBaseFmts[ RULE_END ][ MAXLEVEL ] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

char sOutline[] = "Outline";
char* SwNumRule::pDefOutlineName = sOutline;

// #i30312#
USHORT SwNumRule::aDefNumIndents[ MAXLEVEL ] = {
//inch:   0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
        1440/4, 1440/2, 1440*3/4, 1440, 1440*5/4, 1440*3/2, 1440*7/4, 1440*2,
        1440*9/4, 1440*5/2
};

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

const SwNumFmt& SwNumRule::Get( USHORT i ) const
{
    ASSERT_ID( i < MAXLEVEL && eRuleType < RULE_END, ERR_NUMLEVEL);
    return aFmts[ i ]
           ? *aFmts[ i ]
           // --> OD 2008-02-11 #newlistlevelattrs#
           : ( meDefaultNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION
               ? *aBaseFmts[ eRuleType ][ i ]
               : *aLabelAlignmentBaseFmts[ eRuleType ][ i ] );
           // <--
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

// --> OD 2006-09-12 #i69145#
// Creates list of associated text nodes by copying contents of provided list <_pList>
void SwNumRule::SetList( const SwTxtNodeTable& rList )
{
    if ( pList )
        delete pList;

    pList = new SwTxtNodeTable( rList );
}
// <--

void SwNumRule::SetNumRuleMap(std::hash_map<String, SwNumRule *, StringHash> *
                              _pNumRuleMap)
{
    pNumRuleMap = _pNumRuleMap;
}

// --> OD 2006-06-27 #b6440955#
// function move to namespace <numfunc>
//const Font& SwNumRule::GetDefBulletFont()
//{
//    if( !pDefBulletFont )
//        SwNumRule::_MakeDefBulletFont();
//    return *pDefBulletFont;
//}
// <--

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
    SvxNumberFormat(SVX_NUM_ARABIC),
    SwClient( 0 ),
    pVertOrient(new SwFmtVertOrient( 0, text::VertOrientation::NONE))
{
}
/* -----------------------------22.02.01 13:42--------------------------------

 ---------------------------------------------------------------------------*/
SwNumFmt::SwNumFmt( const SwNumFmt& rFmt) :
    SvxNumberFormat(rFmt),
    SwClient( rFmt.pRegisteredIn ),
    pVertOrient(new SwFmtVertOrient( 0, rFmt.GetVertOrient()))
{
    sal_Int16 eMyVertOrient = rFmt.GetVertOrient();
    SetGraphicBrush( rFmt.GetBrush(), &rFmt.GetGraphicSize(),
                                                &eMyVertOrient);
}
/* -----------------------------22.02.01 13:58--------------------------------

 ---------------------------------------------------------------------------*/
SwNumFmt::SwNumFmt(const SvxNumberFormat& rNumFmt, SwDoc* pDoc) :
    SvxNumberFormat(rNumFmt),
    pVertOrient(new SwFmtVertOrient( 0, rNumFmt.GetVertOrient()))
{
    sal_Int16 eMyVertOrient = rNumFmt.GetVertOrient();
    SetGraphicBrush( rNumFmt.GetBrush(), &rNumFmt.GetGraphicSize(),
                                                &eMyVertOrient);
    const String& rCharStyleName = rNumFmt.SvxNumberFormat::GetCharFmtName();
    if( rCharStyleName.Len() )
    {
        SwCharFmt* pCFmt = pDoc->FindCharFmtByName( rCharStyleName );
        if( !pCFmt )
        {
            USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName( rCharStyleName,
                                            nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
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
    const sal_Int16* pOrient)
{
    if(pOrient)
        pVertOrient->SetVertOrient( *pOrient );
    SvxNumberFormat::SetGraphicBrush( pBrushItem, pSize, pOrient);
}
/* -----------------------------22.02.01 16:05--------------------------------

 ---------------------------------------------------------------------------*/
void    SwNumFmt::SetVertOrient(sal_Int16 eSet)
{
    SvxNumberFormat::SetVertOrient(eSet);
}
/* -----------------------------22.02.01 16:05--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int16   SwNumFmt::GetVertOrient() const
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
    sal_Int16  eOrient = SvxNumberFormat::GetVertOrient();
    if(text::VertOrientation::NONE == eOrient)
        return 0;
    else
    {
        pVertOrient->SetVertOrient(eOrient);
        return pVertOrient;
    }
}

#ifndef PRODUCT
long int SwNumRule::nInstances = 0;
#endif

// --> OD 2008-02-11 #newlistlevelattrs#
// handle new parameter <eDefaultNumberFormatPositionAndSpaceMode>
SwNumRule::SwNumRule( const String& rNm,
                      const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode,
                      SwNumRuleType eType,
                      BOOL bAutoFlg )
    : pList(0),
    aMarkedLevels( MAXLEVEL ), // #i27615#
    pNumRuleMap(0),
    sName( rNm ),
    eRuleType( eType ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX ),
    bAutoRuleFlag( bAutoFlg ),
    bInvalidRuleFlag( TRUE ),
    bContinusNum( FALSE ),
    bAbsSpaces( FALSE ),
    // --> OD 2005-10-21 - initialize member <mbCountPhantoms>
    mbCountPhantoms( true ),
    // <--
    // --> OD 2008-02-11 #newlistlevelattrs#
    meDefaultNumberFormatPositionAndSpaceMode( eDefaultNumberFormatPositionAndSpaceMode )
    // <--
{
#ifndef PRODUCT
    nSerial = nInstances++;
#endif

    if( !nRefCount++ )          // zum erstmal, also initialisiern
    {
        SwNumFmt* pFmt;
        BYTE n;

        // numbering:
        // position-and-space mode LABEL_WIDTH_AND_POSITION:
        for( n = 0; n < MAXLEVEL; ++n )
        {
            pFmt = new SwNumFmt;
            pFmt->SetIncludeUpperLevels( 1 );
            pFmt->SetStart( 1 );
            pFmt->SetLSpace( lNumIndent );
            pFmt->SetAbsLSpace( lNumIndent + SwNumRule::GetNumIndent( n ) );
            pFmt->SetFirstLineOffset( lNumFirstLineOffset );
            pFmt->SetSuffix( aDotStr );
            // --> OD 2006-06-29 #b6440955#
            pFmt->SetBulletChar( numfunc::GetBulletChar(n));
            // <--
            SwNumRule::aBaseFmts[ NUM_RULE ][ n ] = pFmt;
        }
        // --> OD 2008-02-11 #newlistlevelattrs#
        // position-and-space mode LABEL_ALIGNMENT
        // first line indent of general numbering in inch: -0,25 inch
        const long cFirstLineIndent = -1440/4;
        // indent values of general numbering in inch:
        //  0,5         0,75        1,0         1,25        1,5
        //  1,75        2,0         2,25        2,5         2,75
        const long cIndentAt[ MAXLEVEL ] = {
            1440/2,     1440*3/4,   1440,       1440*5/4,   1440*3/2,
            1440*7/4,   1440*2,     1440*9/4,   1440*5/2,   1440*11/4 };
        for( n = 0; n < MAXLEVEL; ++n )
        {
            pFmt = new SwNumFmt;
            pFmt->SetIncludeUpperLevels( 1 );
            pFmt->SetStart( 1 );
            // --> OD 2008-01-15 #newlistlevelattrs#
            pFmt->SetPositionAndSpaceMode( SvxNumberFormat::LABEL_ALIGNMENT );
            pFmt->SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
            pFmt->SetListtabPos( cIndentAt[ n ] );
            pFmt->SetFirstLineIndent( cFirstLineIndent );
            pFmt->SetIndentAt( cIndentAt[ n ] );
            // <--
            pFmt->SetSuffix( aDotStr );
            pFmt->SetBulletChar( numfunc::GetBulletChar(n));
            SwNumRule::aLabelAlignmentBaseFmts[ NUM_RULE ][ n ] = pFmt;
        }
        // <--

        // outline:
        // position-and-space mode LABEL_WIDTH_AND_POSITION:
        for( n = 0; n < MAXLEVEL; ++n )
        {
            pFmt = new SwNumFmt;
            pFmt->SetNumberingType(SVX_NUM_NUMBER_NONE);
            pFmt->SetIncludeUpperLevels( MAXLEVEL );
            pFmt->SetStart( 1 );
            pFmt->SetCharTextDistance( lOutlineMinTextDistance );
            // --> OD 2006-06-29 #b6440955#
            pFmt->SetBulletChar( numfunc::GetBulletChar(n));
            // <--
            SwNumRule::aBaseFmts[ OUTLINE_RULE ][ n ] = pFmt;
        }
        // --> OD 2008-02-11 #newlistlevelattrs#
        // position-and-space mode LABEL_ALIGNMENT:
        // indent values of default outline numbering in inch:
        //  0,3         0,4         0,5         0,6         0,7
        //  0,8         0,9         1,0         1,1         1,2
        const long cOutlineIndentAt[ MAXLEVEL ] = {
            1440*3/10,  1440*2/5,   1440/2,     1440*3/5,   1440*7/10,
            1440*4/5,   1440*9/10,  1440,       1440*11/10, 1440*6/5 };
        for( n = 0; n < MAXLEVEL; ++n )
        {
            pFmt = new SwNumFmt;
            pFmt->SetNumberingType(SVX_NUM_NUMBER_NONE);
            pFmt->SetIncludeUpperLevels( MAXLEVEL );
            pFmt->SetStart( 1 );
            pFmt->SetPositionAndSpaceMode( SvxNumberFormat::LABEL_ALIGNMENT );
            pFmt->SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
            pFmt->SetListtabPos( cOutlineIndentAt[ n ] );
            pFmt->SetFirstLineIndent( -cOutlineIndentAt[ n ] );
            pFmt->SetIndentAt( cOutlineIndentAt[ n ] );
            pFmt->SetBulletChar( numfunc::GetBulletChar(n));
            SwNumRule::aLabelAlignmentBaseFmts[ OUTLINE_RULE ][ n ] = pFmt;
        }
        // <--
    }
    memset( aFmts, 0, sizeof( aFmts ));
    ASSERT( sName.Len(), "NumRule ohne Namen!" );
}

SwNumRule::SwNumRule( const SwNumRule& rNumRule )
    : pList(0),
    aMarkedLevels( MAXLEVEL ), // #i27615#
    pNumRuleMap(0),
    sName( rNumRule.sName ),
    eRuleType( rNumRule.eRuleType ),
    nPoolFmtId( rNumRule.GetPoolFmtId() ),
    nPoolHelpId( rNumRule.GetPoolHelpId() ),
    nPoolHlpFileId( rNumRule.GetPoolHlpFileId() ),
    bAutoRuleFlag( rNumRule.bAutoRuleFlag ),
    bInvalidRuleFlag( TRUE ),
    bContinusNum( rNumRule.bContinusNum ),
    bAbsSpaces( rNumRule.bAbsSpaces ),
    // --> OD 2005-10-21 - initialize member <mbCountPhantoms>
    mbCountPhantoms( true ),
    // <--
    // --> OD 2008-02-11 #newlistlevelattrs#
    meDefaultNumberFormatPositionAndSpaceMode( rNumRule.meDefaultNumberFormatPositionAndSpaceMode )
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

            // --> OD 2008-02-11 #newlistlevelattrs#
            ppFmts = (SwNumFmt**)SwNumRule::aLabelAlignmentBaseFmts;
            for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
                delete *ppFmts, *ppFmts = 0;
            for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
                delete *ppFmts, *ppFmts = 0;
            // <--
    }

    tPamAndNums::iterator aIt;

    for(aIt = aNumberRanges.begin(); aIt != aNumberRanges.end(); aIt++)
    {
        delete (*aIt).first;
        // --> OD 2006-03-07 #131436#
        SwNodeNum::HandleNumberTreeRootNodeDelete( *((*aIt).second) );
        // <--
        delete (*aIt).second;
    }

    // --> OD 2006-09-12 #i69145#
    delete pList;
    pList = 0;
    // <--
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
                             bInclStrings, bOnlyArabic, MAXLEVEL);

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

    if (nLevel < MAXLEVEL)
    {
        const SwNumFmt& rMyNFmt = Get( static_cast<USHORT>(nLevel) );
        // --> OD 2006-06-02 #b6432095#
        // - levels with numbering none has to provide prefix and suffix string
//        if( SVX_NUM_NUMBER_NONE != rMyNFmt.GetNumberingType() )
        // <--
        {
            BYTE i = static_cast<BYTE>(nLevel);

            if( !IsContinusNum() &&
                // --> OD 2006-09-19 #i69672#
                // - do not include upper levels, if level isn't numbered.
                rMyNFmt.GetNumberingType() != SVX_NUM_NUMBER_NONE &&
                // <--
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

// --> OD 2007-09-07 #i81002#
String SwNumRule::MakeRefNumString( const SwNodeNum& rNodeNum,
                                    const bool bInclSuperiorNumLabels,
                                    const sal_uInt8 nRestrictInclToThisLevel ) const
{
    String aRefNumStr;

    if ( rNodeNum.GetLevel() >= 0 )
    {
        const SwNodeNum* pWorkingNodeNum( &rNodeNum );
        do
        {
            bool bMakeNumStringForPhantom( false );
            if ( pWorkingNodeNum->IsPhantom() )
            {
                SwNumFmt aFmt( Get( static_cast<USHORT>(pWorkingNodeNum->GetLevel()) ) );
                bMakeNumStringForPhantom = aFmt.IsEnumeration() &&
                                           SVX_NUM_NUMBER_NONE != aFmt.GetNumberingType();

            }
            if ( bMakeNumStringForPhantom ||
                 ( !pWorkingNodeNum->IsPhantom() &&
                   pWorkingNodeNum->GetTxtNode() &&
                   pWorkingNodeNum->GetTxtNode()->HasNumber() ) )
            {
                aRefNumStr.Insert( MakeNumString( pWorkingNodeNum->GetNumberVector() ), 0 );
            }
            else if ( aRefNumStr.Len() > 0 )
            {
                aRefNumStr.Insert( String::CreateFromAscii(" "), 0 );
            }

            if ( bInclSuperiorNumLabels && pWorkingNodeNum->GetLevel() > 0 )
            {
                BYTE n = Get( static_cast<USHORT>(pWorkingNodeNum->GetLevel()) ).GetIncludeUpperLevels();
                pWorkingNodeNum = dynamic_cast<SwNodeNum*>(pWorkingNodeNum->GetParent());
                // skip parents, whose list label is already contained in the actual list label.
                while ( pWorkingNodeNum && n > 1 )
                {
                    pWorkingNodeNum = dynamic_cast<SwNodeNum*>(pWorkingNodeNum->GetParent());
                    --n;
                }
            }
            else
            {
                break;
            }
        } while ( pWorkingNodeNum &&
                  pWorkingNodeNum->GetLevel() >= 0 &&
                  static_cast<sal_uInt8>(pWorkingNodeNum->GetLevel()) >= nRestrictInclToThisLevel );
    }

    return aRefNumStr;
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

    bInvalidRuleFlag = TRUE;
    bContinusNum = rNumRule.IsContinuousNumbering();
}
/* -----------------30.10.98 08:33-------------------
 *
 * --------------------------------------------------*/
SvxNumRule SwNumRule::MakeSvxNumRule() const
{
    SvxNumRule aRule(NUM_CONTINUOUS|NUM_CHAR_TEXT_DISTANCE|NUM_CHAR_STYLE|
                        NUM_ENABLE_LINKED_BMP|NUM_ENABLE_EMBEDDED_BMP,
                        MAXLEVEL, bContinusNum,
                        eRuleType ==
                            NUM_RULE ?
                                SVX_RULETYPE_NUMBERING :
                                    SVX_RULETYPE_OUTLINE_NUMBERING );
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
                nAmount = nAmount - Get(static_cast<USHORT>(nReferenceLevel)).GetFirstLineOffset();
            else
                nAmount = nAmount - Get(static_cast<USHORT>(nStartLevel)).GetFirstLineOffset();
        }

        BOOL bFirst = TRUE;

        if (nReferenceLevel >= 0)
            nRealAmount = nAmount - Get(static_cast<USHORT>(nReferenceLevel)).GetAbsLSpace();
        else
            for (i = nStartLevel; i < nEndLevel + 1; i++)
            {
                short nTmp = nAmount - Get(static_cast<USHORT>(i)).GetAbsLSpace();

                if (bFirst || nTmp > nRealAmount)
                {
                    nRealAmount = nTmp;
                    bFirst = FALSE;
                }
            }
    }

    if (nRealAmount < 0)
        for (i = nStartLevel; i < nEndLevel + 1; i++)
            if (Get(static_cast<USHORT>(i)).GetAbsLSpace() + nRealAmount < 0)
                nRealAmount = -Get(static_cast<USHORT>(i)).GetAbsLSpace();

    for (i = nStartLevel; i < nEndLevel + 1; i++)
    {
        short nNew = Get(static_cast<USHORT>(i)).GetAbsLSpace() + nRealAmount;

        if (bCheckGtZero && nNew < 0)
            nNew = 0;

        SwNumFmt aTmpNumFmt(Get(static_cast<USHORT>(i)));
        aTmpNumFmt.SetAbsLSpace(nNew);

        Set(static_cast<USHORT>(i), aTmpNumFmt);

        bGotInvalid = TRUE;
    }

    if (bGotInvalid)
        SetInvalidRule(bGotInvalid);
}

void SwNumRule::NewNumberRange(const SwPaM & rPam)
{
    // --> OD 2007-10-26 #i83479#
    SwNodeNum * pNum = new SwNodeNum( this );
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

// --> OD 2006-06-27 #b6440955#
namespace numfunc
{
    /** class containing default bullet list configuration data

        @author OD
    */
    class SwDefBulletConfig : private utl::ConfigItem
    {
        public:
            static SwDefBulletConfig* getInstance()
            {
                if ( mpInstance == 0 )
                {
                    mpInstance = new SwDefBulletConfig;
                }

                return mpInstance;
            }

            inline const String& GetFontname() const
            {
                return msFontname;
            }
            inline const Font& GetFont() const
            {
                return *mpFont;
            }
            inline const short GetFontWeight() const
            {
                return static_cast<short>(meFontWeight);
            }
            inline const short GetFontItalic() const
            {
                return static_cast<short>(meFontItalic);
            }
            inline const sal_Unicode GetChar( BYTE p_nListLevel ) const
            {
                if ( p_nListLevel > MAXLEVEL )
                {
                    p_nListLevel = MAXLEVEL;
                }

                return mnLevelChars[p_nListLevel];
            }

        private:
            SwDefBulletConfig();

            /** sets internal default bullet configuration data to default values

                @author OD
            */
            void SetToDefault();

            /** returns sequence of default bullet configuration property names

                @author OD
            */
            uno::Sequence<rtl::OUString> GetPropNames() const;

            /** loads default bullet configuration properties and applies
                values to internal data

                @author OD
            */
            void LoadConfig();

            /** initialize font instance for default bullet list

                @author OD
            */
            void InitFont();

            /** catches notification about changed default bullet configuration data

                @author OD
            */
            virtual void Notify( const uno::Sequence<rtl::OUString>& aPropertyNames );

            static SwDefBulletConfig* mpInstance;

            // default bullet list configuration data
            String msFontname;
            FontWeight meFontWeight;
            FontItalic meFontItalic;
            sal_Unicode mnLevelChars[MAXLEVEL];

            // default bullet list font instance
            Font* mpFont;
    };

    SwDefBulletConfig* SwDefBulletConfig::mpInstance = 0;

    SwDefBulletConfig::SwDefBulletConfig()
        : ConfigItem( rtl::OUString::createFromAscii("Office.Writer/Numbering/DefaultBulletList") ),
          msFontname( String::CreateFromAscii("StarSymbol") ),
          meFontWeight( WEIGHT_DONTKNOW ),
          meFontItalic( ITALIC_NONE ),
          mpFont( 0 )
    {
        SetToDefault();
        LoadConfig();
        InitFont();

        // enable notification for changes on default bullet configuration change
        EnableNotification( GetPropNames() );
    }

    void SwDefBulletConfig::SetToDefault()
    {
        msFontname = String::CreateFromAscii("StarSymbol");
        meFontWeight = WEIGHT_DONTKNOW;
        meFontItalic = ITALIC_NONE;

        mnLevelChars[0] = 0x25cf;
        mnLevelChars[1] = 0x25cb;
        mnLevelChars[2] = 0x25a0;
        mnLevelChars[3] = 0x25cf;
        mnLevelChars[4] = 0x25cb;
        mnLevelChars[5] = 0x25a0;
        mnLevelChars[6] = 0x25cf;
        mnLevelChars[7] = 0x25cb;
        mnLevelChars[8] = 0x25a0;
        mnLevelChars[9] = 0x25cf;
    }

    uno::Sequence<rtl::OUString> SwDefBulletConfig::GetPropNames() const
    {
        uno::Sequence<rtl::OUString> aPropNames(13);
        rtl::OUString* pNames = aPropNames.getArray();
        pNames[0] = rtl::OUString::createFromAscii("BulletFont/FontFamilyname");
        pNames[1] = rtl::OUString::createFromAscii("BulletFont/FontWeight");
        pNames[2] = rtl::OUString::createFromAscii("BulletFont/FontItalic");
        pNames[3] = rtl::OUString::createFromAscii("BulletCharLvl1");
        pNames[4] = rtl::OUString::createFromAscii("BulletCharLvl2");
        pNames[5] = rtl::OUString::createFromAscii("BulletCharLvl3");
        pNames[6] = rtl::OUString::createFromAscii("BulletCharLvl4");
        pNames[7] = rtl::OUString::createFromAscii("BulletCharLvl5");
        pNames[8] = rtl::OUString::createFromAscii("BulletCharLvl6");
        pNames[9] = rtl::OUString::createFromAscii("BulletCharLvl7");
        pNames[10] = rtl::OUString::createFromAscii("BulletCharLvl8");
        pNames[11] = rtl::OUString::createFromAscii("BulletCharLvl9");
        pNames[12] = rtl::OUString::createFromAscii("BulletCharLvl10");

        return aPropNames;
    }

    void SwDefBulletConfig::LoadConfig()
    {
        uno::Sequence<rtl::OUString> aPropNames = GetPropNames();
        uno::Sequence<uno::Any> aValues =
                                                    GetProperties( aPropNames );
        const uno::Any* pValues = aValues.getConstArray();
        ASSERT( aValues.getLength() == aPropNames.getLength(),
                "<SwDefBulletConfig::SwDefBulletConfig()> - GetProperties failed")
        if ( aValues.getLength() == aPropNames.getLength() )
        {
            for ( int nProp = 0; nProp < aPropNames.getLength(); ++nProp )
            {
                if ( pValues[nProp].hasValue() )
                {
                    switch ( nProp )
                    {
                        case 0:
                        {
                            rtl::OUString aStr;
                            pValues[nProp] >>= aStr;
                            msFontname = aStr;
                        }
                        break;
                        case 1:
                        case 2:
                        {
                            sal_uInt8 nTmp = 0;
                            pValues[nProp] >>= nTmp;
                            if ( nProp == 1 )
                                meFontWeight = static_cast<FontWeight>(nTmp);
                            else if ( nProp == 2 )
                                meFontItalic = static_cast<FontItalic>(nTmp);
                        }
                        break;
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        case 9:
                        case 10:
                        case 11:
                        case 12:
                        {
                            sal_Unicode cChar = sal_Unicode();
                            pValues[nProp] >>= cChar;
                            mnLevelChars[nProp-3] = cChar;
                        }
                        break;
                    }
                }
            }
        }

    }

    void SwDefBulletConfig::InitFont()
    {
        delete mpFont;

        mpFont = new Font( msFontname, aEmptyStr, Size( 0, 14 ) );
        mpFont->SetWeight( meFontWeight );
        mpFont->SetItalic( meFontItalic );
    }

    void SwDefBulletConfig::Notify( const uno::Sequence<rtl::OUString>& )
    {
        SetToDefault();
        LoadConfig();
        InitFont();
    }

    const String& GetDefBulletFontname()
    {
        return SwDefBulletConfig::getInstance()->GetFontname();
    }

    const Font& GetDefBulletFont()
    {
        return SwDefBulletConfig::getInstance()->GetFont();
    }

    const sal_Unicode GetBulletChar( BYTE nLevel )
    {
        return SwDefBulletConfig::getInstance()->GetChar( nLevel );
    }

    /** class containing configuration data about user interface behavior
        regarding lists and list items.

        OD 2007-10-01 #b660435#
        configuration item about behavior of <TAB>/<SHIFT-TAB>-key at first
        position of first list item

        @author OD
    */
    class SwNumberingUIBehaviorConfig : private utl::ConfigItem
    {
        public:
            static SwNumberingUIBehaviorConfig* getInstance()
            {
                if ( mpInstance == 0 )
                {
                    mpInstance = new SwNumberingUIBehaviorConfig();
                }

                return mpInstance;
            }

            inline const sal_Bool ChangeIndentOnTabAtFirstPosOfFirstListItem() const
            {
                return mbChangeIndentOnTabAtFirstPosOfFirstListItem;
            }

        private:
            SwNumberingUIBehaviorConfig();

            /** sets internal configuration data to default values

                @author OD
            */
            void SetToDefault();

            /** returns sequence of configuration property names

                @author OD
            */
            com::sun::star::uno::Sequence<rtl::OUString> GetPropNames() const;

            /** loads configuration properties and applies values to internal data

                @author OD
            */
            void LoadConfig();

            /** catches notification about changed configuration data

                @author OD
            */
            virtual void Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames );

            static SwNumberingUIBehaviorConfig* mpInstance;

            // configuration data
            sal_Bool mbChangeIndentOnTabAtFirstPosOfFirstListItem;
    };

    SwNumberingUIBehaviorConfig* SwNumberingUIBehaviorConfig::mpInstance = 0;

    SwNumberingUIBehaviorConfig::SwNumberingUIBehaviorConfig()
        : ConfigItem( rtl::OUString::createFromAscii("Office.Writer/Numbering/UserInterfaceBehavior") ),
          mbChangeIndentOnTabAtFirstPosOfFirstListItem( sal_True )
    {
        SetToDefault();
        LoadConfig();

        // enable notification for changes on configuration change
        EnableNotification( GetPropNames() );
    }

    void SwNumberingUIBehaviorConfig::SetToDefault()
    {
        mbChangeIndentOnTabAtFirstPosOfFirstListItem = sal_True;
    }

    com::sun::star::uno::Sequence<rtl::OUString> SwNumberingUIBehaviorConfig::GetPropNames() const
    {
        com::sun::star::uno::Sequence<rtl::OUString> aPropNames(1);
        rtl::OUString* pNames = aPropNames.getArray();
        pNames[0] = rtl::OUString::createFromAscii("ChangeIndentOnTabAtFirstPosOfFirstListItem");

        return aPropNames;
    }

    void SwNumberingUIBehaviorConfig::LoadConfig()
    {
        com::sun::star::uno::Sequence<rtl::OUString> aPropNames = GetPropNames();
        com::sun::star::uno::Sequence<com::sun::star::uno::Any> aValues =
                                                    GetProperties( aPropNames );
        const com::sun::star::uno::Any* pValues = aValues.getConstArray();
        ASSERT( aValues.getLength() == aPropNames.getLength(),
                "<SwNumberingUIBehaviorConfig::LoadConfig()> - GetProperties failed")
        if ( aValues.getLength() == aPropNames.getLength() )
        {
            for ( int nProp = 0; nProp < aPropNames.getLength(); ++nProp )
            {
                if ( pValues[nProp].hasValue() )
                {
                    switch ( nProp )
                    {
                        case 0:
                        {
                            pValues[nProp] >>= mbChangeIndentOnTabAtFirstPosOfFirstListItem;
                        }
                        break;
                        default:
                        {
                            ASSERT( false,
                                    "<SwNumberingUIBehaviorConfig::LoadConfig()> - unknown configuration property")
                        }
                    }
                }
            }
        }
    }

    void SwNumberingUIBehaviorConfig::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames )
    {
        (void) aPropertyNames;
        SetToDefault();
        LoadConfig();
    }

    const sal_Bool ChangeIndentOnTabAtFirstPosOfFirstListItem()
    {
        return SwNumberingUIBehaviorConfig::getInstance()->ChangeIndentOnTabAtFirstPosOfFirstListItem();
    }
}
