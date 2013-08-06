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


#include <comphelper/string.hxx>
#include <tools/resid.hxx>
#include <hintids.hxx>
#include <swtypes.hxx>
#include <txtatr.hxx>
#include <ndtxt.hxx>
#include <txttxmrk.hxx>
#include <tox.hxx>
#include <poolfmt.hrc>
#include <doc.hxx>
#include <docary.hxx>
#include <paratr.hxx>
#include <editeng/tstpitem.hxx>
#include <SwStyleNameMapper.hxx>
#include <hints.hxx> // SwPtrMsgPoolItem
#include <algorithm>
#include <functional>
#include <switerator.hxx>

using namespace std;

const sal_Char* SwForm::aFormEntry      = "<E>";
const sal_Char* SwForm::aFormTab        = "<T>";
const sal_Char* SwForm::aFormPageNums   = "<#>";
const sal_Char* SwForm::aFormLinkStt    = "<LS>";
const sal_Char* SwForm::aFormLinkEnd    = "<LE>";
const sal_Char* SwForm::aFormEntryNum   = "<E#>";
const sal_Char* SwForm::aFormEntryTxt   = "<ET>";
const sal_Char* SwForm::aFormChapterMark= "<C>";
const sal_Char* SwForm::aFormText       = "<X>";
const sal_Char* SwForm::aFormAuth       = "<A>";
sal_uInt8 SwForm::nFormTabLen            = 3;
sal_uInt8 SwForm::nFormEntryLen          = 3;
sal_uInt8 SwForm::nFormPageNumsLen       = 3;
sal_uInt8 SwForm::nFormLinkSttLen        = 4;
sal_uInt8 SwForm::nFormLinkEndLen        = 4;
sal_uInt8 SwForm::nFormEntryNumLen       = 4;
sal_uInt8 SwForm::nFormEntryTxtLen       = 4;
sal_uInt8 SwForm::nFormChapterMarkLen    = 3;
sal_uInt8 SwForm::nFormTextLen           = 3;
sal_uInt8 SwForm::nFormAuthLen           = 5;

TYPEINIT2( SwTOXMark, SfxPoolItem, SwClient );    // fuers rtti

struct PatternIni
{
    sal_uInt16 n1;
    sal_uInt16 n2;
    sal_uInt16 n3;
    sal_uInt16 n4;
    sal_uInt16 n5;
};
const PatternIni aPatternIni[] =
{
    {USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX},    //Header - no pattern
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},   //AUTH_TYPE_ARTICLE,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_BOOK,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_BOOKLET,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CONFERENCE,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_INBOOK,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_INCOLLECTION,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_INPROCEEDINGS,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_JOURNAL,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_MANUAL,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_MASTERSTHESIS,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_MISC,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_PHDTHESIS,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_PROCEEDINGS,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_TECHREPORT,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_UNPUBLISHED,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_EMAIL,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, AUTH_FIELD_URL, USHRT_MAX},//AUTH_TYPE_WWW,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM1,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM2,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM3,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM4,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_TYPE_CUSTOM5,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_YEAR,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_URL,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM1,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM2,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM3,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM4,
    {AUTH_FIELD_AUTHOR, AUTH_FIELD_TITLE, AUTH_FIELD_YEAR, USHRT_MAX, USHRT_MAX},     //AUTH_FIELD_CUSTOM5,
    {USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX}
};

static SwFormTokens lcl_GetAuthPattern(sal_uInt16 nTypeId)
{
    SwFormTokens aRet;

    PatternIni aIni = aPatternIni[nTypeId];
    sal_uInt16 nVals[5];
    nVals[0] = aIni.n1;
    nVals[1] = aIni.n2;
    nVals[2] = aIni.n3;
    nVals[3] = aIni.n4;
    nVals[4] = aIni.n5;

    SwFormToken aStartToken( TOKEN_AUTHORITY );
    aStartToken.nAuthorityField = AUTH_FIELD_IDENTIFIER;
    aRet.push_back( aStartToken );
    SwFormToken aSeparatorToken( TOKEN_TEXT );
    aSeparatorToken.sText = OUString(": ");
    aRet.push_back( aSeparatorToken );
    SwFormToken aTextToken( TOKEN_TEXT );
    aTextToken.sText = OUString(", ");

    for(sal_uInt16 i = 0; i < 5 ; i++)
    {
        if(nVals[i] == USHRT_MAX)
            break;
        if( i > 0 )
            aRet.push_back( aTextToken );

         // -> #i21237#
        SwFormToken aToken(TOKEN_AUTHORITY);

        aToken.nAuthorityField = nVals[i];
        aRet.push_back(aToken);
        // <- #i21237#
    }

    return aRet;
}
/*--------------------------------------------------------------------
     TOX dtor and ctor
 --------------------------------------------------------------------*/


/// pool default constructor
SwTOXMark::SwTOXMark()
    : SfxPoolItem( RES_TXTATR_TOXMARK )
    , SwModify( 0 )
    ,
    pTxtAttr( 0 ),
    bAutoGenerated(sal_False),
    bMainEntry(sal_False)
{
}


SwTOXMark::SwTOXMark( const SwTOXType* pTyp )
    : SfxPoolItem( RES_TXTATR_TOXMARK )
    , SwModify( const_cast<SwTOXType*>(pTyp) )
    ,
    pTxtAttr( 0 ), nLevel( 0 ),
    bAutoGenerated(sal_False),
    bMainEntry(sal_False)
{
}


SwTOXMark::SwTOXMark( const SwTOXMark& rCopy )
    : SfxPoolItem( RES_TXTATR_TOXMARK )
    , SwModify(rCopy.GetRegisteredInNonConst())
    ,
    aPrimaryKey( rCopy.aPrimaryKey ), aSecondaryKey( rCopy.aSecondaryKey ),
    aTextReading( rCopy.aTextReading ),
    aPrimaryKeyReading( rCopy.aPrimaryKeyReading ),
    aSecondaryKeyReading( rCopy.aSecondaryKeyReading ),
    pTxtAttr( 0 ), nLevel( rCopy.nLevel ),
    bAutoGenerated( rCopy.bAutoGenerated),
    bMainEntry(rCopy.bMainEntry)
{
    // Copy AlternativString
    aAltText = rCopy.aAltText;
}


SwTOXMark::~SwTOXMark()
{
}


void SwTOXMark::RegisterToTOXType( SwTOXType& rMark )
{
    rMark.Add(this);
}

int SwTOXMark::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rAttr ), "Attributes are not the same" );
    return GetRegisteredIn() == ((SwTOXMark&)rAttr).GetRegisteredIn();
}


SfxPoolItem* SwTOXMark::Clone( SfxItemPool* ) const
{
    return new SwTOXMark( *this );
}

void SwTOXMark::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew)
{
    NotifyClients(pOld, pNew);
    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached uno object
        SetXTOXMark(::com::sun::star::uno::Reference<
                        ::com::sun::star::text::XDocumentIndexMark>(0));
    }
}

void SwTOXMark::InvalidateTOXMark()
{
    SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT,
        &static_cast<SwModify&>(*this) ); // cast to base class!
    NotifyClients(&aMsgHint, &aMsgHint);
}

String SwTOXMark::GetText() const
{
    String aStr;
    if( aAltText.Len() )
        aStr = aAltText;
    else if( pTxtAttr && pTxtAttr->GetpTxtNd() )
    {
        xub_StrLen* pEndIdx = pTxtAttr->GetEnd();
        OSL_ENSURE( pEndIdx, "TOXMark without mark!");
        if( pEndIdx )
        {
            const xub_StrLen nStt = *pTxtAttr->GetStart();
            aStr = pTxtAttr->GetpTxtNd()->GetExpandTxt( nStt, *pEndIdx-nStt );
        }
    }
    return aStr;
}

void SwTOXMark::InsertTOXMarks( SwTOXMarks& aMarks, const SwTOXType& rType )
{
    SwIterator<SwTOXMark,SwTOXType> aIter(rType);
    SwTOXMark* pMark = aIter.First();
    while( pMark )
    {
        if(pMark->GetTxtTOXMark())
            aMarks.push_back(pMark);
        pMark = aIter.Next();
    }
}

/*--------------------------------------------------------------------
  Manage types of TOX
 --------------------------------------------------------------------*/

SwTOXType::SwTOXType( TOXTypes eTyp, const String& rName )
    : SwModify(0),
    aName(rName),
    eType(eTyp)
{
}


SwTOXType::SwTOXType(const SwTOXType& rCopy)
    : SwModify( (SwModify*)rCopy.GetRegisteredIn() ),
    aName(rCopy.aName),
    eType(rCopy.eType)
{
}

/*--------------------------------------------------------------------
    Edit forms
  --------------------------------------------------------------------*/

SwForm::SwForm( TOXTypes eTyp ) // #i21237#
    : eType( eTyp ), nFormMaxLevel( SwForm::GetFormMaxLevel( eTyp )),
//  nFirstTabPos( lNumIndent ),
    bCommaSeparated(sal_False)
{
    //bHasFirstTabPos =
    bGenerateTabPos = sal_False;
    bIsRelTabPos = sal_True;

    // The table of contents has a certain number of headlines + headings
    // The user has 10 levels + headings
    // Keyword has 3 levels + headings+ separator
    // Indexes of tables, object illustrations and authorities consist of a heading and one level

    sal_uInt16 nPoolId;
    switch( eType )
    {
    case TOX_INDEX:         nPoolId = STR_POOLCOLL_TOX_IDXH;    break;
    case TOX_USER:          nPoolId = STR_POOLCOLL_TOX_USERH;   break;
    case TOX_CONTENT:       nPoolId = STR_POOLCOLL_TOX_CNTNTH;  break;
    case TOX_ILLUSTRATIONS: nPoolId = STR_POOLCOLL_TOX_ILLUSH;  break;
    case TOX_OBJECTS      : nPoolId = STR_POOLCOLL_TOX_OBJECTH; break;
    case TOX_TABLES       : nPoolId = STR_POOLCOLL_TOX_TABLESH; break;
    case TOX_AUTHORITIES  : nPoolId = STR_POOLCOLL_TOX_AUTHORITIESH;    break;
    default:
        OSL_ENSURE( !this, "invalid TOXTyp");
        return ;
    }

    SwFormTokens aTokens;
    if (TOX_CONTENT == eType || TOX_ILLUSTRATIONS == eType )
    {
        SwFormToken aLinkStt (TOKEN_LINK_START);
        aLinkStt.sCharStyleName = String(SW_RES(STR_POOLCHR_TOXJUMP));
        aTokens.push_back(aLinkStt);
    }

    if (TOX_CONTENT == eType)
    {
        aTokens.push_back(SwFormToken(TOKEN_ENTRY_NO));
        aTokens.push_back(SwFormToken(TOKEN_ENTRY_TEXT));
    }
    else
        aTokens.push_back(SwFormToken(TOKEN_ENTRY));

    if (TOX_AUTHORITIES != eType)
    {
        SwFormToken aToken(TOKEN_TAB_STOP);
        aToken.nTabStopPosition = 0;

        // #i36870# right aligned tab for all
        aToken.cTabFillChar = '.';
        aToken.eTabAlign = SVX_TAB_ADJUST_END;

        aTokens.push_back(aToken);
        aTokens.push_back(SwFormToken(TOKEN_PAGE_NUMS));
    }

    if (TOX_CONTENT == eType || TOX_ILLUSTRATIONS == eType)
        aTokens.push_back(SwFormToken(TOKEN_LINK_END));

    SetTemplate( 0, SW_RESSTR( nPoolId++ ));

    if(TOX_INDEX == eType)
    {
        for( sal_uInt16 i = 1; i < 5; ++i  )
        {
            if(1 == i)
            {
                SwFormTokens aTmpTokens;
                SwFormToken aTmpToken(TOKEN_ENTRY);
                aTmpTokens.push_back(aTmpToken);

                SetPattern( i, aTmpTokens );
                SetTemplate( i, SW_RESSTR( STR_POOLCOLL_TOX_IDXBREAK    ));
            }
            else
            {
                SetPattern( i, aTokens );
                SetTemplate( i, SW_RESSTR( STR_POOLCOLL_TOX_IDX1 + i - 2 ));
            }
        }
    }
    else
        for( sal_uInt16 i = 1; i < GetFormMax(); ++i, ++nPoolId )    // Number 0 is the title
        {
            if(TOX_AUTHORITIES == eType)
                SetPattern(i, lcl_GetAuthPattern(i));
            else
                SetPattern( i, aTokens );

            if( TOX_CONTENT == eType && 6 == i )
                nPoolId = STR_POOLCOLL_TOX_CNTNT6;
            else if( TOX_USER == eType && 6 == i )
                nPoolId = STR_POOLCOLL_TOX_USER6;
            else if( TOX_AUTHORITIES == eType )
                nPoolId = STR_POOLCOLL_TOX_AUTHORITIES1;
            SetTemplate( i, SW_RESSTR( nPoolId ) );
        }
}


SwForm::SwForm(const SwForm& rForm)
    : eType( rForm.eType )
{
    *this = rForm;
}


SwForm& SwForm::operator=(const SwForm& rForm)
{
    eType = rForm.eType;
    nFormMaxLevel = rForm.nFormMaxLevel;
//  nFirstTabPos = rForm.nFirstTabPos;
//  bHasFirstTabPos = rForm.bHasFirstTabPos;
    bGenerateTabPos = rForm.bGenerateTabPos;
    bIsRelTabPos = rForm.bIsRelTabPos;
    bCommaSeparated = rForm.bCommaSeparated;
    for(sal_uInt16 i=0; i < nFormMaxLevel; ++i)
    {
        aPattern[i] = rForm.aPattern[i];
        aTemplate[i] = rForm.aTemplate[i];
    }
    return *this;
}

sal_uInt16 SwForm::GetFormMaxLevel( TOXTypes eTOXType )
{
    sal_uInt16 nRet = 0;
    switch( eTOXType )
    {
        case TOX_INDEX:         nRet = 5;                   break;
        case TOX_USER:          nRet = MAXLEVEL+1;          break;
        case TOX_CONTENT:       nRet = MAXLEVEL+1;          break;
        case TOX_ILLUSTRATIONS:
        case TOX_OBJECTS      :
        case TOX_TABLES       : nRet = 2; break;
        case TOX_AUTHORITIES  : nRet = AUTH_TYPE_END + 1;       break;
    }
    return nRet;
}

// #i21237#
bool operator == (const SwFormToken & rToken, FormTokenType eType)
{
    return rToken.eTokenType == eType;
}

//-----------------------------------------------------------------------------
void SwForm::AdjustTabStops(SwDoc& rDoc, sal_Bool bInsertNewTapStops) // #i21237#
{
    for(sal_uInt16 nLevel = 1; nLevel < GetFormMax(); nLevel++)
    {
        const String& sTemplateName = GetTemplate(nLevel);

        SwTxtFmtColl* pColl = rDoc.FindTxtFmtCollByName( sTemplateName );
        if( !pColl )
        {
            sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName
                ( sTemplateName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL ); // #i21237#
            if( USHRT_MAX != nId )
                pColl = rDoc.GetTxtCollFromPool( nId );
        }

        const SvxTabStopItem* pTabStops = 0;
        sal_uInt16 nTabCount = 0;
        if( pColl &&
            0 != ( pTabStops = &pColl->GetTabStops(sal_False) ) &&
            0 != ( nTabCount = pTabStops->Count() ) )
        {
            // #i21237#
            SwFormTokens aCurrentPattern = GetPattern(nLevel);
            SwFormTokens::iterator aIt = aCurrentPattern.begin();

            bool bChanged = false;

            for(sal_uInt16 nTab = 0; nTab < nTabCount; ++nTab)
            {
                const SvxTabStop& rTab = (*pTabStops)[nTab];

                // #i29178#
                // For Word import, we do not want to replace existing tokens,
                // we insert new tabstop tokens without a tabstop character:
                if ( bInsertNewTapStops )
                {
                    if ( SVX_TAB_ADJUST_DEFAULT != rTab.GetAdjustment() )
                    {
                        bChanged = true;
                        SwFormToken aToken(TOKEN_TAB_STOP);
                        aToken.bWithTab = sal_False;
                        aToken.nTabStopPosition = rTab.GetTabPos();
                        aToken.eTabAlign = rTab.GetAdjustment();
                        aToken.cTabFillChar = rTab.GetFill();
                        aCurrentPattern.push_back(aToken);
                    }
                }
                else
                {
                    aIt = find_if(aIt, aCurrentPattern.end(),
                                  SwFormTokenEqualToFormTokenType
                                  (TOKEN_TAB_STOP));
                    if ( aIt != aCurrentPattern.end() )
                    {
                        bChanged = true;
                        aIt->nTabStopPosition = rTab.GetTabPos();
                        aIt->eTabAlign = nTab == nTabCount - 1 &&
                                         SVX_TAB_ADJUST_RIGHT == rTab.GetAdjustment() ?
                                         SVX_TAB_ADJUST_END :
                                         rTab.GetAdjustment();
                        aIt->cTabFillChar = rTab.GetFill();
                        ++aIt;
                    }
                    else
                        break; // no more tokens to replace
                }
            }

            if(bChanged)
                SetPattern(nLevel, aCurrentPattern); // #i21237#
        }
    }
}
/*--------------------------------------------------------------------
  Ctor TOXBase
 --------------------------------------------------------------------*/


SwTOXBase::SwTOXBase(const SwTOXType* pTyp, const SwForm& rForm,
                     sal_uInt16 nCreaType, const String& rTitle )
    : SwClient((SwModify*)pTyp),
    aForm(rForm),
    aTitle(rTitle),
    eLanguage(::GetAppLanguage()),
    nCreateType(nCreaType),
    nOLEOptions(0),
    eCaptionDisplay(CAPTION_COMPLETE),
    bProtected( sal_True ),
    bFromChapter(sal_False),
    bFromObjectNames(sal_False),
    bLevelFromChapter(sal_False)
{
    aData.nOptions = 0;
}


SwTOXBase::SwTOXBase( const SwTOXBase& rSource, SwDoc* pDoc )
    : SwClient( rSource.GetRegisteredInNonConst() )
{
    CopyTOXBase( pDoc, rSource );
}

void SwTOXBase::RegisterToTOXType( SwTOXType& rType )
{
    rType.Add( this );
}

SwTOXBase& SwTOXBase::CopyTOXBase( SwDoc* pDoc, const SwTOXBase& rSource )
{
    SwTOXType* pType = (SwTOXType*)rSource.GetTOXType();
    if( pDoc && USHRT_MAX == pDoc->GetTOXTypes().GetPos( pType ))
    {
        // type not in pDoc, so create it now
        const SwTOXTypes& rTypes = pDoc->GetTOXTypes();
        bool bFound = false;
        for( sal_uInt16 n = rTypes.size(); n; )
        {
            const SwTOXType* pCmp = rTypes[ --n ];
            if( pCmp->GetType() == pType->GetType() &&
                pCmp->GetTypeName() == pType->GetTypeName() )
            {
                pType = (SwTOXType*)pCmp;
                bFound = true;
                break;
            }
        }

        if( !bFound )
            pType = (SwTOXType*)pDoc->InsertTOXType( *pType );
    }
    pType->Add( this );

    nCreateType = rSource.nCreateType;
    aTitle      = rSource.aTitle;
    aForm       = rSource.aForm;
    bProtected  = rSource.bProtected;
    bFromChapter = rSource.bFromChapter;
    bFromObjectNames = rSource.bFromObjectNames;
    sMainEntryCharStyle = rSource.sMainEntryCharStyle;
    sSequenceName = rSource.sSequenceName;
    eCaptionDisplay = rSource.eCaptionDisplay;
    nOLEOptions = rSource.nOLEOptions;
    eLanguage = rSource.eLanguage;
    sSortAlgorithm = rSource.sSortAlgorithm;
    bLevelFromChapter = rSource.bLevelFromChapter;

    for( sal_uInt16 i = 0; i < MAXLEVEL; ++i )
        aStyleNames[i] = rSource.aStyleNames[i];

    // its the same data type!
    aData.nOptions =  rSource.aData.nOptions;

    if( !pDoc || pDoc->IsCopyIsMove() )
        aName = rSource.GetTOXName();
    else
        aName = pDoc->GetUniqueTOXBaseName( *pType, rSource.GetTOXName() );

    return *this;
}

/*--------------------------------------------------------------------
  TOX specific functions
 --------------------------------------------------------------------*/

SwTOXBase::~SwTOXBase()
{
//    if( GetTOXType()->GetType() == TOX_USER  )
//        delete aData.pTemplateName;
}

void SwTOXBase::SetTitle(const String& rTitle)
    {   aTitle = rTitle; }


SwTOXBase & SwTOXBase::operator = (const SwTOXBase & rSource)
{
    aForm = rSource.aForm;
    aName = rSource.aName;
    aTitle = rSource.aTitle;
    sMainEntryCharStyle = rSource.sMainEntryCharStyle;
    for(sal_uInt16 nLevel = 0; nLevel < MAXLEVEL; nLevel++)
        aStyleNames[nLevel] = rSource.aStyleNames[nLevel];
    sSequenceName = rSource.sSequenceName;
    eLanguage = rSource.eLanguage;
    sSortAlgorithm = rSource.sSortAlgorithm;
    aData = rSource.aData;
    nCreateType = rSource.nCreateType;
    nOLEOptions = rSource.nOLEOptions;
    eCaptionDisplay = rSource.eCaptionDisplay;
    bProtected = rSource.bProtected;
    bFromChapter = rSource.bFromChapter;
    bFromObjectNames = rSource.bFromObjectNames;
    bLevelFromChapter = rSource.bLevelFromChapter;

    if (rSource.GetAttrSet())
        SetAttrSet(*rSource.GetAttrSet());

    return *this;
}

String SwFormToken::GetString() const
{
    String sRet;

    bool bAppend = true;
    switch( eTokenType )
    {
        case TOKEN_ENTRY_NO:
            sRet.AssignAscii( SwForm::aFormEntryNum );
        break;
        case TOKEN_ENTRY_TEXT:
            sRet.AssignAscii( SwForm::aFormEntryTxt );
        break;
        case TOKEN_ENTRY:
            sRet.AssignAscii( SwForm::aFormEntry );
        break;
        case TOKEN_TAB_STOP:
            sRet.AssignAscii( SwForm::aFormTab );
        break;
        case TOKEN_TEXT:
            sRet.AssignAscii( SwForm::aFormText );
        break;
        case TOKEN_PAGE_NUMS:
            sRet.AssignAscii( SwForm::aFormPageNums );
        break;
        case TOKEN_CHAPTER_INFO:
            sRet.AssignAscii( SwForm::aFormChapterMark );
        break;
        case TOKEN_LINK_START:
            sRet.AssignAscii( SwForm::aFormLinkStt );
        break;
        case TOKEN_LINK_END:
            sRet.AssignAscii( SwForm::aFormLinkEnd );
        break;
        case TOKEN_AUTHORITY:
        {
            sRet.AssignAscii( SwForm::aFormAuth );
            String sTmp( OUString::number( nAuthorityField ));
            if( sTmp.Len() < 2 )
                sTmp.Insert('0', 0);
            sRet.Insert( sTmp, 2 );
        }
        break;
        case TOKEN_END:
        break;
    }
    sRet.Erase( sRet.Len() - 1 );
    sRet += ' ';
    sRet += sCharStyleName;
    sRet += ',';
    sRet += OUString::number( nPoolId );
    sRet += ',';

    // TabStopPosition and TabAlign or ChapterInfoFormat
    if(TOKEN_TAB_STOP == eTokenType)
    {
        sRet += OUString::number( nTabStopPosition );
        sRet += ',';
        sRet += OUString::number( static_cast< sal_Int32 >(eTabAlign) );
        sRet += ',';
        sRet += cTabFillChar;
        sRet += ',';
        sRet += OUString::number( bWithTab );
    }
    else if(TOKEN_CHAPTER_INFO == eTokenType)
    {
        sRet += OUString::number( nChapterFormat );
        // add maximum permitted level
        sRet += ',';
        sRet += OUString::number( nOutlineLevel );
    }
    else if(TOKEN_TEXT == eTokenType)
    {
        //append Text if Len() > 0 only!
        if( sText.Len() )
        {
            sRet += TOX_STYLE_DELIMITER;
            String sTmp(comphelper::string::remove(sText, TOX_STYLE_DELIMITER));
            sRet += sTmp;
            sRet += TOX_STYLE_DELIMITER;
        }
        else
            bAppend = false;
    }
    else if(TOKEN_ENTRY_NO == eTokenType)
    {
        sRet += OUString::number( nChapterFormat );
        // add maximum permitted level
        sRet += ',';
        sRet += OUString::number( nOutlineLevel );
    }

    if(bAppend)
    {
        sRet += '>';
    }
    else
    {
        // don't append empty text tokens
        sRet.Erase();
    }

    return sRet;
}

// -> #i21237#
SwFormTokensHelper::SwFormTokensHelper(const String & rPattern)
{
    xub_StrLen nCurPatternPos = 0;
    xub_StrLen nCurPatternLen = 0;

    while (nCurPatternPos < rPattern.Len())
    {
        nCurPatternPos = nCurPatternPos + nCurPatternLen;

        SwFormToken aToken = BuildToken(rPattern, nCurPatternPos);
        aTokens.push_back(aToken);
    }
}

SwFormToken SwFormTokensHelper::BuildToken( const String & sPattern,
                                           xub_StrLen & nCurPatternPos ) const
{
    String sToken( SearchNextToken(sPattern, nCurPatternPos) );
    nCurPatternPos = nCurPatternPos + sToken.Len();
    xub_StrLen nTokenLen;
    FormTokenType eTokenType = GetTokenType(sToken, &nTokenLen);

    // at this point sPattern contains the
    // character style name, the PoolId, tab stop position, tab stop alignment, chapter info format
    // the form is: CharStyleName, PoolId[, TabStopPosition|ChapterInfoFormat[, TabStopAlignment[, TabFillChar]]]
    // in text tokens the form differs from the others: CharStyleName, PoolId[,\0xffinserted text\0xff]
    SwFormToken eRet( eTokenType );
    String sAuthFieldEnum = sToken.Copy( 2, 2 );
    sToken = sToken.Copy( nTokenLen, sToken.Len() - nTokenLen - 1);

    eRet.sCharStyleName = sToken.GetToken( 0, ',');
    String sTmp( sToken.GetToken( 1, ',' ));
    if( sTmp.Len() )
        eRet.nPoolId = static_cast<sal_uInt16>(sTmp.ToInt32());

    switch( eTokenType )
    {
//i53420
    case TOKEN_ENTRY_NO:
        if( (sTmp = sToken.GetToken( 2, ',' ) ).Len() )
            eRet.nChapterFormat = static_cast<sal_uInt16>(sTmp.ToInt32());
        if( (sTmp = sToken.GetToken( 3, ',' ) ).Len() )
            eRet.nOutlineLevel = static_cast<sal_uInt16>(sTmp.ToInt32()); //the maximum outline level to examine
        break;

    case TOKEN_TEXT:
        {
            xub_StrLen nStartText = sToken.Search( TOX_STYLE_DELIMITER );
            if( STRING_NOTFOUND != nStartText )
            {
                xub_StrLen nEndText = sToken.Search( TOX_STYLE_DELIMITER,
                                                nStartText + 1);
                if( STRING_NOTFOUND != nEndText )
                {
                    eRet.sText = sToken.Copy( nStartText + 1,
                                                nEndText - nStartText - 1);
                }
            }
        }
        break;

    case TOKEN_TAB_STOP:
        if( (sTmp = sToken.GetToken( 2, ',' ) ).Len() )
            eRet.nTabStopPosition = sTmp.ToInt32();

        if( (sTmp = sToken.GetToken( 3, ',' ) ).Len() )
            eRet.eTabAlign = static_cast<SvxTabAdjust>(sTmp.ToInt32());

        if( (sTmp = sToken.GetToken( 4, ',' ) ).Len() )
            eRet.cTabFillChar = sTmp.GetChar(0);

        if( (sTmp = sToken.GetToken( 5, ',' ) ).Len() )
            eRet.bWithTab = 0 != sTmp.ToInt32();
        break;

    case TOKEN_CHAPTER_INFO:
        if( (sTmp = sToken.GetToken( 2, ',' ) ).Len() )
            eRet.nChapterFormat = static_cast<sal_uInt16>(sTmp.ToInt32()); //SwChapterFormat;
 //i53420
        if( (sTmp = sToken.GetToken( 3, ',' ) ).Len() )
            eRet.nOutlineLevel = static_cast<sal_uInt16>(sTmp.ToInt32()); //the maximum outline level to examine

        break;

    case TOKEN_AUTHORITY:
        eRet.nAuthorityField = static_cast<sal_uInt16>(sAuthFieldEnum.ToInt32());
        break;
    default: break;
    }
    return eRet;
}

String SwFormTokensHelper::SearchNextToken( const String & sPattern,
                                            xub_StrLen nStt ) const
{
    //it's not so easy - it doesn't work if the text part contains a '>'
    //sal_uInt16 nTokenEnd = sPattern.Search('>');

    String aResult;

    xub_StrLen nEnd = sPattern.Search( '>', nStt );
    if( STRING_NOTFOUND == nEnd )
    {
        nEnd = sPattern.Len();
    }
    else
    {
        xub_StrLen nTextSeparatorFirst = sPattern.Search( TOX_STYLE_DELIMITER, nStt );
        if( STRING_NOTFOUND != nTextSeparatorFirst )
        {
            xub_StrLen nTextSeparatorSecond = sPattern.Search( TOX_STYLE_DELIMITER,
                                                               nTextSeparatorFirst + 1 );
            if( STRING_NOTFOUND != nTextSeparatorSecond &&
                nEnd > nTextSeparatorFirst )
                nEnd = sPattern.Search( '>', nTextSeparatorSecond );
        }

        ++nEnd;

        aResult = sPattern.Copy( nStt, nEnd - nStt );
    }

    return aResult;
}

FormTokenType SwFormTokensHelper::GetTokenType(const String & sToken,
                                               xub_StrLen * pTokenLen) const
{
    static struct
    {
        const sal_Char* pNm;
        sal_uInt16 nLen;
        sal_uInt16 nOffset;
        FormTokenType eToken;
    } const aTokenArr[] = {
        { SwForm::aFormTab,       SwForm::nFormEntryLen,      1, TOKEN_TAB_STOP },
        { SwForm::aFormPageNums,  SwForm::nFormPageNumsLen,   1, TOKEN_PAGE_NUMS },
        { SwForm::aFormLinkStt,   SwForm::nFormLinkSttLen,    1, TOKEN_LINK_START },
        { SwForm::aFormLinkEnd,   SwForm::nFormLinkEndLen,    1, TOKEN_LINK_END },
        { SwForm::aFormEntryNum,  SwForm::nFormEntryNumLen,   1, TOKEN_ENTRY_NO },
        { SwForm::aFormEntryTxt,  SwForm::nFormEntryTxtLen,   1, TOKEN_ENTRY_TEXT },
        { SwForm::aFormChapterMark,SwForm::nFormChapterMarkLen,1,TOKEN_CHAPTER_INFO },
        { SwForm::aFormText,      SwForm::nFormTextLen,       1, TOKEN_TEXT },
        { SwForm::aFormEntry,     SwForm::nFormEntryLen,      1, TOKEN_ENTRY },
        { SwForm::aFormAuth,      SwForm::nFormAuthLen,       3, TOKEN_AUTHORITY },
        { 0,                      0,                          0, TOKEN_END }
    };

    FormTokenType eTokenType = TOKEN_TEXT;
    xub_StrLen nTokenLen = 0;
    const sal_Char* pNm;
    for( int i = 0; 0 != ( pNm = aTokenArr[ i ].pNm ); ++i )
        if( COMPARE_EQUAL == sToken.CompareToAscii( pNm,
                            aTokenArr[ i ].nLen - aTokenArr[ i ].nOffset ))
        {
            eTokenType = aTokenArr[ i ].eToken;
            nTokenLen = aTokenArr[ i ].nLen;
            break;
        }

    OSL_ENSURE( pNm, "wrong token" );
    if (pTokenLen)
        *pTokenLen = nTokenLen;

    return eTokenType;
}

// <- #i21237#

void SwForm::SetPattern(sal_uInt16 nLevel, const SwFormTokens& rTokens)
{
    OSL_ENSURE(nLevel < GetFormMax(), "Index >= FORM_MAX");
    aPattern[nLevel] = rTokens;
}

void SwForm::SetPattern(sal_uInt16 nLevel, const String & rStr)
{
    OSL_ENSURE(nLevel < GetFormMax(), "Index >= FORM_MAX");

    SwFormTokensHelper aHelper(rStr);
    aPattern[nLevel] = aHelper.GetTokens();
}

const SwFormTokens& SwForm::GetPattern(sal_uInt16 nLevel) const
{
    OSL_ENSURE(nLevel < GetFormMax(), "Index >= FORM_MAX");
    return aPattern[nLevel];
}

sal_uInt16 SwTOXTypes::GetPos(const SwTOXType* pTOXType) const
{
    const_iterator it = std::find(begin(), end(), pTOXType);
    return it == end() ? USHRT_MAX : it - begin();
}

SwTOXTypes::~SwTOXTypes()
{
    for(const_iterator it = begin(); it != end(); ++it)
        delete *it;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
