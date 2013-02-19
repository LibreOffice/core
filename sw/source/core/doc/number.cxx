/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <hintids.hxx>

#include <string.h>
#include <vcl/font.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/numitem.hxx>
#include <fmtornt.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <charfmt.hxx>
#include <paratr.hxx>
#include <frmfmt.hxx>
#include <ndtxt.hxx>
#include <docary.hxx>
#include <docsh.hxx>
#include <SwStyleNameMapper.hxx>

// Needed to load default bullet list configuration
#include <unotools/configitem.hxx>

#include <numrule.hxx>
#include <SwNodeNum.hxx>

#include <boost/unordered_map.hpp>

#include <list.hxx>
#include <algorithm>

#include <unotools/saveopt.hxx>

#include <IDocumentListsAccess.hxx>

using namespace ::com::sun::star;


sal_uInt16 SwNumRule::nRefCount = 0;
SwNumFmt* SwNumRule::aBaseFmts[ RULE_END ][ MAXLEVEL ] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

SwNumFmt* SwNumRule::aLabelAlignmentBaseFmts[ RULE_END ][ MAXLEVEL ] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

char sOutline[] = "Outline";
char* SwNumRule::pDefOutlineName = sOutline;

sal_uInt16 SwNumRule::aDefNumIndents[ MAXLEVEL ] = {
//inch:   0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
        1440/4, 1440/2, 1440*3/4, 1440, 1440*5/4, 1440*3/2, 1440*7/4, 1440*2,
        1440*9/4, 1440*5/2
};

const SwNumFmt& SwNumRule::Get( sal_uInt16 i ) const
{
    OSL_ASSERT( i < MAXLEVEL && eRuleType < RULE_END );
    return aFmts[ i ]
           ? *aFmts[ i ]
           : ( meDefaultNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION
               ? *aBaseFmts[ eRuleType ][ i ]
               : *aLabelAlignmentBaseFmts[ eRuleType ][ i ] );
}

const SwNumFmt* SwNumRule::GetNumFmt( sal_uInt16 i ) const
{
    const SwNumFmt * pResult = NULL;

    OSL_ASSERT( i < MAXLEVEL && eRuleType < RULE_END );
    if ( i < MAXLEVEL && eRuleType < RULE_END)
    {
        pResult = aFmts[ i ];
    }

    return pResult;
}

// #i91400#
void SwNumRule::SetName( const String & rName,
                         IDocumentListsAccess& rDocListAccess)
{
    if ( sName != rName )
    {
        if (pNumRuleMap)
        {
            pNumRuleMap->erase(sName);
            (*pNumRuleMap)[rName] = this;

            if ( GetDefaultListId().Len() > 0 )
            {
                rDocListAccess.trackChangeOfListStyleName( sName, rName );
            }
        }

        sName = rName;
    }
}


void SwNumRule::GetTxtNodeList( SwNumRule::tTxtNodeList& rTxtNodeList ) const
{
    rTxtNodeList = maTxtNodeList;
}


SwNumRule::tTxtNodeList::size_type SwNumRule::GetTxtNodeListSize() const
{
    return maTxtNodeList.size();
}


void SwNumRule::AddTxtNode( SwTxtNode& rTxtNode )
{
    tTxtNodeList::iterator aIter =
        std::find( maTxtNodeList.begin(), maTxtNodeList.end(), &rTxtNode );

    if ( aIter == maTxtNodeList.end() )
    {
        maTxtNodeList.push_back( &rTxtNode );
    }
}


void SwNumRule::RemoveTxtNode( SwTxtNode& rTxtNode )
{
    tTxtNodeList::iterator aIter =
        std::find( maTxtNodeList.begin(), maTxtNodeList.end(), &rTxtNode );

    if ( aIter != maTxtNodeList.end() )
    {
        maTxtNodeList.erase( aIter );
    }
}


void SwNumRule::SetNumRuleMap(boost::unordered_map<String, SwNumRule *, StringHash> *
                              _pNumRuleMap)
{
    pNumRuleMap = _pNumRuleMap;
}

sal_uInt16 SwNumRule::GetNumIndent( sal_uInt8 nLvl )
{
    OSL_ENSURE( MAXLEVEL > nLvl, "NumLevel is out of range" );
    return aDefNumIndents[ nLvl ];
}

sal_uInt16 SwNumRule::GetBullIndent( sal_uInt8 nLvl )
{
    OSL_ENSURE( MAXLEVEL > nLvl, "NumLevel is out of range" );
    return aDefNumIndents[ nLvl ];
}

static void lcl_SetRuleChgd( SwTxtNode& rNd, sal_uInt8 nLevel )
{
    if( rNd.GetActualListLevel() == nLevel )
        rNd.NumRuleChgd();
}

SwNumFmt::SwNumFmt() :
    SvxNumberFormat(SVX_NUM_ARABIC),
    SwClient( 0 ),
    pVertOrient(new SwFmtVertOrient( 0, text::VertOrientation::NONE))
{
}

SwNumFmt::SwNumFmt( const SwNumFmt& rFmt) :
    SvxNumberFormat(rFmt),
    SwClient( rFmt.GetRegisteredInNonConst() ),
    pVertOrient(new SwFmtVertOrient( 0, rFmt.GetVertOrient()))
{
    sal_Int16 eMyVertOrient = rFmt.GetVertOrient();
    SetGraphicBrush( rFmt.GetBrush(), &rFmt.GetGraphicSize(),
                                                &eMyVertOrient);
}

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
            sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( rCharStyleName,
                                            nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            pCFmt = nId != USHRT_MAX
                        ? pDoc->GetCharFmtFromPool( nId )
                        : pDoc->MakeCharFmt( rCharStyleName, 0 );
        }
        pCFmt->Add( this );
    }
    else if( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );

}

SwNumFmt::~SwNumFmt()
{
    delete pVertOrient;
}

void SwNumFmt::NotifyGraphicArrived()
{
    if( GetCharFmt() )
        UpdateNumNodes( (SwDoc*)GetCharFmt()->GetDoc() );
}

// #i22362#
sal_Bool SwNumFmt::IsEnumeration() const
{
    // #i30655# native numbering did not work any longer
    // using this code. Therefore HBRINKM and I agreed upon defining
    // IsEnumeration() as !IsItemize()
    return !IsItemize();
}


sal_Bool SwNumFmt::IsItemize() const
{
    sal_Bool bResult;

    switch(GetNumberingType())
    {
    case SVX_NUM_CHAR_SPECIAL:
    case SVX_NUM_BITMAP:
        bResult = sal_True;

        break;

    default:
        bResult = sal_False;
    }

    return bResult;

}

SwNumFmt& SwNumFmt::operator=( const SwNumFmt& rNumFmt)
{
    SvxNumberFormat::operator=(rNumFmt);
    if( rNumFmt.GetRegisteredIn() )
        rNumFmt.GetRegisteredInNonConst()->Add( this );
    else if( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );
    return *this;
}

sal_Bool SwNumFmt::operator==( const SwNumFmt& rNumFmt) const
{
    sal_Bool bRet = SvxNumberFormat::operator==(rNumFmt) &&
        GetRegisteredIn() == rNumFmt.GetRegisteredIn();
    return bRet;
}

void SwNumFmt::SetCharFmt( SwCharFmt* pChFmt)
{
    if( pChFmt )
        pChFmt->Add( this );
    else if( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );
}

void SwNumFmt::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    // Look for the NumRules object in the Doc where this NumFormat is set.
    // The format does not need to exist!
    const SwCharFmt* pFmt = 0;
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
    case RES_ATTRSET_CHG:
    case RES_FMT_CHG:
        pFmt = GetCharFmt();
        break;
    }

    if( pFmt && !pFmt->GetDoc()->IsInDtor() )
        UpdateNumNodes( (SwDoc*)pFmt->GetDoc() );
    else
        CheckRegistration( pOld, pNew );
}

void SwNumFmt::SetCharFmtName(const String& rSet)
{
    SvxNumberFormat::SetCharFmtName(rSet);
}

const String&   SwNumFmt::GetCharFmtName() const
{
    if((SwCharFmt*)GetRegisteredIn())
        return ((SwCharFmt*)GetRegisteredIn())->GetName();
    else
        return aEmptyStr;
}

void    SwNumFmt::SetGraphicBrush( const SvxBrushItem* pBrushItem, const Size* pSize,
    const sal_Int16* pOrient)
{
    if(pOrient)
        pVertOrient->SetVertOrient( *pOrient );
    SvxNumberFormat::SetGraphicBrush( pBrushItem, pSize, pOrient);
}

void    SwNumFmt::SetVertOrient(sal_Int16 eSet)
{
    SvxNumberFormat::SetVertOrient(eSet);
}

sal_Int16   SwNumFmt::GetVertOrient() const
{
    return SvxNumberFormat::GetVertOrient();
}

void SwNumFmt::UpdateNumNodes( SwDoc* pDoc )
{
    sal_Bool bDocIsModified = pDoc->IsModified();
    bool bFnd = false;
    const SwNumRule* pRule;
    for( sal_uInt16 n = pDoc->GetNumRuleTbl().size(); !bFnd && n; )
    {
        pRule = pDoc->GetNumRuleTbl()[ --n ];
        for( sal_uInt8 i = 0; i < MAXLEVEL; ++i )
            if( pRule->GetNumFmt( i ) == this )
            {
                SwNumRule::tTxtNodeList aTxtNodeList;
                pRule->GetTxtNodeList( aTxtNodeList );
                for ( SwNumRule::tTxtNodeList::iterator aIter = aTxtNodeList.begin();
                      aIter != aTxtNodeList.end(); ++aIter )
                {
                    lcl_SetRuleChgd( *(*aIter), i );
                }
                bFnd = true;
                break;
            }
    }

    if( bFnd && !bDocIsModified )
        pDoc->ResetModified();
}

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


// handle new parameter <eDefaultNumberFormatPositionAndSpaceMode>
SwNumRule::SwNumRule( const String& rNm,
                      const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode,
                      SwNumRuleType eType,
                      sal_Bool bAutoFlg )
    : maTxtNodeList(),
      maParagraphStyleList(),
    pNumRuleMap(0),
    sName( rNm ),
    eRuleType( eType ),
    nPoolFmtId( USHRT_MAX ),
    nPoolHelpId( USHRT_MAX ),
    nPoolHlpFileId( UCHAR_MAX ),
    bAutoRuleFlag( bAutoFlg ),
    bInvalidRuleFlag( sal_True ),
    bContinusNum( sal_False ),
    bAbsSpaces( sal_False ),
    bHidden( sal_False ),
    mbCountPhantoms( true ),
    meDefaultNumberFormatPositionAndSpaceMode( eDefaultNumberFormatPositionAndSpaceMode ),
    msDefaultListId()
{
    if( !nRefCount++ )          // for the first time, initialize
    {
        SwNumFmt* pFmt;
        sal_uInt8 n;

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
            pFmt->SetBulletChar( numfunc::GetBulletChar(n));
            SwNumRule::aBaseFmts[ NUM_RULE ][ n ] = pFmt;
        }
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
            pFmt->SetPositionAndSpaceMode( SvxNumberFormat::LABEL_ALIGNMENT );
            pFmt->SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
            pFmt->SetListtabPos( cIndentAt[ n ] );
            pFmt->SetFirstLineIndent( cFirstLineIndent );
            pFmt->SetIndentAt( cIndentAt[ n ] );
            pFmt->SetSuffix( aDotStr );
            pFmt->SetBulletChar( numfunc::GetBulletChar(n));
            SwNumRule::aLabelAlignmentBaseFmts[ NUM_RULE ][ n ] = pFmt;
        }

        // outline:
        // position-and-space mode LABEL_WIDTH_AND_POSITION:
        for( n = 0; n < MAXLEVEL; ++n )
        {
            pFmt = new SwNumFmt;
            pFmt->SetNumberingType(SVX_NUM_NUMBER_NONE);
            pFmt->SetIncludeUpperLevels( MAXLEVEL );
            pFmt->SetStart( 1 );
            pFmt->SetCharTextDistance( lOutlineMinTextDistance );
            pFmt->SetBulletChar( numfunc::GetBulletChar(n));
            SwNumRule::aBaseFmts[ OUTLINE_RULE ][ n ] = pFmt;
        }
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
    }
    memset( aFmts, 0, sizeof( aFmts ));
    OSL_ENSURE( sName.Len(), "NumRule without a name!" );
}

SwNumRule::SwNumRule( const SwNumRule& rNumRule )
    : maTxtNodeList(),
      maParagraphStyleList(),
      pNumRuleMap(0),
      sName( rNumRule.sName ),
      eRuleType( rNumRule.eRuleType ),
      nPoolFmtId( rNumRule.GetPoolFmtId() ),
      nPoolHelpId( rNumRule.GetPoolHelpId() ),
      nPoolHlpFileId( rNumRule.GetPoolHlpFileId() ),
      bAutoRuleFlag( rNumRule.bAutoRuleFlag ),
      bInvalidRuleFlag( sal_True ),
      bContinusNum( rNumRule.bContinusNum ),
      bAbsSpaces( rNumRule.bAbsSpaces ),
      bHidden( rNumRule.bHidden ),
      mbCountPhantoms( true ),
      meDefaultNumberFormatPositionAndSpaceMode( rNumRule.meDefaultNumberFormatPositionAndSpaceMode ),
      msDefaultListId( rNumRule.msDefaultListId )
{
    ++nRefCount;
    memset( aFmts, 0, sizeof( aFmts ));
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
        if( rNumRule.aFmts[ n ] )
            Set( n, *rNumRule.aFmts[ n ] );
}

SwNumRule::~SwNumRule()
{
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
        delete aFmts[ n ];

    if (pNumRuleMap)
    {
        pNumRuleMap->erase(GetName());
    }

    if( !--nRefCount )          // the last one closes the door (?)
    {
            // Numbering:
            SwNumFmt** ppFmts = (SwNumFmt**)SwNumRule::aBaseFmts;
            int n;

            for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
                delete *ppFmts, *ppFmts = 0;

            // Outline:
            for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
                delete *ppFmts, *ppFmts = 0;

            ppFmts = (SwNumFmt**)SwNumRule::aLabelAlignmentBaseFmts;
            for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
                delete *ppFmts, *ppFmts = 0;
            for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
                delete *ppFmts, *ppFmts = 0;
    }

    maTxtNodeList.clear();
    maParagraphStyleList.clear();
}

void SwNumRule::CheckCharFmts( SwDoc* pDoc )
{
    SwCharFmt* pFmt;
    for( sal_uInt8 n = 0; n < MAXLEVEL; ++n )
        if( aFmts[ n ] && 0 != ( pFmt = aFmts[ n ]->GetCharFmt() ) &&
            pFmt->GetDoc() != pDoc )
        {
            // copy
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
        for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
            Set( n, rNumRule.aFmts[ n ] );

        eRuleType = rNumRule.eRuleType;
        sName = rNumRule.sName;
        bAutoRuleFlag = rNumRule.bAutoRuleFlag;
        bInvalidRuleFlag = sal_True;
        bContinusNum = rNumRule.bContinusNum;
        bAbsSpaces = rNumRule.bAbsSpaces;
        bHidden = rNumRule.bHidden;
        nPoolFmtId = rNumRule.GetPoolFmtId();
        nPoolHelpId = rNumRule.GetPoolHelpId();
        nPoolHlpFileId = rNumRule.GetPoolHlpFileId();
    }
    return *this;
}

sal_Bool SwNumRule::operator==( const SwNumRule& rRule ) const
{
    sal_Bool bRet = eRuleType == rRule.eRuleType &&
                sName == rRule.sName &&
                bAutoRuleFlag == rRule.bAutoRuleFlag &&
                bContinusNum == rRule.bContinusNum &&
                bAbsSpaces == rRule.bAbsSpaces &&
                nPoolFmtId == rRule.GetPoolFmtId() &&
                nPoolHelpId == rRule.GetPoolHelpId() &&
                nPoolHlpFileId == rRule.GetPoolHlpFileId();
    if( bRet )
    {
        for( sal_uInt8 n = 0; n < MAXLEVEL; ++n )
            if( !( rRule.Get( n ) == Get( n ) ))
            {
                bRet = sal_False;
                break;
            }
    }
    return bRet;
}

void SwNumRule::Set( sal_uInt16 i, const SwNumFmt& rNumFmt )
{
    OSL_ENSURE( i < MAXLEVEL, "Serious defect, please inform OD" );
    if( i < MAXLEVEL )
    {
        if( !aFmts[ i ] || !(rNumFmt == Get( i )) )
        {
            delete aFmts[ i ];
            aFmts[ i ] = new SwNumFmt( rNumFmt );
            bInvalidRuleFlag = sal_True;
        }
    }
}

void SwNumRule::Set( sal_uInt16 i, const SwNumFmt* pNumFmt )
{
    OSL_ENSURE( i < MAXLEVEL, "Serious defect, please inform OD" );
    if( i >= MAXLEVEL )
        return;
    SwNumFmt* pOld = aFmts[ i ];
    if( !pOld )
    {
        if( pNumFmt )
        {
            aFmts[ i ] = new SwNumFmt( *pNumFmt );
            bInvalidRuleFlag = sal_True;
        }
    }
    else if( !pNumFmt )
        delete pOld, aFmts[ i ] = 0, bInvalidRuleFlag = sal_True;
    else if( *pOld != *pNumFmt )
        *pOld = *pNumFmt, bInvalidRuleFlag = sal_True;
}

String SwNumRule::MakeNumString( const SwNodeNum& rNum, sal_Bool bInclStrings,
                                sal_Bool bOnlyArabic ) const
{
    String aStr;

    if (rNum.IsCounted())
        aStr = MakeNumString(rNum.GetNumberVector(),
                             bInclStrings, bOnlyArabic, MAXLEVEL);

    return aStr;
}

String SwNumRule::MakeNumString( const SwNumberTree::tNumberVector & rNumVector,
                                 const sal_Bool bInclStrings,
                                 const sal_Bool bOnlyArabic,
                                 const unsigned int _nRestrictToThisLevel,
                                 SwNumRule::Extremities* pExtremities ) const
{
    String aStr;

    unsigned int nLevel = rNumVector.size() - 1;

    if ( pExtremities )
        pExtremities->nPrefixChars = pExtremities->nSuffixChars = 0;

    if ( nLevel > _nRestrictToThisLevel )
    {
        nLevel = _nRestrictToThisLevel;
    }

    if (nLevel < MAXLEVEL)
    {
        const SwNumFmt& rMyNFmt = Get( static_cast<sal_uInt16>(nLevel) );

        {
            sal_uInt8 i = static_cast<sal_uInt8>(nLevel);

            if( !IsContinusNum() &&
                // - do not include upper levels, if level isn't numbered.
                rMyNFmt.GetNumberingType() != SVX_NUM_NUMBER_NONE &&
                rMyNFmt.GetIncludeUpperLevels() )  // Just the own level?
            {
                sal_uInt8 n = rMyNFmt.GetIncludeUpperLevels();
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
                    // Should 1.1.1 --> 2. NoNum --> 1..1 or 1.1 ??
                    //                 if( i != rNum.nMyLevel )
                    //                    aStr += aDotStr;
                    continue;
                }

                if( rNumVector[ i ] )
                {
                    if( bOnlyArabic )
                        aStr += OUString::number( rNumVector[ i ] );
                    else
                        aStr += rNFmt.GetNumStr( rNumVector[ i ] );
                }
                else
                    aStr += '0';        // all 0 level are a 0
                if( i != nLevel && aStr.Len() )
                    aStr += aDotStr;
            }

            // The type doesn't have any number, so don't append
            // the post-/prefix string
            if( bInclStrings && !bOnlyArabic &&
                SVX_NUM_CHAR_SPECIAL != rMyNFmt.GetNumberingType() &&
                SVX_NUM_BITMAP != rMyNFmt.GetNumberingType() )
            {
                String const &rPrefix = rMyNFmt.GetPrefix();
                String const &rSuffix = rMyNFmt.GetSuffix();

                aStr.Insert( rPrefix, 0 );
                aStr += rSuffix;
                if ( pExtremities )
                {
                    pExtremities->nPrefixChars = rPrefix.Len();
                    pExtremities->nSuffixChars = rSuffix.Len();
                }
            }
        }
    }

    return aStr;
}


String SwNumRule::MakeRefNumString( const SwNodeNum& rNodeNum,
                                    const bool bInclSuperiorNumLabels,
                                    const sal_uInt8 nRestrictInclToThisLevel ) const
{
    String aRefNumStr;

    if ( rNodeNum.GetLevelInListTree() >= 0 )
    {
        bool bOldHadPrefix = true;
        bool bFirstIteration = true;
        ::rtl::OUString sOldPrefix;

        const SwNodeNum* pWorkingNodeNum( &rNodeNum );
        do
        {
            bool bMakeNumStringForPhantom( false );
            if ( pWorkingNodeNum->IsPhantom() )
            {
                SwNumFmt aFmt( Get( static_cast<sal_uInt16>(pWorkingNodeNum->GetLevelInListTree()) ) );
                bMakeNumStringForPhantom = aFmt.IsEnumeration() &&
                                           SVX_NUM_NUMBER_NONE != aFmt.GetNumberingType();

            }
            if ( bMakeNumStringForPhantom ||
                 ( !pWorkingNodeNum->IsPhantom() &&
                   pWorkingNodeNum->GetTxtNode() &&
                   pWorkingNodeNum->GetTxtNode()->HasNumber() ) )
            {
                Extremities aExtremities;
                String aPrevStr = MakeNumString( pWorkingNodeNum->GetNumberVector(),
                                                 sal_True, sal_False, MAXLEVEL,
                                                 &aExtremities);
                int        nStrip = 0;
                sal_Unicode        c;


                while ( nStrip < aExtremities.nPrefixChars &&
                       ( '\t' == ( c = aPrevStr.GetChar( nStrip ) ) ||
                         ' ' == c) )
                {
                        ++nStrip;
                }

                if (nStrip)
                {
                    aPrevStr.Erase( 0, nStrip );
                    aExtremities.nPrefixChars -= nStrip;
                }

                if ((bFirstIteration || bOldHadPrefix) &&
                     aExtremities.nSuffixChars &&
                     !aExtremities.nPrefixChars
                   )
                {
                    int nStrip2 = aPrevStr.Len();
                    while (aPrevStr.Len() - nStrip2 < aExtremities.nSuffixChars)
                    {
                        char const cur = aPrevStr.GetChar(nStrip2);
                        if  (!bFirstIteration && '\t' != cur && ' ' != cur)
                        {
                            break;
                        }
                        --nStrip2;
                    }
                    if (nStrip2 < aPrevStr.Len())
                    {
                        aPrevStr.Erase(nStrip2, aPrevStr.Len() - nStrip2);
                    }
                }
                else if (sOldPrefix.getLength())
                {
                    aRefNumStr.Insert(sOldPrefix, 0);
                }
                sOldPrefix = ::rtl::OUString();

                bOldHadPrefix = ( aExtremities.nPrefixChars >  0);

                aRefNumStr.Insert( aPrevStr, 0 );
            }
            else if ( aRefNumStr.Len() > 0 )
            {
                sOldPrefix += " ";
                bOldHadPrefix = true;
            }

            if ( bInclSuperiorNumLabels && pWorkingNodeNum->GetLevelInListTree() > 0 )
            {
                sal_uInt8 n = Get( static_cast<sal_uInt16>(pWorkingNodeNum->GetLevelInListTree()) ).GetIncludeUpperLevels();
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
            bFirstIteration = false;
        } while ( pWorkingNodeNum &&
                  pWorkingNodeNum->GetLevelInListTree() >= 0 &&
                  static_cast<sal_uInt8>(pWorkingNodeNum->GetLevelInListTree()) >= nRestrictInclToThisLevel );
    }

    return aRefNumStr;
}

//  ----- Copy method of SwNumRule ------

    // A kind of copy constructor, so that the num formats are attached
    // to the right CharFormats of a Document.
    // Copies the NumFormats and returns itself.
SwNumRule& SwNumRule::CopyNumRule( SwDoc* pDoc, const SwNumRule& rNumRule )
{
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        Set( n, rNumRule.aFmts[ n ] );
        if( aFmts[ n ] && aFmts[ n ]->GetCharFmt() &&
            !pDoc->GetCharFmts()->Contains( aFmts[n]->GetCharFmt() ))
            // If we copy across different Documents, then copy the
            // corresponding CharFormat into the new Document.
            aFmts[n]->SetCharFmt( pDoc->CopyCharFmt( *aFmts[n]->
                                        GetCharFmt() ) );
    }
    eRuleType = rNumRule.eRuleType;
    sName = rNumRule.sName;
    bAutoRuleFlag = rNumRule.bAutoRuleFlag;
    nPoolFmtId = rNumRule.GetPoolFmtId();
    nPoolHelpId = rNumRule.GetPoolHelpId();
    nPoolHlpFileId = rNumRule.GetPoolHlpFileId();
    bInvalidRuleFlag = sal_True;
    return *this;
}

void SwNumRule::SetSvxRule(const SvxNumRule& rNumRule, SwDoc* pDoc)
{
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        const SvxNumberFormat* pSvxFmt = rNumRule.Get(n);
        delete aFmts[n];
        aFmts[n] = pSvxFmt ? new SwNumFmt(*pSvxFmt, pDoc) : 0;
    }

    bInvalidRuleFlag = sal_True;
    bContinusNum = rNumRule.IsContinuousNumbering();
}

SvxNumRule SwNumRule::MakeSvxNumRule() const
{
    SvxNumRule aRule(NUM_CONTINUOUS|NUM_CHAR_TEXT_DISTANCE|NUM_CHAR_STYLE|
                        NUM_ENABLE_LINKED_BMP|NUM_ENABLE_EMBEDDED_BMP,
                        MAXLEVEL, bContinusNum,
                        eRuleType ==
                            NUM_RULE ?
                                SVX_RULETYPE_NUMBERING :
                                    SVX_RULETYPE_OUTLINE_NUMBERING );
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        SwNumFmt aNumFmt = Get(n);
        if(aNumFmt.GetCharFmt())
            aNumFmt.SetCharFmtName(aNumFmt.GetCharFmt()->GetName());
        aRule.SetLevel(n, aNumFmt, aFmts[n] != 0);
    }
    return aRule;
}

void SwNumRule::SetInvalidRule(sal_Bool bFlag)
{
    if (bFlag)
    {
        std::set< SwList* > aLists;
        tTxtNodeList::iterator aIter;
        for ( aIter = maTxtNodeList.begin(); aIter != maTxtNodeList.end(); ++aIter )
        {
            const SwTxtNode* pTxtNode = *aIter;
            // #i111681# - applying patch from cmc
            SwList* pList = pTxtNode->GetDoc()->getListByName( pTxtNode->GetListId() );
            OSL_ENSURE( pList, "<SwNumRule::SetInvalidRule(..)> - list at which the text node is registered at does not exist. This is a serious issue --> please inform OD.");
            if ( pList )
            {
                aLists.insert( pList );
            }
        }
        std::for_each( aLists.begin(), aLists.end(),
                       std::mem_fun( &SwList::InvalidateListTree ) );
    }

    bInvalidRuleFlag = bFlag;
}


// change indent of all list levels by given difference
void SwNumRule::ChangeIndent( const short nDiff )
{
    for ( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        SwNumFmt aTmpNumFmt( Get(i) );

        const SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode(
                                        aTmpNumFmt.GetPositionAndSpaceMode() );
        if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
        {
            short nNewIndent = nDiff +
                               aTmpNumFmt.GetAbsLSpace();
            if ( nNewIndent < 0 )
            {
                nNewIndent = 0;
            }
            aTmpNumFmt.SetAbsLSpace( nNewIndent );
        }
        else if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            // adjust also the list tab position, if a list tab stop is applied
            if ( aTmpNumFmt.GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
            {
                const long nNewListTab = aTmpNumFmt.GetListtabPos() +  nDiff;
                aTmpNumFmt.SetListtabPos( nNewListTab );
            }

            const long nNewIndent = nDiff +
                              aTmpNumFmt.GetIndentAt();
            aTmpNumFmt.SetIndentAt( nNewIndent );
        }

        Set( i, aTmpNumFmt );
    }

    SetInvalidRule( sal_True );
}

// set indent of certain list level to given value
void SwNumRule::SetIndent( const short nNewIndent,
                           const sal_uInt16 nListLevel )
{
    SwNumFmt aTmpNumFmt( Get(nListLevel) );

    const SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode(
                                        aTmpNumFmt.GetPositionAndSpaceMode() );
    if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        aTmpNumFmt.SetAbsLSpace( nNewIndent );
    }
    else if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        // adjust also the list tab position, if a list tab stop is applied
        if ( aTmpNumFmt.GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
        {
            const long nNewListTab = aTmpNumFmt.GetListtabPos() +
                                     ( nNewIndent - aTmpNumFmt.GetIndentAt() );
            aTmpNumFmt.SetListtabPos( nNewListTab );
        }

        aTmpNumFmt.SetIndentAt( nNewIndent );
    }

    SetInvalidRule( sal_True );
}

// set indent of first list level to given value and change other list level's
// indents accordingly
void SwNumRule::SetIndentOfFirstListLevelAndChangeOthers( const short nNewIndent )
{
    SwNumFmt aTmpNumFmt( Get(0) );

    short nDiff( 0 );
    const SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode(
                                        aTmpNumFmt.GetPositionAndSpaceMode() );
    if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        nDiff = nNewIndent
                - aTmpNumFmt.GetFirstLineOffset()
                - aTmpNumFmt.GetAbsLSpace();
    }
    else if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        nDiff = static_cast<short>(nNewIndent
                                   - aTmpNumFmt.GetIndentAt());
    }
    if ( nDiff != 0  )
    {
        ChangeIndent( nDiff );
    }
}


void SwNumRule::Validate()
{
    std::set< SwList* > aLists;
    tTxtNodeList::iterator aIter;
    for ( aIter = maTxtNodeList.begin(); aIter != maTxtNodeList.end(); ++aIter )
    {
        const SwTxtNode* pTxtNode = *aIter;
        aLists.insert( pTxtNode->GetDoc()->getListByName( pTxtNode->GetListId() ) );
    }
    std::for_each( aLists.begin(), aLists.end(),
                   std::mem_fun( &SwList::ValidateListTree ) );


    SetInvalidRule(sal_False);
}


bool SwNumRule::IsCountPhantoms() const
{
    return mbCountPhantoms;
}


void SwNumRule::SetCountPhantoms(bool bCountPhantoms)
{
    mbCountPhantoms = bCountPhantoms;
}


SwNumRule::tParagraphStyleList::size_type SwNumRule::GetParagraphStyleListSize() const
{
    return maParagraphStyleList.size();
}


void SwNumRule::AddParagraphStyle( SwTxtFmtColl& rTxtFmtColl )
{
    tParagraphStyleList::iterator aIter =
        std::find( maParagraphStyleList.begin(), maParagraphStyleList.end(), &rTxtFmtColl );

    if ( aIter == maParagraphStyleList.end() )
    {
        maParagraphStyleList.push_back( &rTxtFmtColl );
    }
}


void SwNumRule::RemoveParagraphStyle( SwTxtFmtColl& rTxtFmtColl )
{
    tParagraphStyleList::iterator aIter =
        std::find( maParagraphStyleList.begin(), maParagraphStyleList.end(), &rTxtFmtColl );

    if ( aIter != maParagraphStyleList.end() )
    {
        maParagraphStyleList.erase( aIter );
    }
}


namespace numfunc
{
    /** class containing default bullet list configuration data

        @author OD
    */
    class SwDefBulletConfig : private utl::ConfigItem
    {
        public:
            static SwDefBulletConfig& getInstance();

            inline const String& GetFontname() const
            {
                return msFontname;
            }

            inline bool IsFontnameUserDefined() const
            {
                return mbUserDefinedFontname;
            }

            inline const Font& GetFont() const
            {
                return *mpFont;
            }

            inline short GetFontWeight() const
            {
                return static_cast<short>(meFontWeight);
            }

            inline short GetFontItalic() const
            {
                return static_cast<short>(meFontItalic);
            }
            inline sal_Unicode GetChar( sal_uInt8 p_nListLevel ) const
            {
                if (p_nListLevel >= MAXLEVEL)
                {
                    p_nListLevel = MAXLEVEL - 1;
                }

                return mnLevelChars[p_nListLevel];
            }

            SwDefBulletConfig();
            ~SwDefBulletConfig();
        private:

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
            virtual void Commit();

            // default bullet list configuration data
            String msFontname;
            bool mbUserDefinedFontname;
            FontWeight meFontWeight;
            FontItalic meFontItalic;
            sal_Unicode mnLevelChars[MAXLEVEL];

            // default bullet list font instance
            Font* mpFont;
    };

    namespace
    {
        class theSwDefBulletConfig
            : public rtl::Static<SwDefBulletConfig, theSwDefBulletConfig>{};
    }

    SwDefBulletConfig& SwDefBulletConfig::getInstance()
    {
        return theSwDefBulletConfig::get();
    }

    SwDefBulletConfig::SwDefBulletConfig()
        : ConfigItem( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Writer/Numbering/DefaultBulletList")) ),
          // default bullet font is now OpenSymbol
          msFontname( rtl::OUString("OpenSymbol") ),
          mbUserDefinedFontname( false ),
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

    SwDefBulletConfig::~SwDefBulletConfig()
    {
        delete mpFont;
    }

    void SwDefBulletConfig::SetToDefault()
    {
        msFontname = rtl::OUString("OpenSymbol");
        mbUserDefinedFontname = false;
        meFontWeight = WEIGHT_DONTKNOW;
        meFontItalic = ITALIC_NONE;

        mnLevelChars[0] = 0x2022;
        mnLevelChars[1] = 0x25e6;
        mnLevelChars[2] = 0x25aa;
        mnLevelChars[3] = 0x2022;
        mnLevelChars[4] = 0x25e6;
        mnLevelChars[5] = 0x25aa;
        mnLevelChars[6] = 0x2022;
        mnLevelChars[7] = 0x25e6;
        mnLevelChars[8] = 0x25aa;
        mnLevelChars[9] = 0x2022;
    }

    uno::Sequence<rtl::OUString> SwDefBulletConfig::GetPropNames() const
    {
        uno::Sequence<rtl::OUString> aPropNames(13);
        rtl::OUString* pNames = aPropNames.getArray();
        pNames[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletFont/FontFamilyname"));
        pNames[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletFont/FontWeight"));
        pNames[2] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletFont/FontItalic"));
        pNames[3] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletCharLvl1"));
        pNames[4] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletCharLvl2"));
        pNames[5] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletCharLvl3"));
        pNames[6] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletCharLvl4"));
        pNames[7] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletCharLvl5"));
        pNames[8] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletCharLvl6"));
        pNames[9] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletCharLvl7"));
        pNames[10] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletCharLvl8"));
        pNames[11] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletCharLvl9"));
        pNames[12] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletCharLvl10"));

        return aPropNames;
    }

    void SwDefBulletConfig::LoadConfig()
    {
        uno::Sequence<rtl::OUString> aPropNames = GetPropNames();
        uno::Sequence<uno::Any> aValues =
                                                    GetProperties( aPropNames );
        const uno::Any* pValues = aValues.getConstArray();
        OSL_ENSURE( aValues.getLength() == aPropNames.getLength(),
                "<SwDefBulletConfig::SwDefBulletConfig()> - GetProperties failed");
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
                            mbUserDefinedFontname = true;
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

    void SwDefBulletConfig::Commit()
    {
    }

    const String& GetDefBulletFontname()
    {
        return SwDefBulletConfig::getInstance().GetFontname();
    }

    bool IsDefBulletFontUserDefined()
    {
        return SwDefBulletConfig::getInstance().IsFontnameUserDefined();
    }

    const Font& GetDefBulletFont()
    {
        return SwDefBulletConfig::getInstance().GetFont();
    }

    sal_Unicode GetBulletChar( sal_uInt8 nLevel )
    {
        return SwDefBulletConfig::getInstance().GetChar( nLevel );
    }

    /** class containing configuration data about user interface behavior
        regarding lists and list items.
        configuration item about behavior of <TAB>/<SHIFT-TAB>-key at first
        position of first list item

        @author OD
    */
    class SwNumberingUIBehaviorConfig : private utl::ConfigItem
    {
        public:
            static SwNumberingUIBehaviorConfig& getInstance();

            inline sal_Bool ChangeIndentOnTabAtFirstPosOfFirstListItem() const
            {
                return mbChangeIndentOnTabAtFirstPosOfFirstListItem;
            }

            SwNumberingUIBehaviorConfig();

        private:

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
            virtual void Commit();

            // configuration data
            sal_Bool mbChangeIndentOnTabAtFirstPosOfFirstListItem;
    };

    namespace
    {
        class theSwNumberingUIBehaviorConfig : public rtl::Static<SwNumberingUIBehaviorConfig, theSwNumberingUIBehaviorConfig>{};
    }

    SwNumberingUIBehaviorConfig& SwNumberingUIBehaviorConfig::getInstance()
    {
        return theSwNumberingUIBehaviorConfig::get();
    }

    SwNumberingUIBehaviorConfig::SwNumberingUIBehaviorConfig()
        : ConfigItem( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Writer/Numbering/UserInterfaceBehavior")) ),
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
        pNames[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ChangeIndentOnTabAtFirstPosOfFirstListItem"));

        return aPropNames;
    }

    void SwNumberingUIBehaviorConfig::Commit() {}

    void SwNumberingUIBehaviorConfig::LoadConfig()
    {
        com::sun::star::uno::Sequence<rtl::OUString> aPropNames = GetPropNames();
        com::sun::star::uno::Sequence<com::sun::star::uno::Any> aValues =
                                                    GetProperties( aPropNames );
        const com::sun::star::uno::Any* pValues = aValues.getConstArray();
        OSL_ENSURE( aValues.getLength() == aPropNames.getLength(),
                "<SwNumberingUIBehaviorConfig::LoadConfig()> - GetProperties failed");
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
                            OSL_FAIL( "<SwNumberingUIBehaviorConfig::LoadConfig()> - unknown configuration property");
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

    sal_Bool ChangeIndentOnTabAtFirstPosOfFirstListItem()
    {
        return SwNumberingUIBehaviorConfig::getInstance().ChangeIndentOnTabAtFirstPosOfFirstListItem();
    }

    SvxNumberFormat::SvxNumPositionAndSpaceMode GetDefaultPositionAndSpaceMode()
    {
        SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode;
        SvtSaveOptions aSaveOptions;
        switch ( aSaveOptions.GetODFDefaultVersion() )
        {
            case SvtSaveOptions::ODFVER_010:
            case SvtSaveOptions::ODFVER_011:
            {
                ePosAndSpaceMode = SvxNumberFormat::LABEL_WIDTH_AND_POSITION;
            }
            break;
            default: // ODFVER_UNKNOWN or ODFVER_012
            {
                ePosAndSpaceMode = SvxNumberFormat::LABEL_ALIGNMENT;
            }
        }

        return ePosAndSpaceMode;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
