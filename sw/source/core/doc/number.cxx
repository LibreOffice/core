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
#include <svl/grabbagitem.hxx>
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

#include <list.hxx>

#include <algorithm>
#include <unordered_map>
#include <libxml/xmlwriter.h>

#include <unotools/saveopt.hxx>

#include <IDocumentListsAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentState.hxx>

using namespace ::com::sun::star;

sal_uInt16 SwNumRule::mnRefCount = 0;
SwNumFmt* SwNumRule::maBaseFmts[ RULE_END ][ MAXLEVEL ] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

SwNumFmt* SwNumRule::maLabelAlignmentBaseFmts[ RULE_END ][ MAXLEVEL ] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

const sal_uInt16 SwNumRule::maDefNumIndents[ MAXLEVEL ] = {
//inch:   0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
        1440/4, 1440/2, 1440*3/4, 1440, 1440*5/4, 1440*3/2, 1440*7/4, 1440*2,
        1440*9/4, 1440*5/2
};

OUString SwNumRule::GetOutlineRuleName()
{
    return OUString("Outline");
}

const SwNumFmt& SwNumRule::Get( sal_uInt16 i ) const
{
    OSL_ASSERT( i < MAXLEVEL && meRuleType < RULE_END );
    return maFmts[ i ]
           ? *maFmts[ i ]
           : ( meDefaultNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION
               ? *maBaseFmts[ meRuleType ][ i ]
               : *maLabelAlignmentBaseFmts[ meRuleType ][ i ] );
}

const SwNumFmt* SwNumRule::GetNumFmt( sal_uInt16 i ) const
{
    const SwNumFmt * pResult = NULL;

    OSL_ASSERT( i < MAXLEVEL && meRuleType < RULE_END );
    if ( i < MAXLEVEL && meRuleType < RULE_END)
    {
        pResult = maFmts[ i ];
    }

    return pResult;
}

// #i91400#
void SwNumRule::SetName( const OUString & rName,
                         IDocumentListsAccess& rDocListAccess)
{
    if ( msName != rName )
    {
        if (mpNumRuleMap)
        {
            mpNumRuleMap->erase(msName);
            (*mpNumRuleMap)[rName] = this;

            if ( !GetDefaultListId().isEmpty() )
            {
                rDocListAccess.trackChangeOfListStyleName( msName, rName );
            }
        }

        msName = rName;
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

void SwNumRule::SetNumRuleMap(std::unordered_map<OUString, SwNumRule *, OUStringHash> *
                              pNumRuleMap)
{
    mpNumRuleMap = pNumRuleMap;
}

sal_uInt16 SwNumRule::GetNumIndent( sal_uInt8 nLvl )
{
    OSL_ENSURE( MAXLEVEL > nLvl, "NumLevel is out of range" );
    return maDefNumIndents[ nLvl ];
}

sal_uInt16 SwNumRule::GetBullIndent( sal_uInt8 nLvl )
{
    OSL_ENSURE( MAXLEVEL > nLvl, "NumLevel is out of range" );
    return maDefNumIndents[ nLvl ];
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
    ,cGrfBulletCP(USHRT_MAX)//For i120928,record the cp info of graphic within bullet
{
}

SwNumFmt::SwNumFmt( const SwNumFmt& rFmt) :
    SvxNumberFormat(rFmt),
    SwClient( rFmt.GetRegisteredInNonConst() ),
    pVertOrient(new SwFmtVertOrient( 0, rFmt.GetVertOrient()))
    ,cGrfBulletCP(rFmt.cGrfBulletCP)//For i120928,record the cp info of graphic within bullet
{
    sal_Int16 eMyVertOrient = rFmt.GetVertOrient();
    SetGraphicBrush( rFmt.GetBrush(), &rFmt.GetGraphicSize(),
                                                &eMyVertOrient);
}

SwNumFmt::SwNumFmt(const SvxNumberFormat& rNumFmt, SwDoc* pDoc)
    : SvxNumberFormat(rNumFmt)
    , pVertOrient(new SwFmtVertOrient( 0, rNumFmt.GetVertOrient()))
    , cGrfBulletCP(USHRT_MAX)
{
    sal_Int16 eMyVertOrient = rNumFmt.GetVertOrient();
    SetGraphicBrush( rNumFmt.GetBrush(), &rNumFmt.GetGraphicSize(),
                                                &eMyVertOrient);
    const OUString rCharStyleName = rNumFmt.SvxNumberFormat::GetCharFmtName();
    if( !rCharStyleName.isEmpty() )
    {
        SwCharFmt* pCFmt = pDoc->FindCharFmtByName( rCharStyleName );
        if( !pCFmt )
        {
            sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( rCharStyleName,
                                            nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            pCFmt = nId != USHRT_MAX
                        ? pDoc->getIDocumentStylePoolAccess().GetCharFmtFromPool( nId )
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
        UpdateNumNodes( GetCharFmt()->GetDoc() );
}

// #i22362#
bool SwNumFmt::IsEnumeration() const
{
    // #i30655# native numbering did not work any longer
    // using this code. Therefore HBRINKM and I agreed upon defining
    // IsEnumeration() as !IsItemize()
    return !IsItemize();
}

bool SwNumFmt::IsItemize() const
{
    bool bResult;

    switch(GetNumberingType())
    {
    case SVX_NUM_CHAR_SPECIAL:
    case SVX_NUM_BITMAP:
        bResult = true;

        break;

    default:
        bResult = false;
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
    //For i120928,record the cp info of graphic within bullet
    cGrfBulletCP = rNumFmt.cGrfBulletCP;
    return *this;
}

bool SwNumFmt::operator==( const SwNumFmt& rNumFmt) const
{
    bool bRet = SvxNumberFormat::operator==(rNumFmt) &&
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
        UpdateNumNodes( const_cast<SwDoc*>(pFmt->GetDoc()) );
    else
        CheckRegistration( pOld, pNew );
}

void SwNumFmt::SetCharFmtName(const OUString& rSet)
{
    SvxNumberFormat::SetCharFmtName(rSet);
}

OUString SwNumFmt::GetCharFmtName() const
{
    if(static_cast<const SwCharFmt*>(GetRegisteredIn()))
        return static_cast<const SwCharFmt*>(GetRegisteredIn())->GetName();

    return OUString();
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
    bool bDocIsModified = pDoc->getIDocumentState().IsModified();
    bool bFnd = false;
    for( SwNumRuleTbl::size_type n = pDoc->GetNumRuleTbl().size(); !bFnd && n; )
    {
        const SwNumRule* pRule = pDoc->GetNumRuleTbl()[ --n ];
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
        pDoc->getIDocumentState().ResetModified();
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

SwNumRule::SwNumRule( const OUString& rNm,
                      const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode,
                      SwNumRuleType eType,
                      bool bAutoFlg )
  : maTxtNodeList(),
    maParagraphStyleList(),
    mpNumRuleMap(0),
    msName( rNm ),
    meRuleType( eType ),
    mnPoolFmtId( USHRT_MAX ),
    mnPoolHelpId( USHRT_MAX ),
    mnPoolHlpFileId( UCHAR_MAX ),
    mbAutoRuleFlag( bAutoFlg ),
    mbInvalidRuleFlag( true ),
    mbContinusNum( false ),
    mbAbsSpaces( false ),
    mbHidden( false ),
    mbCountPhantoms( true ),
    meDefaultNumberFormatPositionAndSpaceMode( eDefaultNumberFormatPositionAndSpaceMode ),
    msDefaultListId()
{
    if( !mnRefCount++ )          // for the first time, initialize
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
            pFmt->SetAbsLSpace( lNumIndent + SwNumRule::GetNumIndent( n ) );
            pFmt->SetFirstLineOffset( lNumFirstLineOffset );
            pFmt->SetSuffix( "." );
            pFmt->SetBulletChar( numfunc::GetBulletChar(n));
            SwNumRule::maBaseFmts[ NUM_RULE ][ n ] = pFmt;
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
            pFmt->SetSuffix( "." );
            pFmt->SetBulletChar( numfunc::GetBulletChar(n));
            SwNumRule::maLabelAlignmentBaseFmts[ NUM_RULE ][ n ] = pFmt;
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
            SwNumRule::maBaseFmts[ OUTLINE_RULE ][ n ] = pFmt;
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
            SwNumRule::maLabelAlignmentBaseFmts[ OUTLINE_RULE ][ n ] = pFmt;
        }
    }
    memset( maFmts, 0, sizeof( maFmts ));
    OSL_ENSURE( !msName.isEmpty(), "NumRule without a name!" );
}

SwNumRule::SwNumRule( const SwNumRule& rNumRule )
    : maTxtNodeList(),
      maParagraphStyleList(),
      mpNumRuleMap(0),
      msName( rNumRule.msName ),
      meRuleType( rNumRule.meRuleType ),
      mnPoolFmtId( rNumRule.GetPoolFmtId() ),
      mnPoolHelpId( rNumRule.GetPoolHelpId() ),
      mnPoolHlpFileId( rNumRule.GetPoolHlpFileId() ),
      mbAutoRuleFlag( rNumRule.mbAutoRuleFlag ),
      mbInvalidRuleFlag( true ),
      mbContinusNum( rNumRule.mbContinusNum ),
      mbAbsSpaces( rNumRule.mbAbsSpaces ),
      mbHidden( rNumRule.mbHidden ),
      mbCountPhantoms( true ),
      meDefaultNumberFormatPositionAndSpaceMode( rNumRule.meDefaultNumberFormatPositionAndSpaceMode ),
      msDefaultListId( rNumRule.msDefaultListId )
{
    ++mnRefCount;
    memset( maFmts, 0, sizeof( maFmts ));
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
        if( rNumRule.maFmts[ n ] )
            Set( n, *rNumRule.maFmts[ n ] );
}

SwNumRule::~SwNumRule()
{
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
        delete maFmts[ n ];

    if (mpNumRuleMap)
    {
        mpNumRuleMap->erase(GetName());
    }

    if( !--mnRefCount )          // the last one closes the door (?)
    {
            // Numbering:
            SwNumFmt** ppFmts = &SwNumRule::maBaseFmts[0][0];
            int n;

            for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
                delete *ppFmts, *ppFmts = 0;

            // Outline:
            for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
                delete *ppFmts, *ppFmts = 0;

            ppFmts = &SwNumRule::maLabelAlignmentBaseFmts[0][0];
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
    for( sal_uInt8 n = 0; n < MAXLEVEL; ++n )
    {
        SwCharFmt* pFmt;
        if( maFmts[ n ] && 0 != ( pFmt = maFmts[ n ]->GetCharFmt() ) &&
            pFmt->GetDoc() != pDoc )
        {
            // copy
            SwNumFmt* pNew = new SwNumFmt( *maFmts[ n ] );
            pNew->SetCharFmt( pDoc->CopyCharFmt( *pFmt ) );
            delete maFmts[ n ];
            maFmts[ n ] = pNew;
        }
    }
}

SwNumRule& SwNumRule::operator=( const SwNumRule& rNumRule )
{
    if( this != &rNumRule )
    {
        for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
            Set( n, rNumRule.maFmts[ n ] );

        meRuleType = rNumRule.meRuleType;
        msName = rNumRule.msName;
        mbAutoRuleFlag = rNumRule.mbAutoRuleFlag;
        mbInvalidRuleFlag = true;
        mbContinusNum = rNumRule.mbContinusNum;
        mbAbsSpaces = rNumRule.mbAbsSpaces;
        mbHidden = rNumRule.mbHidden;
        mnPoolFmtId = rNumRule.GetPoolFmtId();
        mnPoolHelpId = rNumRule.GetPoolHelpId();
        mnPoolHlpFileId = rNumRule.GetPoolHlpFileId();
    }
    return *this;
}

bool SwNumRule::operator==( const SwNumRule& rRule ) const
{
    bool bRet = meRuleType == rRule.meRuleType &&
                msName == rRule.msName &&
                mbAutoRuleFlag == rRule.mbAutoRuleFlag &&
                mbContinusNum == rRule.mbContinusNum &&
                mbAbsSpaces == rRule.mbAbsSpaces &&
                mnPoolFmtId == rRule.GetPoolFmtId() &&
                mnPoolHelpId == rRule.GetPoolHelpId() &&
                mnPoolHlpFileId == rRule.GetPoolHlpFileId();
    if( bRet )
    {
        for( sal_uInt8 n = 0; n < MAXLEVEL; ++n )
            if( !( rRule.Get( n ) == Get( n ) ))
            {
                bRet = false;
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
        if( !maFmts[ i ] || !(rNumFmt == Get( i )) )
        {
            delete maFmts[ i ];
            maFmts[ i ] = new SwNumFmt( rNumFmt );
            mbInvalidRuleFlag = true;
        }
    }
}

void SwNumRule::Set( sal_uInt16 i, const SwNumFmt* pNumFmt )
{
    OSL_ENSURE( i < MAXLEVEL, "Serious defect, please inform OD" );
    if( i >= MAXLEVEL )
        return;
    SwNumFmt* pOld = maFmts[ i ];
    if( !pOld )
    {
        if( pNumFmt )
        {
            maFmts[ i ] = new SwNumFmt( *pNumFmt );
            mbInvalidRuleFlag = true;
        }
    }
    else if( !pNumFmt )
        delete pOld, maFmts[ i ] = 0, mbInvalidRuleFlag = true;
    else if( *pOld != *pNumFmt )
        *pOld = *pNumFmt, mbInvalidRuleFlag = true;
}

OUString SwNumRule::MakeNumString( const SwNodeNum& rNum, bool bInclStrings,
                                bool bOnlyArabic ) const
{
    if (rNum.IsCounted())
        return MakeNumString(rNum.GetNumberVector(),
                             bInclStrings, bOnlyArabic, MAXLEVEL);

    return OUString();
}

OUString SwNumRule::MakeNumString( const SwNumberTree::tNumberVector & rNumVector,
                                 const bool bInclStrings,
                                 const bool bOnlyArabic,
                                 const unsigned int _nRestrictToThisLevel,
                                 SwNumRule::Extremities* pExtremities ) const
{
    OUString aStr;

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
                    //                    aStr += ".";
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
                    aStr += "0";        // all 0 level are a 0
                if( i != nLevel && !aStr.isEmpty() )
                    aStr += ".";
            }

            // The type doesn't have any number, so don't append
            // the post-/prefix string
            if( bInclStrings && !bOnlyArabic &&
                SVX_NUM_CHAR_SPECIAL != rMyNFmt.GetNumberingType() &&
                SVX_NUM_BITMAP != rMyNFmt.GetNumberingType() )
            {
                const OUString sPrefix = rMyNFmt.GetPrefix();
                const OUString sSuffix = rMyNFmt.GetSuffix();

                aStr = sPrefix + aStr + sSuffix;
                if ( pExtremities )
                {
                    pExtremities->nPrefixChars = sPrefix.getLength();
                    pExtremities->nSuffixChars = sSuffix.getLength();
                }
            }
        }
    }

    return aStr;
}

OUString SwNumRule::MakeRefNumString( const SwNodeNum& rNodeNum,
                                    const bool bInclSuperiorNumLabels,
                                    const sal_uInt8 nRestrictInclToThisLevel ) const
{
    OUString aRefNumStr;

    if ( rNodeNum.GetLevelInListTree() >= 0 )
    {
        bool bOldHadPrefix = true;

        const SwNodeNum* pWorkingNodeNum( &rNodeNum );
        do
        {
            bool bMakeNumStringForPhantom( false );
            if ( pWorkingNodeNum->IsPhantom() )
            {
                int nListLevel = pWorkingNodeNum->GetLevelInListTree();

                if (nListLevel < 0)
                    nListLevel = 0;

                if (nListLevel >= MAXLEVEL)
                    nListLevel = MAXLEVEL - 1;

                SwNumFmt aFmt( Get( static_cast<sal_uInt16>(nListLevel) ) );
                bMakeNumStringForPhantom = aFmt.IsEnumeration() &&
                                           SVX_NUM_NUMBER_NONE != aFmt.GetNumberingType();

            }
            if ( bMakeNumStringForPhantom ||
                 ( !pWorkingNodeNum->IsPhantom() &&
                   pWorkingNodeNum->GetTxtNode() &&
                   pWorkingNodeNum->GetTxtNode()->HasNumber() ) )
            {
                Extremities aExtremities;
                OUString aPrevStr = MakeNumString( pWorkingNodeNum->GetNumberVector(),
                                                 true, false, MAXLEVEL,
                                                 &aExtremities);
                sal_Int32 nStrip = 0;
                while ( nStrip < aExtremities.nPrefixChars )
                {
                    const sal_Unicode c = aPrevStr[nStrip];
                    if ( c!='\t' && c!=' ')
                        break;
                    ++nStrip;
                }

                if (nStrip)
                {
                    aPrevStr = aPrevStr.copy( nStrip );
                    aExtremities.nPrefixChars -= nStrip;
                }

                if (bOldHadPrefix &&
                     aExtremities.nSuffixChars &&
                     !aExtremities.nPrefixChars
                   )
                {
                    aPrevStr = aPrevStr.copy(0,
                        aPrevStr.getLength() - aExtremities.nSuffixChars);
                }

                bOldHadPrefix = ( aExtremities.nPrefixChars >  0);

                aRefNumStr = aPrevStr + aRefNumStr;
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
        } while ( pWorkingNodeNum &&
                  pWorkingNodeNum->GetLevelInListTree() >= 0 &&
                  static_cast<sal_uInt8>(pWorkingNodeNum->GetLevelInListTree()) >= nRestrictInclToThisLevel );
    }

    return aRefNumStr;
}

/** Copy method of SwNumRule

    A kind of copy constructor, so that the num formats are attached to the
    right CharFormats of a Document.
    Copies the NumFormats and returns itself. */
SwNumRule& SwNumRule::CopyNumRule( SwDoc* pDoc, const SwNumRule& rNumRule )
{
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        Set( n, rNumRule.maFmts[ n ] );
        if( maFmts[ n ] && maFmts[ n ]->GetCharFmt() &&
            !pDoc->GetCharFmts()->Contains( maFmts[n]->GetCharFmt() ))
            // If we copy across different Documents, then copy the
            // corresponding CharFormat into the new Document.
            maFmts[n]->SetCharFmt( pDoc->CopyCharFmt( *maFmts[n]->
                                        GetCharFmt() ) );
    }
    meRuleType = rNumRule.meRuleType;
    msName = rNumRule.msName;
    mbAutoRuleFlag = rNumRule.mbAutoRuleFlag;
    mnPoolFmtId = rNumRule.GetPoolFmtId();
    mnPoolHelpId = rNumRule.GetPoolHelpId();
    mnPoolHlpFileId = rNumRule.GetPoolHlpFileId();
    mbInvalidRuleFlag = true;
    return *this;
}

void SwNumRule::SetSvxRule(const SvxNumRule& rNumRule, SwDoc* pDoc)
{
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        const SvxNumberFormat* pSvxFmt = rNumRule.Get(n);
        delete maFmts[n];
        maFmts[n] = pSvxFmt ? new SwNumFmt(*pSvxFmt, pDoc) : 0;
    }

    mbInvalidRuleFlag = true;
    mbContinusNum = rNumRule.IsContinuousNumbering();
}

SvxNumRule SwNumRule::MakeSvxNumRule() const
{
    SvxNumRule aRule(SvxNumRuleFlags::CONTINUOUS | SvxNumRuleFlags::CHAR_TEXT_DISTANCE | SvxNumRuleFlags::CHAR_STYLE |
                     SvxNumRuleFlags::ENABLE_LINKED_BMP | SvxNumRuleFlags::ENABLE_EMBEDDED_BMP,
                     MAXLEVEL, mbContinusNum,
                     meRuleType == NUM_RULE ? SvxNumRuleType::NUMBERING : SvxNumRuleType::OUTLINE_NUMBERING );
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        SwNumFmt aNumFmt = Get(n);
        if(aNumFmt.GetCharFmt())
            aNumFmt.SetCharFmtName(aNumFmt.GetCharFmt()->GetName());
        aRule.SetLevel(n, aNumFmt, maFmts[n] != 0);
    }
    return aRule;
}

void SwNumRule::SetInvalidRule(bool bFlag)
{
    if (bFlag)
    {
        std::set< SwList* > aLists;
        tTxtNodeList::iterator aIter;
        for ( aIter = maTxtNodeList.begin(); aIter != maTxtNodeList.end(); ++aIter )
        {
            const SwTxtNode* pTxtNode = *aIter;
            // #i111681# - applying patch from cmc
            SwList* pList = pTxtNode->GetDoc()->getIDocumentListsAccess().getListByName( pTxtNode->GetListId() );
            OSL_ENSURE( pList, "<SwNumRule::SetInvalidRule(..)> - list at which the text node is registered at does not exist. This is a serious issue --> please inform OD.");
            if ( pList )
            {
                aLists.insert( pList );
            }
        }
        std::for_each( aLists.begin(), aLists.end(),
                       std::mem_fun( &SwList::InvalidateListTree ) );
    }

    mbInvalidRuleFlag = bFlag;
}

/// change indent of all list levels by given difference
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

    SetInvalidRule( true );
}

/// set indent of certain list level to given value
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

    SetInvalidRule( true );
}

/// set indent of first list level to given value and change other list level's
/// indents accordingly
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
        aLists.insert( pTxtNode->GetDoc()->getIDocumentListsAccess().getListByName( pTxtNode->GetListId() ) );
    }
    std::for_each( aLists.begin(), aLists.end(),
                   std::mem_fun( &SwList::ValidateListTree ) );

    SetInvalidRule(false);
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

void SwNumRule::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swNumRule"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("msName"), BAD_CAST(msName.toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("mnPoolFmtId"), BAD_CAST(OString::number(mnPoolFmtId).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("mbAutoRuleFlag"), BAD_CAST(OString::boolean(mbAutoRuleFlag).getStr()));
    xmlTextWriterEndElement(pWriter);
}

void SwNumRule::GetGrabBagItem(uno::Any& rVal) const
{
    if (mpGrabBagItem.get())
        mpGrabBagItem->QueryValue(rVal);
    else
    {
        uno::Sequence<beans::PropertyValue> aValue(0);
        rVal = uno::makeAny(aValue);
    }
}

void SwNumRule::SetGrabBagItem(const uno::Any& rVal)
{
    if (!mpGrabBagItem.get())
        mpGrabBagItem.reset(new SfxGrabBagItem);

    mpGrabBagItem->PutValue(rVal);
}

namespace numfunc
{
    /** class containing default bullet list configuration data */
    class SwDefBulletConfig : private utl::ConfigItem
    {
        public:
            static SwDefBulletConfig& getInstance();

            inline OUString GetFontname() const
            {
                return msFontname;
            }

            inline bool IsFontnameUserDefined() const
            {
                return mbUserDefinedFontname;
            }

            inline const vcl::Font& GetFont() const
            {
                return *mpFont;
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
            virtual ~SwDefBulletConfig();

        private:
            /** sets internal default bullet configuration data to default values */
            void SetToDefault();

            /** returns sequence of default bullet configuration property names */
            static uno::Sequence<OUString> GetPropNames();

            /** loads default bullet configuration properties and applies
                values to internal data */
            void LoadConfig();

            /** initialize font instance for default bullet list */
            void InitFont();

            /** catches notification about changed default bullet configuration data */
            virtual void Notify( const uno::Sequence<OUString>& aPropertyNames ) SAL_OVERRIDE;
            virtual void ImplCommit() SAL_OVERRIDE;

            // default bullet list configuration data
            OUString msFontname;
            bool mbUserDefinedFontname;
            FontWeight meFontWeight;
            FontItalic meFontItalic;
            sal_Unicode mnLevelChars[MAXLEVEL];

            // default bullet list font instance
            vcl::Font* mpFont;
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
        : ConfigItem( OUString("Office.Writer/Numbering/DefaultBulletList") ),
          // default bullet font is now OpenSymbol
          msFontname( OUString("OpenSymbol") ),
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
        msFontname = "OpenSymbol";
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

    uno::Sequence<OUString> SwDefBulletConfig::GetPropNames()
    {
        uno::Sequence<OUString> aPropNames(13);
        OUString* pNames = aPropNames.getArray();
        pNames[0] = "BulletFont/FontFamilyname";
        pNames[1] = "BulletFont/FontWeight";
        pNames[2] = "BulletFont/FontItalic";
        pNames[3] = "BulletCharLvl1";
        pNames[4] = "BulletCharLvl2";
        pNames[5] = "BulletCharLvl3";
        pNames[6] = "BulletCharLvl4";
        pNames[7] = "BulletCharLvl5";
        pNames[8] = "BulletCharLvl6";
        pNames[9] = "BulletCharLvl7";
        pNames[10] = "BulletCharLvl8";
        pNames[11] = "BulletCharLvl9";
        pNames[12] = "BulletCharLvl10";

        return aPropNames;
    }

    void SwDefBulletConfig::LoadConfig()
    {
        uno::Sequence<OUString> aPropNames = GetPropNames();
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
                            OUString aStr;
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

        mpFont = new vcl::Font( msFontname, OUString(), Size( 0, 14 ) );
        mpFont->SetWeight( meFontWeight );
        mpFont->SetItalic( meFontItalic );
        mpFont->SetCharSet( RTL_TEXTENCODING_SYMBOL );
    }

    void SwDefBulletConfig::Notify( const uno::Sequence<OUString>& )
    {
        SetToDefault();
        LoadConfig();
        InitFont();
    }

    void SwDefBulletConfig::ImplCommit()
    {
    }

    OUString GetDefBulletFontname()
    {
        return SwDefBulletConfig::getInstance().GetFontname();
    }

    bool IsDefBulletFontUserDefined()
    {
        return SwDefBulletConfig::getInstance().IsFontnameUserDefined();
    }

    const vcl::Font& GetDefBulletFont()
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
    */
    class SwNumberingUIBehaviorConfig : private utl::ConfigItem
    {
        public:
            static SwNumberingUIBehaviorConfig& getInstance();

            inline bool ChangeIndentOnTabAtFirstPosOfFirstListItem() const
            {
                return mbChangeIndentOnTabAtFirstPosOfFirstListItem;
            }

            SwNumberingUIBehaviorConfig();

        private:

            /** sets internal configuration data to default values */
            void SetToDefault();

            /** returns sequence of configuration property names */
            static com::sun::star::uno::Sequence<OUString> GetPropNames();

            /** loads configuration properties and applies values to internal data */
            void LoadConfig();

            /** catches notification about changed configuration data */
            virtual void Notify( const com::sun::star::uno::Sequence<OUString>& aPropertyNames ) SAL_OVERRIDE;
            virtual void ImplCommit() SAL_OVERRIDE;

            // configuration data
            bool mbChangeIndentOnTabAtFirstPosOfFirstListItem;
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
        : ConfigItem( OUString("Office.Writer/Numbering/UserInterfaceBehavior") ),
          mbChangeIndentOnTabAtFirstPosOfFirstListItem( true )
    {
        SetToDefault();
        LoadConfig();

        // enable notification for changes on configuration change
        EnableNotification( GetPropNames() );
    }

    void SwNumberingUIBehaviorConfig::SetToDefault()
    {
        mbChangeIndentOnTabAtFirstPosOfFirstListItem = true;
    }

    com::sun::star::uno::Sequence<OUString> SwNumberingUIBehaviorConfig::GetPropNames()
    {
        com::sun::star::uno::Sequence<OUString> aPropNames(1);
        OUString* pNames = aPropNames.getArray();
        pNames[0] = "ChangeIndentOnTabAtFirstPosOfFirstListItem";

        return aPropNames;
    }

    void SwNumberingUIBehaviorConfig::ImplCommit() {}

    void SwNumberingUIBehaviorConfig::LoadConfig()
    {
        com::sun::star::uno::Sequence<OUString> aPropNames = GetPropNames();
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

    void SwNumberingUIBehaviorConfig::Notify( const com::sun::star::uno::Sequence<OUString>& aPropertyNames )
    {
        (void) aPropertyNames;
        SetToDefault();
        LoadConfig();
    }

    bool ChangeIndentOnTabAtFirstPosOfFirstListItem()
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

void SwNumRuleTbl::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swNumRuleTbl"));
    for (SwNumRule* pNumRule : *this)
        pNumRule->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
