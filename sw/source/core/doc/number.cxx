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

#include <memory>
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
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>

#include <numrule.hxx>
#include <SwNodeNum.hxx>

#include <list.hxx>

#include <algorithm>
#include <unordered_map>
#include <libxml/xmlwriter.h>

#include <i18nlangtag/languagetag.hxx>
#include <unotools/saveopt.hxx>
#include <osl/diagnose.h>

#include <IDocumentListsAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentState.hxx>

using namespace ::com::sun::star;

sal_uInt16 SwNumRule::mnRefCount = 0;
SwNumFormat* SwNumRule::maBaseFormats[ RULE_END ][ MAXLEVEL ] = {
    {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr } };

SwNumFormat* SwNumRule::maLabelAlignmentBaseFormats[ RULE_END ][ MAXLEVEL ] = {
    {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr } };

const sal_uInt16 SwNumRule::maDefNumIndents[ MAXLEVEL ] = {
//inch:   0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
        1440/4, 1440/2, 1440*3/4, 1440, 1440*5/4, 1440*3/2, 1440*7/4, 1440*2,
        1440*9/4, 1440*5/2
};

OUString SwNumRule::GetOutlineRuleName()
{
    return OUString("Outline");
}

const SwNumFormat& SwNumRule::Get( sal_uInt16 i ) const
{
    assert( i < MAXLEVEL && meRuleType < RULE_END );
    return maFormats[ i ]
           ? *maFormats[ i ]
           : ( meDefaultNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION
               ? *maBaseFormats[ meRuleType ][ i ]
               : *maLabelAlignmentBaseFormats[ meRuleType ][ i ] );
}

const SwNumFormat* SwNumRule::GetNumFormat( sal_uInt16 i ) const
{
    const SwNumFormat * pResult = nullptr;

    assert( i < MAXLEVEL && meRuleType < RULE_END );
    if ( i < MAXLEVEL && meRuleType < RULE_END)
    {
        pResult = maFormats[ i ].get();
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

void SwNumRule::GetTextNodeList( SwNumRule::tTextNodeList& rTextNodeList ) const
{
    rTextNodeList = maTextNodeList;
}

SwNumRule::tTextNodeList::size_type SwNumRule::GetTextNodeListSize() const
{
    return maTextNodeList.size();
}

void SwNumRule::AddTextNode( SwTextNode& rTextNode )
{
    tTextNodeList::iterator aIter =
        std::find( maTextNodeList.begin(), maTextNodeList.end(), &rTextNode );

    if ( aIter == maTextNodeList.end() )
    {
        maTextNodeList.push_back( &rTextNode );
    }
}

void SwNumRule::RemoveTextNode( SwTextNode& rTextNode )
{
    tTextNodeList::iterator aIter =
        std::find( maTextNodeList.begin(), maTextNodeList.end(), &rTextNode );

    if ( aIter != maTextNodeList.end() )
    {
        maTextNodeList.erase( aIter );
    }
}

void SwNumRule::SetNumRuleMap(std::unordered_map<OUString, SwNumRule *> *
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

static void lcl_SetRuleChgd( SwTextNode& rNd, sal_uInt8 nLevel )
{
    if( rNd.GetActualListLevel() == nLevel )
        rNd.NumRuleChgd();
}

SwNumFormat::SwNumFormat() :
    SvxNumberFormat(SVX_NUM_ARABIC),
    SwClient( nullptr ),
    m_pVertOrient(new SwFormatVertOrient( 0, text::VertOrientation::NONE))
    ,m_cGrfBulletCP(USHRT_MAX)//For i120928,record the cp info of graphic within bullet
{
}

SwNumFormat::SwNumFormat( const SwNumFormat& rFormat) :
    SvxNumberFormat(rFormat),
    SwClient( rFormat.GetRegisteredInNonConst() ),
    m_pVertOrient(new SwFormatVertOrient( 0, rFormat.GetVertOrient()))
    ,m_cGrfBulletCP(rFormat.m_cGrfBulletCP)//For i120928,record the cp info of graphic within bullet
{
    sal_Int16 eMyVertOrient = rFormat.GetVertOrient();
    SetGraphicBrush( rFormat.GetBrush(), &rFormat.GetGraphicSize(),
                                                &eMyVertOrient);
}

SwNumFormat::SwNumFormat(const SvxNumberFormat& rNumFormat, SwDoc* pDoc)
    : SvxNumberFormat(rNumFormat)
    , m_pVertOrient(new SwFormatVertOrient( 0, rNumFormat.GetVertOrient()))
    , m_cGrfBulletCP(USHRT_MAX)
{
    sal_Int16 eMyVertOrient = rNumFormat.GetVertOrient();
    SetGraphicBrush( rNumFormat.GetBrush(), &rNumFormat.GetGraphicSize(),
                                                &eMyVertOrient);
    const OUString rCharStyleName = rNumFormat.SvxNumberFormat::GetCharFormatName();
    if( !rCharStyleName.isEmpty() )
    {
        SwCharFormat* pCFormat = pDoc->FindCharFormatByName( rCharStyleName );
        if( !pCFormat )
        {
            sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( rCharStyleName,
                                            SwGetPoolIdFromName::ChrFmt );
            pCFormat = nId != USHRT_MAX
                        ? pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( nId )
                        : pDoc->MakeCharFormat( rCharStyleName, nullptr );
        }
        pCFormat->Add( this );
    }
    else
        EndListeningAll();
}

SwNumFormat::~SwNumFormat()
{
}

// #i22362#
bool SwNumFormat::IsEnumeration() const
{
    // #i30655# native numbering did not work any longer
    // using this code. Therefore HBRINKM and I agreed upon defining
    // IsEnumeration() as !IsItemize()
    return !IsItemize();
}

bool SwNumFormat::IsItemize() const
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

SwNumFormat& SwNumFormat::operator=( const SwNumFormat& rNumFormat)
{
    SvxNumberFormat::operator=(rNumFormat);
    StartListeningToSameModifyAs(rNumFormat);
    //For i120928,record the cp info of graphic within bullet
    m_cGrfBulletCP = rNumFormat.m_cGrfBulletCP;
    return *this;
}

bool SwNumFormat::operator==( const SwNumFormat& rNumFormat) const
{
    bool bRet = SvxNumberFormat::operator==(rNumFormat) &&
        GetRegisteredIn() == rNumFormat.GetRegisteredIn();
    return bRet;
}

void SwNumFormat::SetCharFormat( SwCharFormat* pChFormat)
{
    if( pChFormat )
        pChFormat->Add( this );
    else
        EndListeningAll();
}

void SwNumFormat::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    // Look for the NumRules object in the Doc where this NumFormat is set.
    // The format does not need to exist!
    const SwCharFormat* pFormat = nullptr;
    sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
    case RES_ATTRSET_CHG:
    case RES_FMT_CHG:
        pFormat = GetCharFormat();
        break;
    }

    if( pFormat && !pFormat->GetDoc()->IsInDtor() )
        UpdateNumNodes( const_cast<SwDoc*>(pFormat->GetDoc()) );
    else
        CheckRegistration( pOld );
}

OUString SwNumFormat::GetCharFormatName() const
{
    if(static_cast<const SwCharFormat*>(GetRegisteredIn()))
        return static_cast<const SwCharFormat*>(GetRegisteredIn())->GetName();

    return OUString();
}

void    SwNumFormat::SetGraphicBrush( const SvxBrushItem* pBrushItem, const Size* pSize,
    const sal_Int16* pOrient)
{
    if(pOrient)
        m_pVertOrient->SetVertOrient( *pOrient );
    SvxNumberFormat::SetGraphicBrush( pBrushItem, pSize, pOrient);
}

void SwNumFormat::UpdateNumNodes( SwDoc* pDoc )
{
    bool bDocIsModified = pDoc->getIDocumentState().IsModified();
    bool bFnd = false;
    for( SwNumRuleTable::size_type n = pDoc->GetNumRuleTable().size(); !bFnd && n; )
    {
        const SwNumRule* pRule = pDoc->GetNumRuleTable()[ --n ];
        for( sal_uInt8 i = 0; i < MAXLEVEL; ++i )
            if( pRule->GetNumFormat( i ) == this )
            {
                SwNumRule::tTextNodeList aTextNodeList;
                pRule->GetTextNodeList( aTextNodeList );
                for ( auto& rpTextNode : aTextNodeList )
                {
                    lcl_SetRuleChgd( *rpTextNode, i );
                }
                bFnd = true;
                break;
            }
    }

    if( bFnd && !bDocIsModified )
        pDoc->getIDocumentState().ResetModified();
}

const SwFormatVertOrient*      SwNumFormat::GetGraphicOrientation() const
{
    sal_Int16  eOrient = SvxNumberFormat::GetVertOrient();
    if(text::VertOrientation::NONE == eOrient)
        return nullptr;
    else
    {
        m_pVertOrient->SetVertOrient(eOrient);
        return m_pVertOrient.get();
    }
}

SwNumRule::SwNumRule( const OUString& rNm,
                      const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode,
                      SwNumRuleType eType )
  : maTextNodeList(),
    maParagraphStyleList(),
    mpNumRuleMap(nullptr),
    msName( rNm ),
    meRuleType( eType ),
    mnPoolFormatId( USHRT_MAX ),
    mnPoolHelpId( USHRT_MAX ),
    mnPoolHlpFileId( UCHAR_MAX ),
    mbAutoRuleFlag( true ),
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
        SwNumFormat* pFormat;
        sal_uInt8 n;

        // numbering:
        // position-and-space mode LABEL_WIDTH_AND_POSITION:
        for( n = 0; n < MAXLEVEL; ++n )
        {
            pFormat = new SwNumFormat;
            pFormat->SetIncludeUpperLevels( 1 );
            pFormat->SetStart( 1 );
            pFormat->SetAbsLSpace( lNumIndent + SwNumRule::GetNumIndent( n ) );
            pFormat->SetFirstLineOffset( lNumFirstLineOffset );
            pFormat->SetSuffix( "." );
            pFormat->SetBulletChar( numfunc::GetBulletChar(n));
            SwNumRule::maBaseFormats[ NUM_RULE ][ n ] = pFormat;
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
            pFormat = new SwNumFormat;
            pFormat->SetIncludeUpperLevels( 1 );
            pFormat->SetStart( 1 );
            pFormat->SetPositionAndSpaceMode( SvxNumberFormat::LABEL_ALIGNMENT );
            pFormat->SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
            pFormat->SetListtabPos( cIndentAt[ n ] );
            pFormat->SetFirstLineIndent( cFirstLineIndent );
            pFormat->SetIndentAt( cIndentAt[ n ] );
            pFormat->SetSuffix( "." );
            pFormat->SetBulletChar( numfunc::GetBulletChar(n));
            SwNumRule::maLabelAlignmentBaseFormats[ NUM_RULE ][ n ] = pFormat;
        }

        // outline:
        // position-and-space mode LABEL_WIDTH_AND_POSITION:
        for( n = 0; n < MAXLEVEL; ++n )
        {
            pFormat = new SwNumFormat;
            pFormat->SetNumberingType(SVX_NUM_NUMBER_NONE);
            pFormat->SetIncludeUpperLevels( MAXLEVEL );
            pFormat->SetStart( 1 );
            pFormat->SetCharTextDistance( lOutlineMinTextDistance );
            pFormat->SetBulletChar( numfunc::GetBulletChar(n));
            SwNumRule::maBaseFormats[ OUTLINE_RULE ][ n ] = pFormat;
        }
        // position-and-space mode LABEL_ALIGNMENT:
        for( n = 0; n < MAXLEVEL; ++n )
        {
            pFormat = new SwNumFormat;
            pFormat->SetNumberingType(SVX_NUM_NUMBER_NONE);
            pFormat->SetIncludeUpperLevels( MAXLEVEL );
            pFormat->SetStart( 1 );
            pFormat->SetPositionAndSpaceMode( SvxNumberFormat::LABEL_ALIGNMENT );
            pFormat->SetBulletChar( numfunc::GetBulletChar(n));
            SwNumRule::maLabelAlignmentBaseFormats[ OUTLINE_RULE ][ n ] = pFormat;
        }
    }
    memset( maFormats, 0, sizeof( maFormats ));
    OSL_ENSURE( !msName.isEmpty(), "NumRule without a name!" );
}

SwNumRule::SwNumRule( const SwNumRule& rNumRule )
    : maTextNodeList(),
      maParagraphStyleList(),
      mpNumRuleMap(nullptr),
      msName( rNumRule.msName ),
      meRuleType( rNumRule.meRuleType ),
      mnPoolFormatId( rNumRule.GetPoolFormatId() ),
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
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
        if( rNumRule.maFormats[ n ] )
            Set( n, *rNumRule.maFormats[ n ] );
}

SwNumRule::~SwNumRule()
{
    for (auto & i : maFormats)
        i.reset();

    if (mpNumRuleMap)
    {
        mpNumRuleMap->erase(GetName());
    }

    if( !--mnRefCount )          // the last one closes the door (?)
    {
            // Numbering:
            SwNumFormat** ppFormats = &SwNumRule::maBaseFormats[0][0];
            int n;

            for( n = 0; n < MAXLEVEL; ++n, ++ppFormats )
            {
                delete *ppFormats;
                *ppFormats = nullptr;
            }

            // Outline:
            for( n = 0; n < MAXLEVEL; ++n, ++ppFormats )
            {
                delete *ppFormats;
                *ppFormats = nullptr;
            }

            ppFormats = &SwNumRule::maLabelAlignmentBaseFormats[0][0];
            for( n = 0; n < MAXLEVEL; ++n, ++ppFormats )
            {
                delete *ppFormats;
                *ppFormats = nullptr;
            }
            for( n = 0; n < MAXLEVEL; ++n, ++ppFormats )
            {
                delete *ppFormats;
                *ppFormats = nullptr;
            }
    }

    maTextNodeList.clear();
    maParagraphStyleList.clear();
}

void SwNumRule::CheckCharFormats( SwDoc* pDoc )
{
    for(auto& rpNumFormat : maFormats)
    {
        SwCharFormat* pFormat;
        if( rpNumFormat && nullptr != ( pFormat = rpNumFormat->GetCharFormat() ) &&
            pFormat->GetDoc() != pDoc )
        {
            // copy
            SwNumFormat* pNew = new SwNumFormat( *rpNumFormat );
            pNew->SetCharFormat( pDoc->CopyCharFormat( *pFormat ) );
            rpNumFormat.reset(pNew);
        }
    }
}

SwNumRule& SwNumRule::operator=( const SwNumRule& rNumRule )
{
    if( this != &rNumRule )
    {
        for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
            Set( n, rNumRule.maFormats[ n ].get() );

        meRuleType = rNumRule.meRuleType;
        msName = rNumRule.msName;
        mbAutoRuleFlag = rNumRule.mbAutoRuleFlag;
        mbInvalidRuleFlag = true;
        mbContinusNum = rNumRule.mbContinusNum;
        mbAbsSpaces = rNumRule.mbAbsSpaces;
        mbHidden = rNumRule.mbHidden;
        mnPoolFormatId = rNumRule.GetPoolFormatId();
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
                mnPoolFormatId == rRule.GetPoolFormatId() &&
                mnPoolHelpId == rRule.GetPoolHelpId() &&
                mnPoolHlpFileId == rRule.GetPoolHlpFileId();
    if( bRet )
    {
        for( sal_uInt8 n = 0; n < MAXLEVEL; ++n )
            if( rRule.Get( n ) != Get( n ) )
            {
                bRet = false;
                break;
            }
    }
    return bRet;
}

void SwNumRule::Set( sal_uInt16 i, const SwNumFormat& rNumFormat )
{
    OSL_ENSURE( i < MAXLEVEL, "Serious defect" );
    if( i < MAXLEVEL )
    {
        if( !maFormats[ i ] || (rNumFormat != Get( i )) )
        {
            maFormats[ i ].reset(new SwNumFormat( rNumFormat ));
            mbInvalidRuleFlag = true;
        }
    }
}

void SwNumRule::Set( sal_uInt16 i, const SwNumFormat* pNumFormat )
{
    OSL_ENSURE( i < MAXLEVEL, "Serious defect" );
    if( i >= MAXLEVEL )
        return;
    if( !maFormats[ i ] )
    {
        if( pNumFormat )
        {
            maFormats[ i ].reset(new SwNumFormat( *pNumFormat ));
            mbInvalidRuleFlag = true;
        }
    }
    else if( !pNumFormat )
    {
        maFormats[ i ].reset();
        mbInvalidRuleFlag = true;
    }
    else if( *maFormats[i] != *pNumFormat )
    {
        *maFormats[ i ] = *pNumFormat;
        mbInvalidRuleFlag = true;
    }
}

OUString SwNumRule::MakeNumString( const SwNodeNum& rNum, bool bInclStrings ) const
{
    if (rNum.IsCounted())
        return MakeNumString(rNum.GetNumberVector(), bInclStrings);

    return OUString();
}

OUString SwNumRule::MakeNumString( const SwNumberTree::tNumberVector & rNumVector,
                                 const bool bInclStrings,
                                 const bool bOnlyArabic,
                                 const unsigned int _nRestrictToThisLevel,
                                 SwNumRule::Extremities* pExtremities,
                                 LanguageType nLang ) const
{
    OUStringBuffer aStr;

    SwNumberTree::tNumberVector::size_type nLevel = rNumVector.size() - 1;

    if ( pExtremities )
        pExtremities->nPrefixChars = pExtremities->nSuffixChars = 0;

    if ( nLevel > _nRestrictToThisLevel )
    {
        nLevel = _nRestrictToThisLevel;
    }

    if (nLevel < MAXLEVEL)
    {
        const SwNumFormat& rMyNFormat = Get( static_cast<sal_uInt16>(nLevel) );

        {
            SwNumberTree::tNumberVector::size_type i = nLevel;

            if( !IsContinusNum() &&
                // - do not include upper levels, if level isn't numbered.
                rMyNFormat.GetNumberingType() != SVX_NUM_NUMBER_NONE &&
                rMyNFormat.GetIncludeUpperLevels() )  // Just the own level?
            {
                sal_uInt8 n = rMyNFormat.GetIncludeUpperLevels();
                if( 1 < n )
                {
                    if( i+1 >= n )
                        i -= n - 1;
                    else
                        i = 0;
                }
            }

            css::lang::Locale aLocale( LanguageTag::convertToLocale(nLang));

            for( ; i <= nLevel; ++i )
            {
                const SwNumFormat& rNFormat = Get( i );
                if( SVX_NUM_NUMBER_NONE == rNFormat.GetNumberingType() )
                {
                    // Should 1.1.1 --> 2. NoNum --> 1..1 or 1.1 ??
                    //                 if( i != rNum.nMyLevel )
                    //                    aStr += ".";
                    continue;
                }

                if( rNumVector[ i ] )
                {
                    if( bOnlyArabic )
                        aStr.append(OUString::number( rNumVector[ i ] ));
                    else
                        aStr.append(rNFormat.GetNumStr( rNumVector[ i ], aLocale ));
                }
                else
                    aStr.append("0");        // all 0 level are a 0
                if( i != nLevel && !aStr.isEmpty() )
                    aStr.append(".");
            }

            // The type doesn't have any number, so don't append
            // the post-/prefix string
            if( bInclStrings && !bOnlyArabic &&
                SVX_NUM_CHAR_SPECIAL != rMyNFormat.GetNumberingType() &&
                SVX_NUM_BITMAP != rMyNFormat.GetNumberingType() )
            {
                const OUString& sPrefix = rMyNFormat.GetPrefix();
                const OUString& sSuffix = rMyNFormat.GetSuffix();

                aStr.insert(0, sPrefix);
                aStr.append(sSuffix);
                if ( pExtremities )
                {
                    pExtremities->nPrefixChars = sPrefix.getLength();
                    pExtremities->nSuffixChars = sSuffix.getLength();
                }
            }
        }
    }

    return aStr.makeStringAndClear();
}

OUString SwNumRule::MakeRefNumString( const SwNodeNum& rNodeNum,
                                    const bool bInclSuperiorNumLabels,
                                    const int nRestrictInclToThisLevel ) const
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

                SwNumFormat aFormat( Get( static_cast<sal_uInt16>(nListLevel) ) );
                bMakeNumStringForPhantom = aFormat.IsEnumeration() &&
                                           SVX_NUM_NUMBER_NONE != aFormat.GetNumberingType();

            }
            if ( bMakeNumStringForPhantom ||
                 ( !pWorkingNodeNum->IsPhantom() &&
                   pWorkingNodeNum->GetTextNode() &&
                   pWorkingNodeNum->GetTextNode()->HasNumber() ) )
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
                  pWorkingNodeNum->GetLevelInListTree() >= nRestrictInclToThisLevel );
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
        Set( n, rNumRule.maFormats[ n ].get() );
        if( maFormats[ n ] && maFormats[ n ]->GetCharFormat() &&
            !pDoc->GetCharFormats()->IsAlive(maFormats[n]->GetCharFormat()))
        {
            // If we copy across different Documents, then copy the
            // corresponding CharFormat into the new Document.
            maFormats[n]->SetCharFormat( pDoc->CopyCharFormat( *maFormats[n]->
                                        GetCharFormat() ) );
        }
    }
    meRuleType = rNumRule.meRuleType;
    msName = rNumRule.msName;
    mbAutoRuleFlag = rNumRule.mbAutoRuleFlag;
    mnPoolFormatId = rNumRule.GetPoolFormatId();
    mnPoolHelpId = rNumRule.GetPoolHelpId();
    mnPoolHlpFileId = rNumRule.GetPoolHlpFileId();
    mbInvalidRuleFlag = true;
    return *this;
}

void SwNumRule::SetSvxRule(const SvxNumRule& rNumRule, SwDoc* pDoc)
{
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        const SvxNumberFormat* pSvxFormat = rNumRule.Get(n);
        maFormats[n].reset( pSvxFormat ? new SwNumFormat(*pSvxFormat, pDoc) : nullptr );
    }

    mbInvalidRuleFlag = true;
    mbContinusNum = rNumRule.IsContinuousNumbering();
}

SvxNumRule SwNumRule::MakeSvxNumRule() const
{
    SvxNumRule aRule(SvxNumRuleFlags::CONTINUOUS | SvxNumRuleFlags::CHAR_STYLE |
                     SvxNumRuleFlags::ENABLE_LINKED_BMP | SvxNumRuleFlags::ENABLE_EMBEDDED_BMP,
                     MAXLEVEL, mbContinusNum,
                     meRuleType == NUM_RULE ? SvxNumRuleType::NUMBERING : SvxNumRuleType::OUTLINE_NUMBERING );
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        SwNumFormat aNumFormat = Get(n);
        if(aNumFormat.GetCharFormat())
            aNumFormat.SetCharFormatName(aNumFormat.GetCharFormat()->GetName());
        aRule.SetLevel(n, aNumFormat, maFormats[n] != nullptr);
    }
    return aRule;
}

void SwNumRule::SetInvalidRule(bool bFlag)
{
    if (bFlag)
    {
        std::set< SwList* > aLists;
        for ( const SwTextNode* pTextNode : maTextNodeList )
        {
            // #i111681# - applying patch from cmc
            SwList* pList = pTextNode->GetDoc()->getIDocumentListsAccess().getListByName( pTextNode->GetListId() );
            OSL_ENSURE( pList, "<SwNumRule::SetInvalidRule(..)> - list at which the text node is registered at does not exist. This is a serious issue.");
            if ( pList )
            {
                aLists.insert( pList );
            }
        }
        for ( auto aList : aLists )
            aList->InvalidateListTree();
    }

    mbInvalidRuleFlag = bFlag;
}

/// change indent of all list levels by given difference
void SwNumRule::ChangeIndent( const sal_Int32 nDiff )
{
    for ( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
    {
        SwNumFormat aTmpNumFormat( Get(i) );

        const SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode(
                                        aTmpNumFormat.GetPositionAndSpaceMode() );
        if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
        {
            auto nNewIndent = nDiff +
                               aTmpNumFormat.GetAbsLSpace();
            if ( nNewIndent < 0 )
            {
                nNewIndent = 0;
            }
            aTmpNumFormat.SetAbsLSpace( nNewIndent );
        }
        else if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            // adjust also the list tab position, if a list tab stop is applied
            if ( aTmpNumFormat.GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
            {
                const long nNewListTab = aTmpNumFormat.GetListtabPos() +  nDiff;
                aTmpNumFormat.SetListtabPos( nNewListTab );
            }

            const long nNewIndent = nDiff +
                              aTmpNumFormat.GetIndentAt();
            aTmpNumFormat.SetIndentAt( nNewIndent );
        }

        Set( i, aTmpNumFormat );
    }

    SetInvalidRule( true );
}

/// set indent of certain list level to given value
void SwNumRule::SetIndent( const short nNewIndent,
                           const sal_uInt16 nListLevel )
{
    SwNumFormat aTmpNumFormat( Get(nListLevel) );

    const SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode(
                                        aTmpNumFormat.GetPositionAndSpaceMode() );
    if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        aTmpNumFormat.SetAbsLSpace( nNewIndent );
    }
    else if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        // adjust also the list tab position, if a list tab stop is applied
        if ( aTmpNumFormat.GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
        {
            const long nNewListTab = aTmpNumFormat.GetListtabPos() +
                                     ( nNewIndent - aTmpNumFormat.GetIndentAt() );
            aTmpNumFormat.SetListtabPos( nNewListTab );
        }

        aTmpNumFormat.SetIndentAt( nNewIndent );
    }

    SetInvalidRule( true );
}

/// set indent of first list level to given value and change other list level's
/// indents accordingly
void SwNumRule::SetIndentOfFirstListLevelAndChangeOthers( const short nNewIndent )
{
    SwNumFormat aTmpNumFormat( Get(0) );

    sal_Int32 nDiff( 0 );
    const SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode(
                                        aTmpNumFormat.GetPositionAndSpaceMode() );
    if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        nDiff = nNewIndent
                - aTmpNumFormat.GetFirstLineOffset()
                - aTmpNumFormat.GetAbsLSpace();
    }
    else if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        nDiff = nNewIndent - aTmpNumFormat.GetIndentAt();
    }
    if ( nDiff != 0  )
    {
        ChangeIndent( nDiff );
    }
}

void SwNumRule::Validate()
{
    std::set< SwList* > aLists;
    for ( const SwTextNode* pTextNode : maTextNodeList )
    {
        aLists.insert( pTextNode->GetDoc()->getIDocumentListsAccess().getListByName( pTextNode->GetListId() ) );
    }
    for ( auto aList : aLists )
        aList->ValidateListTree();

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

void SwNumRule::AddParagraphStyle( SwTextFormatColl& rTextFormatColl )
{
    tParagraphStyleList::iterator aIter =
        std::find( maParagraphStyleList.begin(), maParagraphStyleList.end(), &rTextFormatColl );

    if ( aIter == maParagraphStyleList.end() )
    {
        maParagraphStyleList.push_back( &rTextFormatColl );
    }
}

void SwNumRule::RemoveParagraphStyle( SwTextFormatColl& rTextFormatColl )
{
    tParagraphStyleList::iterator aIter =
        std::find( maParagraphStyleList.begin(), maParagraphStyleList.end(), &rTextFormatColl );

    if ( aIter != maParagraphStyleList.end() )
    {
        maParagraphStyleList.erase( aIter );
    }
}

void SwNumRule::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwNumRule"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("msName"), BAD_CAST(msName.toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("mnPoolFormatId"), BAD_CAST(OString::number(mnPoolFormatId).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("mbAutoRuleFlag"), BAD_CAST(OString::boolean(mbAutoRuleFlag).getStr()));
    xmlTextWriterEndElement(pWriter);
}

void SwNumRule::GetGrabBagItem(uno::Any& rVal) const
{
    if (mpGrabBagItem.get())
        mpGrabBagItem->QueryValue(rVal);
    else
        rVal <<= uno::Sequence<beans::PropertyValue>();
}

void SwNumRule::SetGrabBagItem(const uno::Any& rVal)
{
    if (!mpGrabBagItem.get())
        mpGrabBagItem.reset(new SfxGrabBagItem);

    mpGrabBagItem->PutValue(rVal, 0);
}

namespace numfunc
{
    /** class containing default bullet list configuration data */
    class SwDefBulletConfig : private utl::ConfigItem
    {
        public:
            static SwDefBulletConfig& getInstance();

            const OUString& GetFontname() const
            {
                return msFontname;
            }

            bool IsFontnameUserDefined() const
            {
                return mbUserDefinedFontname;
            }

            const vcl::Font& GetFont() const
            {
                return *mpFont;
            }

            sal_Unicode GetChar( sal_uInt8 p_nListLevel ) const
            {
                if (p_nListLevel >= MAXLEVEL)
                {
                    p_nListLevel = MAXLEVEL - 1;
                }

                return mnLevelChars[p_nListLevel];
            }

            SwDefBulletConfig();

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
            virtual void Notify( const uno::Sequence<OUString>& aPropertyNames ) override;
            virtual void ImplCommit() override;

            // default bullet list configuration data
            OUString msFontname;
            bool mbUserDefinedFontname;
            FontWeight meFontWeight;
            FontItalic meFontItalic;
            sal_Unicode mnLevelChars[MAXLEVEL];

            // default bullet list font instance
            std::unique_ptr<vcl::Font> mpFont;
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
        : ConfigItem( "Office.Writer/Numbering/DefaultBulletList" ),
          // default bullet font is now OpenSymbol
          msFontname( OUString("OpenSymbol") ),
          mbUserDefinedFontname( false ),
          meFontWeight( WEIGHT_DONTKNOW ),
          meFontItalic( ITALIC_NONE )
    {
        SetToDefault();
        LoadConfig();
        InitFont();

        // enable notification for changes on default bullet configuration change
        EnableNotification( GetPropNames() );
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
                            sal_Int16 nTmp = 0;
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
        mpFont.reset( new vcl::Font( msFontname, OUString(), Size( 0, 14 ) ) );
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

    OUString const & GetDefBulletFontname()
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

            bool ChangeIndentOnTabAtFirstPosOfFirstListItem() const
            {
                return mbChangeIndentOnTabAtFirstPosOfFirstListItem;
            }

            SwNumberingUIBehaviorConfig();

        private:

            /** sets internal configuration data to default values */
            void SetToDefault();

            /** returns sequence of configuration property names */
            static css::uno::Sequence<OUString> GetPropNames();

            /** loads configuration properties and applies values to internal data */
            void LoadConfig();

            /** catches notification about changed configuration data */
            virtual void Notify( const css::uno::Sequence<OUString>& aPropertyNames ) override;
            virtual void ImplCommit() override;

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
        : ConfigItem( "Office.Writer/Numbering/UserInterfaceBehavior" ),
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

    css::uno::Sequence<OUString> SwNumberingUIBehaviorConfig::GetPropNames()
    {
        css::uno::Sequence<OUString> aPropNames { "ChangeIndentOnTabAtFirstPosOfFirstListItem" };

        return aPropNames;
    }

    void SwNumberingUIBehaviorConfig::ImplCommit() {}

    void SwNumberingUIBehaviorConfig::LoadConfig()
    {
        css::uno::Sequence<OUString> aPropNames = GetPropNames();
        css::uno::Sequence<css::uno::Any> aValues = GetProperties( aPropNames );
        const css::uno::Any* pValues = aValues.getConstArray();
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

    void SwNumberingUIBehaviorConfig::Notify( const css::uno::Sequence<OUString>& )
    {
        SetToDefault();
        LoadConfig();
    }

    bool ChangeIndentOnTabAtFirstPosOfFirstListItem()
    {
        return SwNumberingUIBehaviorConfig::getInstance().ChangeIndentOnTabAtFirstPosOfFirstListItem();
    }

    SvxNumberFormat::SvxNumPositionAndSpaceMode GetDefaultPositionAndSpaceMode()
    {
        if (utl::ConfigManager::IsFuzzing())
            return SvxNumberFormat::LABEL_ALIGNMENT;

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

void SwNumRuleTable::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwNumRuleTable"));
    for (SwNumRule* pNumRule : *this)
        pNumRule->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
