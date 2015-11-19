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

#include <sal/config.h>

#include <UndoTable.hxx>
#include <hintids.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/charclass.hxx>
#include <editeng/unolingu.hxx>
#include <svx/pageitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/fontitem.hxx>
#include <com/sun/star/text/SetVariableType.hpp>
#include <unofield.hxx>
#include <frmfmt.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <fmtanchr.hxx>
#include <txtftn.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <layfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <rootfrm.hxx>
#include <tabfrm.hxx>
#include <flyfrm.hxx>
#include <ftnfrm.hxx>
#include <rowfrm.hxx>
#include <expfld.hxx>
#include <usrfld.hxx>
#include <ndtxt.hxx>
#include <calc.hxx>
#include <pam.hxx>
#include <docfld.hxx>
#include <swcache.hxx>
#include <swtable.hxx>
#include <breakit.hxx>
#include <SwStyleNameMapper.hxx>
#include <unofldmid.h>
#include <numrule.hxx>
#include <calbck.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;

static sal_Int16 lcl_SubTypeToAPI(sal_uInt16 nSubType)
{
        sal_Int16 nRet = 0;
        switch(nSubType)
        {
            case nsSwGetSetExpType::GSE_EXPR:
                nRet = SetVariableType::VAR;      // 0
                break;
            case nsSwGetSetExpType::GSE_SEQ:
                nRet = SetVariableType::SEQUENCE; // 1
                break;
            case nsSwGetSetExpType::GSE_FORMULA:
                nRet = SetVariableType::FORMULA;  // 2
                break;
            case nsSwGetSetExpType::GSE_STRING:
                nRet = SetVariableType::STRING;   // 3
                break;
        }
        return nRet;
}

static sal_Int32 lcl_APIToSubType(const uno::Any& rAny)
{
        sal_Int16 nVal = 0;
        rAny >>= nVal;
        sal_Int32 nSet = 0;
        switch(nVal)
        {
            case SetVariableType::VAR:      nSet = nsSwGetSetExpType::GSE_EXPR;  break;
            case SetVariableType::SEQUENCE: nSet = nsSwGetSetExpType::GSE_SEQ;  break;
            case SetVariableType::FORMULA:  nSet = nsSwGetSetExpType::GSE_FORMULA; break;
            case SetVariableType::STRING:   nSet = nsSwGetSetExpType::GSE_STRING;   break;
            default:
                OSL_FAIL("wrong value");
                nSet = -1;
        }
        return nSet;
}

OUString ReplacePoint( const OUString& rTmpName, bool bWithCommandType )
{
    // replace first and last (if bWithCommandType: last two) dot
    // since table names may contain dots

    sal_Int32 nIndex = rTmpName.lastIndexOf('.');
    if (nIndex<0)
    {
        return rTmpName;
    }

    OUString sRes = rTmpName.replaceAt(nIndex, 1, OUString(DB_DELIM));

    if (bWithCommandType)
    {
        nIndex = sRes.lastIndexOf('.', nIndex);
        if (nIndex<0)
        {
            return sRes;
        }
        sRes = sRes.replaceAt(nIndex, 1, OUString(DB_DELIM));
    }

    nIndex = sRes.indexOf('.');
    if (nIndex>=0)
    {
        sRes = sRes.replaceAt(nIndex, 1, OUString(DB_DELIM));
    }
    return sRes;
}

SwTextNode* GetFirstTextNode( const SwDoc& rDoc, SwPosition& rPos,
                            const SwContentFrame *pCFrame, Point &rPt )
{
    SwTextNode* pTextNode = nullptr;
    if ( !pCFrame )
    {
        const SwNodes& rNodes = rDoc.GetNodes();
        rPos.nNode = *rNodes.GetEndOfContent().StartOfSectionNode();
        SwContentNode* pCNd;
        while( nullptr != (pCNd = rNodes.GoNext( &rPos.nNode ) ) &&
                nullptr == ( pTextNode = pCNd->GetTextNode() ) )
                        ;
        OSL_ENSURE( pTextNode, "Where is the 1. TextNode?" );
        rPos.nContent.Assign( pTextNode, 0 );
    }
    else if ( !pCFrame->IsValid() )
    {
        pTextNode = const_cast<SwTextNode*>(static_cast<const SwTextNode*>(pCFrame->GetNode()));
        rPos.nNode = *pTextNode;
        rPos.nContent.Assign( pTextNode, 0 );
    }
    else
    {
        pCFrame->GetCursorOfst( &rPos, rPt );
        pTextNode = rPos.nNode.GetNode().GetTextNode();
    }
    return pTextNode;
}

const SwTextNode* GetBodyTextNode( const SwDoc& rDoc, SwPosition& rPos,
                                const SwFrame& rFrame )
{
    const SwLayoutFrame* pLayout = rFrame.GetUpper();
    const SwTextNode* pTextNode = nullptr;

    while( pLayout )
    {
        if( pLayout->IsFlyFrame() )
        {
            // get the FlyFormat
            const SwFrameFormat* pFlyFormat = static_cast<const SwFlyFrame*>(pLayout)->GetFormat();
            OSL_ENSURE( pFlyFormat, "Could not find FlyFormat, where is the field?" );

            const SwFormatAnchor &rAnchor = pFlyFormat->GetAnchor();

            if( FLY_AT_FLY == rAnchor.GetAnchorId() )
            {
                // the fly needs to be attached somewhere, so ask it
                pLayout = static_cast<const SwLayoutFrame*>(static_cast<const SwFlyFrame*>(pLayout)->GetAnchorFrame());
                continue;
            }
            else if ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                     (FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
                     (FLY_AS_CHAR == rAnchor.GetAnchorId()))
            {
                OSL_ENSURE( rAnchor.GetContentAnchor(), "no valid position" );
                rPos = *rAnchor.GetContentAnchor();
                pTextNode = rPos.nNode.GetNode().GetTextNode();
                if ( FLY_AT_PARA == rAnchor.GetAnchorId() )
                {
                    const_cast<SwTextNode*>(pTextNode)->MakeStartIndex(
                            &rPos.nContent );
                }

                // do not break yet, might be as well in Header/Footer/Footnote/Fly
                pLayout = static_cast<const SwFlyFrame*>(pLayout)->GetAnchorFrame()
                            ? static_cast<const SwFlyFrame*>(pLayout)->GetAnchorFrame()->GetUpper() : nullptr;
                continue;
            }
            else
            {
                pLayout->FindPageFrame()->GetContentPosition(
                                                pLayout->Frame().Pos(), rPos );
                pTextNode = rPos.nNode.GetNode().GetTextNode();
            }
        }
        else if( pLayout->IsFootnoteFrame() )
        {
            // get the anchor's node
            const SwTextFootnote* pFootnote = static_cast<const SwFootnoteFrame*>(pLayout)->GetAttr();
            pTextNode = &pFootnote->GetTextNode();
            rPos.nNode = *pTextNode;
            rPos.nContent = pFootnote->GetStart();
        }
        else if( pLayout->IsHeaderFrame() || pLayout->IsFooterFrame() )
        {
            const SwContentFrame* pContentFrame;
            const SwPageFrame* pPgFrame = pLayout->FindPageFrame();
            if( pLayout->IsHeaderFrame() )
            {
                const SwTabFrame *pTab;
                if( nullptr != ( pContentFrame = pPgFrame->FindFirstBodyContent()) &&
                    nullptr != (pTab = pContentFrame->FindTabFrame()) && pTab->IsFollow() &&
                    pTab->GetTable()->GetRowsToRepeat() > 0 &&
                    pTab->IsInHeadline( *pContentFrame ) )
                {
                    // take the next line
                    const SwLayoutFrame* pRow = pTab->GetFirstNonHeadlineRow();
                    pContentFrame = pRow->ContainsContent();
                }
            }
            else
                pContentFrame = pPgFrame->FindLastBodyContent();

            if( pContentFrame )
            {
                pTextNode = pContentFrame->GetNode()->GetTextNode();
                rPos.nNode = *pTextNode;
                const_cast<SwTextNode*>(pTextNode)->MakeEndIndex( &rPos.nContent );
            }
            else
            {
                Point aPt( pLayout->Frame().Pos() );
                aPt.Y()++;      // get out of the header
                pContentFrame = pPgFrame->GetContentPos( aPt, false, true );
                pTextNode = GetFirstTextNode( rDoc, rPos, pContentFrame, aPt );
            }
        }
        else
        {
            pLayout = pLayout->GetUpper();
            continue;
        }
        break; // found, so finish loop
    }
    return pTextNode;
}

SwGetExpFieldType::SwGetExpFieldType(SwDoc* pDc)
    : SwValueFieldType( pDc, RES_GETEXPFLD )
{
}

SwFieldType* SwGetExpFieldType::Copy() const
{
    return new SwGetExpFieldType(GetDoc());
}

void SwGetExpFieldType::Modify( const SfxPoolItem*, const SfxPoolItem* pNew )
{
    if( pNew && RES_DOCPOS_UPDATE == pNew->Which() )
        NotifyClients( nullptr, pNew );
    // do not expand anything else
}

SwGetExpField::SwGetExpField(SwGetExpFieldType* pTyp, const OUString& rFormel,
                            sal_uInt16 nSub, sal_uLong nFormat)
    : SwFormulaField( pTyp, nFormat, 0.0 ),
    bIsInBodyText( true ),
    nSubType(nSub),
    bLateInitialization( false )
{
    SetFormula( rFormel );
}

OUString SwGetExpField::Expand() const
{
    if(nSubType & nsSwExtendedSubType::SUB_CMD)
        return GetFormula();

    return sExpand;
}

OUString SwGetExpField::GetFieldName() const
{
    const sal_uInt16 nType = static_cast<sal_uInt16>(
        (nsSwGetSetExpType::GSE_FORMULA & nSubType)
        ? TYP_FORMELFLD
        : TYP_GETFLD);

    return SwFieldType::GetTypeStr(nType) + " " + GetFormula();
}

SwField* SwGetExpField::Copy() const
{
    SwGetExpField *pTmp = new SwGetExpField(static_cast<SwGetExpFieldType*>(GetTyp()),
                                            GetFormula(), nSubType, GetFormat());
    pTmp->SetLanguage(GetLanguage());
    pTmp->SwValueField::SetValue(GetValue());
    pTmp->sExpand       = sExpand;
    pTmp->bIsInBodyText  = bIsInBodyText;
    pTmp->SetAutomaticLanguage(IsAutomaticLanguage());
    if( bLateInitialization )
        pTmp->SetLateInitialization();

    return pTmp;
}

void SwGetExpField::ChangeExpansion( const SwFrame& rFrame, const SwTextField& rField )
{
    if( bIsInBodyText ) // only fields in Footer, Header, FootNote, Flys
        return;

    OSL_ENSURE( !rFrame.IsInDocBody(), "Flag incorrect, frame is in DocBody" );

    // determine document (or is there an easier way?)
    const SwTextNode* pTextNode = &rField.GetTextNode();
    SwDoc& rDoc = *const_cast<SwDoc*>(pTextNode->GetDoc());

    // create index for determination of the TextNode
    SwPosition aPos( SwNodeIndex( rDoc.GetNodes() ) );
    pTextNode = GetBodyTextNode( rDoc, aPos, rFrame );

    // If no layout exists, ChangeExpansion is called for header and
    // footer lines via layout formatting without existing TextNode.
    if(!pTextNode)
        return;
    // #i82544#
    if( bLateInitialization )
    {
        SwFieldType* pSetExpField = rDoc.getIDocumentFieldsAccess().GetFieldType(RES_SETEXPFLD, GetFormula(), false);
        if( pSetExpField )
        {
            bLateInitialization = false;
            if( !(GetSubType() & nsSwGetSetExpType::GSE_STRING) &&
                static_cast< SwSetExpFieldType* >(pSetExpField)->GetType() == nsSwGetSetExpType::GSE_STRING )
            SetSubType( nsSwGetSetExpType::GSE_STRING );
        }
    }

    _SetGetExpField aEndField( aPos.nNode, &rField, &aPos.nContent );
    if(GetSubType() & nsSwGetSetExpType::GSE_STRING)
    {
        SwHash** ppHashTable;
        sal_uInt16 nSize;
        rDoc.getIDocumentFieldsAccess().FieldsToExpand( ppHashTable, nSize, aEndField );
        sExpand = LookString( ppHashTable, nSize, GetFormula() );
        ::DeleteHashTable( ppHashTable, nSize );
    }
    else
    {
        // fill calculator with values
        SwCalc aCalc( rDoc );
        rDoc.getIDocumentFieldsAccess().FieldsToCalc(aCalc, aEndField);

        // calculate value
        SetValue(aCalc.Calculate(GetFormula()).GetDouble());

        // analyse based on format
        sExpand = static_cast<SwValueFieldType*>(GetTyp())->ExpandValue(
                                GetValue(), GetFormat(), GetLanguage());
    }
}

OUString SwGetExpField::GetPar2() const
{
    return GetFormula();
}

void SwGetExpField::SetPar2(const OUString& rStr)
{
    SetFormula(rStr);
}

sal_uInt16 SwGetExpField::GetSubType() const
{
    return nSubType;
}

void SwGetExpField::SetSubType(sal_uInt16 nType)
{
    nSubType = nType;
}

void SwGetExpField::SetLanguage(sal_uInt16 nLng)
{
    if (nSubType & nsSwExtendedSubType::SUB_CMD)
        SwField::SetLanguage(nLng);
    else
        SwValueField::SetLanguage(nLng);
}

bool SwGetExpField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_DOUBLE:
        rAny <<= GetValue();
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= (sal_Int32)GetFormat();
        break;
    case FIELD_PROP_USHORT1:
         rAny <<= (sal_Int16)nSubType;
        break;
    case FIELD_PROP_PAR1:
         rAny <<= GetFormula();
        break;
    case FIELD_PROP_SUBTYPE:
        {
            sal_Int16 nRet = lcl_SubTypeToAPI(GetSubType() & 0xff);
            rAny <<= nRet;
        }
        break;
    case FIELD_PROP_BOOL2:
        rAny <<= 0 != (nSubType & nsSwExtendedSubType::SUB_CMD);
        break;
    case FIELD_PROP_PAR4:
        rAny <<= GetExpStr();
        break;
    default:
        return SwField::QueryValue(rAny, nWhichId);
    }
    return true;
}

bool SwGetExpField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    sal_Int32 nTmp = 0;
    switch( nWhichId )
    {
    case FIELD_PROP_DOUBLE:
        SwValueField::SetValue(*static_cast<double const *>(rAny.getValue()));
        break;
    case FIELD_PROP_FORMAT:
        rAny >>= nTmp;
        SetFormat(nTmp);
        break;
    case FIELD_PROP_USHORT1:
         rAny >>= nTmp;
         nSubType = static_cast<sal_uInt16>(nTmp);
        break;
    case FIELD_PROP_PAR1:
    {
        OUString sTmp;
        rAny >>= sTmp;
        SetFormula(sTmp);
        break;
    }
    case FIELD_PROP_SUBTYPE:
        nTmp = lcl_APIToSubType(rAny);
        if( nTmp >=0 )
            SetSubType( static_cast<sal_uInt16>((GetSubType() & 0xff00) | nTmp));
        break;
    case FIELD_PROP_BOOL2:
        if(*static_cast<sal_Bool const *>(rAny.getValue()))
            nSubType |= nsSwExtendedSubType::SUB_CMD;
        else
            nSubType &= (~nsSwExtendedSubType::SUB_CMD);
        break;
    case FIELD_PROP_PAR4:
    {
        OUString sTmp;
        rAny >>= sTmp;
        ChgExpStr(sTmp);
        break;
    }
    default:
        return SwField::PutValue(rAny, nWhichId);
    }
    return true;
}

SwSetExpFieldType::SwSetExpFieldType( SwDoc* pDc, const OUString& rName, sal_uInt16 nTyp )
    : SwValueFieldType( pDc, RES_SETEXPFLD ),
    sName( rName ),
    pOutlChgNd( nullptr ),
    sDelim( "." ),
    nType(nTyp), nLevel( UCHAR_MAX ),
    bDeleted( false )
{
    if( ( nsSwGetSetExpType::GSE_SEQ | nsSwGetSetExpType::GSE_STRING ) & nType )
        EnableFormat(false);    // do not use Numberformatter
}

SwFieldType* SwSetExpFieldType::Copy() const
{
    SwSetExpFieldType* pNew = new SwSetExpFieldType(GetDoc(), sName, nType);
    pNew->bDeleted = bDeleted;
    pNew->sDelim = sDelim;
    pNew->nLevel = nLevel;

    return pNew;
}

OUString SwSetExpFieldType::GetName() const
{
    return sName;
}

void SwSetExpFieldType::Modify( const SfxPoolItem*, const SfxPoolItem* )
{
    return;     // do not expand further
}

void SwSetExpFieldType::SetSeqFormat(sal_uLong nFormat)
{
    SwIterator<SwFormatField,SwFieldType> aIter(*this);
    for( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
        pFormatField->GetField()->ChangeFormat( nFormat );
}

sal_uLong SwSetExpFieldType::GetSeqFormat()
{
    if( !HasWriterListeners() )
        return SVX_NUM_ARABIC;

    const SwField *pField = SwIterator<SwFormatField,SwSetExpFieldType>(*this).First()->GetField();
    return pField->GetFormat();
}

sal_uInt16 SwSetExpFieldType::SetSeqRefNo( SwSetExpField& rField )
{
    if( !HasWriterListeners() || !(nsSwGetSetExpType::GSE_SEQ & nType) )
        return USHRT_MAX;

    std::vector<sal_uInt16> aArr;

    // check if number is already used and if a new one needs to be created
    SwIterator<SwFormatField,SwFieldType> aIter( *this );
    for( SwFormatField* pF = aIter.First(); pF; pF = aIter.Next() )
    {
        const SwTextNode* pNd;
        if( pF->GetField() != &rField && pF->GetTextField() &&
            nullptr != ( pNd = pF->GetTextField()->GetpTextNode() ) &&
            pNd->GetNodes().IsDocNodes() )
        {
            InsertSort( aArr, static_cast<SwSetExpField*>(pF->GetField())->GetSeqNumber() );
        }
    }

    // check first if number already exists
    sal_uInt16 nNum = rField.GetSeqNumber();
    if( USHRT_MAX != nNum )
    {
        std::vector<sal_uInt16>::size_type n {0};

        for( n = 0; n < aArr.size(); ++n )
            if( aArr[ n ] >= nNum )
                break;

        if( n == aArr.size() || aArr[ n ] > nNum )
            return nNum;            // no -> use it
    }

    // flagged all numbers, so determine the right number
    sal_uInt16 n = aArr.size();
    OSL_ENSURE( n == aArr.size(), "Array is too big for using a sal_uInt16 index" );

    if ( n > 0 && aArr[ n-1 ] != n-1 )
    {
        for( n = 0; n < aArr.size(); ++n )
            if( n != aArr[ n ] )
                break;
    }

    rField.SetSeqNumber( n );
    return n;
}

size_t SwSetExpFieldType::GetSeqFieldList( SwSeqFieldList& rList )
{
    rList.Clear();

    SwIterator<SwFormatField,SwFieldType> aIter( *this );
    for( SwFormatField* pF = aIter.First(); pF; pF = aIter.Next() )
    {
        const SwTextNode* pNd;
        if( pF->GetTextField() &&
            nullptr != ( pNd = pF->GetTextField()->GetpTextNode() ) &&
            pNd->GetNodes().IsDocNodes() )
        {
            _SeqFieldLstElem* pNew = new _SeqFieldLstElem(
                    pNd->GetExpandText(),
                    static_cast<SwSetExpField*>(pF->GetField())->GetSeqNumber() );
            rList.InsertSort( pNew );
        }
    }

    return rList.Count();
}

void SwSetExpFieldType::SetChapter( SwSetExpField& rField, const SwNode& rNd )
{
    const SwTextNode* pTextNd = rNd.FindOutlineNodeOfLevel( nLevel );
    if( pTextNd )
    {
        SwNumRule * pRule = pTextNd->GetNumRule();

        if (pRule)
        {
            // --> OD 2005-11-02 #i51089 - TUNING#
            if ( pTextNd->GetNum() )
            {
                const SwNodeNum & aNum = *(pTextNd->GetNum());

                // only get the number, without pre-/post-fixstrings
                OUString sNumber( pRule->MakeNumString(aNum, false ));

                if( !sNumber.isEmpty() )
                    rField.ChgExpStr( sNumber + sDelim + rField.GetExpStr() );
            }
            else
            {
                OSL_FAIL( "<SwSetExpFieldType::SetChapter(..)> - text node with numbering rule, but without number. This is a serious defect" );
            }
        }
    }
}

bool SwSetExpFieldType::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_SUBTYPE:
        {
            sal_Int16 nRet = lcl_SubTypeToAPI(GetType());
            rAny <<= nRet;
        }
        break;
    case FIELD_PROP_PAR2:
        rAny <<= GetDelimiter();
        break;
    case FIELD_PROP_SHORT1:
        {
            sal_Int8 nRet = nLevel < MAXLEVEL? nLevel : -1;
            rAny <<= nRet;
        }
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwSetExpFieldType::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_SUBTYPE:
        {
            sal_Int32 nSet = lcl_APIToSubType(rAny);
            if(nSet >=0)
                SetType(static_cast<sal_uInt16>(nSet));
        }
        break;
    case FIELD_PROP_PAR2:
        {
            OUString sTmp;
            rAny >>= sTmp;
            if( !sTmp.isEmpty() )
                SetDelimiter( sTmp );
            else
                SetDelimiter( " " );
        }
        break;
    case FIELD_PROP_SHORT1:
        {
            sal_Int8 nLvl = 0;
            rAny >>= nLvl;
            if(nLvl < 0 || nLvl >= MAXLEVEL)
                SetOutlineLvl(UCHAR_MAX);
            else
                SetOutlineLvl(nLvl);
        }
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwSeqFieldList::InsertSort( _SeqFieldLstElem* pNew )
{
    OUStringBuffer aBuf(pNew->sDlgEntry);
    const sal_Int32 nLen = aBuf.getLength();
    for (sal_Int32 i = 0; i < nLen; ++i)
    {
        if (aBuf[i]<' ')
        {
            aBuf[i]=' ';
        }
    }
    pNew->sDlgEntry = aBuf.makeStringAndClear();

    size_t nPos = 0;
    bool bRet = SeekEntry( *pNew, &nPos );
    if( !bRet )
        maData.insert( maData.begin() + nPos, pNew );
    return bRet;
}

bool SwSeqFieldList::SeekEntry( const _SeqFieldLstElem& rNew, size_t* pP ) const
{
    size_t nO = maData.size();
    size_t nU = 0;
    if( nO > 0 )
    {
        CollatorWrapper & rCaseColl = ::GetAppCaseCollator(),
                        & rColl = ::GetAppCollator();
        const CharClass& rCC = GetAppCharClass();

        //#59900# Sorting should sort number correctly (e.g. "10" after "9" not after "1")
        const OUString rTmp2 = rNew.sDlgEntry;
        sal_Int32 nFndPos2 = 0;
        const OUString sNum2( rTmp2.getToken( 0, ' ', nFndPos2 ));
        bool bIsNum2IsNumeric = CharClass::isAsciiNumeric( sNum2 );
        sal_Int32 nNum2 = bIsNum2IsNumeric ? sNum2.toInt32() : 0;

        nO--;
        while( nU <= nO )
        {
            const size_t nM = nU + ( nO - nU ) / 2;

            //#59900# Sorting should sort number correctly (e.g. "10" after "9" not after "1")
            const OUString rTmp1 = maData[nM]->sDlgEntry;
            sal_Int32 nFndPos1 = 0;
            const OUString sNum1( rTmp1.getToken( 0, ' ', nFndPos1 ));
            sal_Int32 nCmp;

            if( bIsNum2IsNumeric && rCC.isNumeric( sNum1 ) )
            {
                sal_Int32 nNum1 = sNum1.toInt32();
                nCmp = nNum2 - nNum1;
                if( 0 == nCmp )
                {
                    OUString aTmp1 = nFndPos1 != -1 ? rTmp1.copy(nFndPos1) : OUString();
                    OUString aTmp2 = nFndPos2 != -1 ? rTmp2.copy(nFndPos2) : OUString();
                    nCmp = rCaseColl.compareString(aTmp2, aTmp1);
                }
            }
            else
                nCmp = rColl.compareString( rTmp2, rTmp1 );

            if( 0 == nCmp )
            {
                if( pP ) *pP = nM;
                return true;
            }
            else if( 0 < nCmp )
                nU = nM + 1;
            else if( nM == 0 )
                break;
            else
                nO = nM - 1;
        }
    }
    if( pP ) *pP = nU;
    return false;
}

SwSetExpField::SwSetExpField(SwSetExpFieldType* pTyp, const OUString& rFormel,
                                        sal_uLong nFormat)
    : SwFormulaField( pTyp, nFormat, 0.0 ), nSeqNo( USHRT_MAX ),
    nSubType(0)
    , mpFormatField(nullptr)
{
    SetFormula(rFormel);
    // ignore SubType
    bInput = false;
    if( IsSequenceField() )
    {
        SwValueField::SetValue(1.0);
        if( rFormel.isEmpty() )
        {
            SetFormula(pTyp->GetName() + "+1");
        }
    }
}

void SwSetExpField::SetFormatField(SwFormatField & rFormatField)
{
    mpFormatField = &rFormatField;
}

OUString SwSetExpField::Expand() const
{
    if (nSubType & nsSwExtendedSubType::SUB_CMD)
    {   // we need the CommandString
        return GetTyp()->GetName() + " = " + GetFormula();
    }
    if(!(nSubType & nsSwExtendedSubType::SUB_INVISIBLE))
    {   // value is visible
        return sExpand;
    }
    return OUString();
}

/// @return the field name
OUString SwSetExpField::GetFieldName() const
{
    SwFieldTypesEnum const nStrType( (IsSequenceField())
                            ? TYP_SEQFLD
                            : (bInput)
                                ? TYP_SETINPFLD
                                : TYP_SETFLD   );

    OUString aStr(
        SwFieldType::GetTypeStr( static_cast<sal_uInt16>(nStrType) )
        + " "
        + GetTyp()->GetName() );

    // Sequence: without formula
    if (TYP_SEQFLD != nStrType)
    {
        aStr += " = " + GetFormula();
    }
    return aStr;
}

SwField* SwSetExpField::Copy() const
{
    SwSetExpField *pTmp = new SwSetExpField(static_cast<SwSetExpFieldType*>(GetTyp()),
                                            GetFormula(), GetFormat());
    pTmp->SwValueField::SetValue(GetValue());
    pTmp->sExpand       = sExpand;
    pTmp->SetAutomaticLanguage(IsAutomaticLanguage());
    pTmp->SetLanguage(GetLanguage());
    pTmp->aPText        = aPText;
    pTmp->bInput        = bInput;
    pTmp->nSeqNo        = nSeqNo;
    pTmp->SetSubType(GetSubType());

    return pTmp;
}

void SwSetExpField::SetSubType(sal_uInt16 nSub)
{
    static_cast<SwSetExpFieldType*>(GetTyp())->SetType(nSub & 0xff);
    nSubType = nSub & 0xff00;

    OSL_ENSURE( (nSub & 0xff) != 3, "SubType ist illegal!" );
}

sal_uInt16 SwSetExpField::GetSubType() const
{
    return static_cast<SwSetExpFieldType*>(GetTyp())->GetType() | nSubType;
}

void SwSetExpField::SetValue( const double& rAny )
{
    SwValueField::SetValue(rAny);

    if( IsSequenceField() )
        sExpand = FormatNumber( GetValue(), GetFormat() );
    else
        sExpand = static_cast<SwValueFieldType*>(GetTyp())->ExpandValue( rAny,
                                                GetFormat(), GetLanguage());
}

void SwGetExpField::SetValue( const double& rAny )
{
    SwValueField::SetValue(rAny);
    sExpand = static_cast<SwValueFieldType*>(GetTyp())->ExpandValue( rAny, GetFormat(),
                                                            GetLanguage());
}

/** Find the index of the reference text following the current field
 *
 * @param rFormat
 * @param rDoc
 * @param nHint search starting position after the current field (or 0 if default)
 * @return
 */
sal_Int32 SwGetExpField::GetReferenceTextPos( const SwFormatField& rFormat, SwDoc& rDoc, sal_Int32 nHint)
{

    const SwTextField* pTextField = rFormat.GetTextField();
    const SwTextNode& rTextNode = pTextField->GetTextNode();

    sal_Int32 nRet = nHint ? nHint : pTextField->GetStart() + 1;
    OUString sNodeText = rTextNode.GetText();

    if(nRet<sNodeText.getLength())
    {
        sNodeText = sNodeText.copy(nRet);

        // now check if sNodeText starts with a non-alphanumeric character plus blanks
        sal_uInt16 nSrcpt = g_pBreakIt->GetRealScriptOfText( sNodeText, 0 );

        static const sal_uInt16 nIds[] =
        {
            RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
            RES_CHRATR_FONT, RES_CHRATR_FONT,
            RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
            RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONT,
            RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_LANGUAGE,
            RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONT,
            0, 0
        };
        SwAttrSet aSet(rDoc.GetAttrPool(), nIds);
        rTextNode.GetAttr(aSet, nRet, nRet+1);

        if( RTL_TEXTENCODING_SYMBOL != static_cast<const SvxFontItem&>(aSet.Get(
                GetWhichOfScript( RES_CHRATR_FONT, nSrcpt )) ).GetCharSet() )
        {
            LanguageType eLang = static_cast<const SvxLanguageItem&>(aSet.Get(
                GetWhichOfScript( RES_CHRATR_LANGUAGE, nSrcpt )) ).GetLanguage();
            LanguageTag aLanguageTag( eLang);
            CharClass aCC( aLanguageTag);
            sal_Unicode c0 = sNodeText[0];
            bool bIsAlphaNum = aCC.isAlphaNumeric( sNodeText, 0 );
            if( !bIsAlphaNum ||
                (c0 == ' ' || c0 == '\t'))
            {
                // ignoring blanks
                nRet++;
                const sal_Int32 nLen = sNodeText.getLength();
                for (sal_Int32 i = 1;
                     i<nLen && (sNodeText[i]==' ' || sNodeText[i]=='\t');
                     ++i
                )
                    ++nRet;
            }
        }
    }
    return nRet;
}

OUString SwSetExpField::GetPar1() const
{
    return static_cast<const SwSetExpFieldType*>(GetTyp())->GetName();
}

OUString SwSetExpField::GetPar2() const
{
    sal_uInt16 nType = static_cast<SwSetExpFieldType*>(GetTyp())->GetType();

    if (nType & nsSwGetSetExpType::GSE_STRING)
        return GetFormula();
    return GetExpandedFormula();
}

void SwSetExpField::SetPar2(const OUString& rStr)
{
    sal_uInt16 nType = static_cast<SwSetExpFieldType*>(GetTyp())->GetType();

    if( !(nType & nsSwGetSetExpType::GSE_SEQ) || !rStr.isEmpty() )
    {
        if (nType & nsSwGetSetExpType::GSE_STRING)
            SetFormula(rStr);
        else
            SetExpandedFormula(rStr);
    }
}

bool SwSetExpField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    sal_Int32 nTmp32 = 0;
    sal_Int16 nTmp16 = 0;
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL2:
        if(*static_cast<sal_Bool const *>(rAny.getValue()))
            nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
        else
            nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
        break;
    case FIELD_PROP_FORMAT:
        rAny >>= nTmp32;
        SetFormat(nTmp32);
        break;
    case FIELD_PROP_USHORT2:
        {
            rAny >>= nTmp16;
            if(nTmp16 <= SVX_NUMBER_NONE )
                SetFormat(nTmp16);
            else {
                //exception(wrong_value)
                ;
            }
        }
        break;
    case FIELD_PROP_USHORT1:
        rAny >>= nTmp16;
        nSeqNo = nTmp16;
        break;
    case FIELD_PROP_PAR1:
        {
            OUString sTmp;
            rAny >>= sTmp;
            SetPar1( SwStyleNameMapper::GetUIName( sTmp, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL ) );
        }
        break;
    case FIELD_PROP_PAR2:
        {
            OUString uTmp;
            rAny >>= uTmp;
            //I18N - if the formula contains only "TypeName+1"
            //and it's one of the initially created sequence fields
            //then the localized names has to be replaced by a programmatic name
            OUString sMyFormula = SwXFieldMaster::LocalizeFormula(*this, uTmp, false);
            SetFormula( sMyFormula );
        }
        break;
    case FIELD_PROP_DOUBLE:
        {
            double fVal = 0.0;
            rAny >>= fVal;
            SetValue(fVal);
        }
        break;
    case FIELD_PROP_SUBTYPE:
        nTmp32 = lcl_APIToSubType(rAny);
        if(nTmp32 >= 0)
            SetSubType(static_cast<sal_uInt16>((GetSubType() & 0xff00) | nTmp32));
        break;
    case FIELD_PROP_PAR3:
        rAny >>= aPText;
        break;
    case FIELD_PROP_BOOL3:
        if(*static_cast<sal_Bool const *>(rAny.getValue()))
            nSubType |= nsSwExtendedSubType::SUB_CMD;
        else
            nSubType &= (~nsSwExtendedSubType::SUB_CMD);
        break;
    case FIELD_PROP_BOOL1:
        SetInputFlag(*static_cast<sal_Bool const *>(rAny.getValue()));
        break;
    case FIELD_PROP_PAR4:
        {
            OUString sTmp;
            rAny >>= sTmp;
            ChgExpStr( sTmp );
        }
        break;
    default:
        return SwField::PutValue(rAny, nWhichId);
    }
    return true;
}

bool SwSetExpField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL2:
        rAny <<= 0 == (nSubType & nsSwExtendedSubType::SUB_INVISIBLE);
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= (sal_Int32)GetFormat();
        break;
    case FIELD_PROP_USHORT2:
        rAny <<= (sal_Int16)GetFormat();
        break;
    case FIELD_PROP_USHORT1:
        rAny <<= (sal_Int16)nSeqNo;
        break;
    case FIELD_PROP_PAR1:
        rAny <<= OUString ( SwStyleNameMapper::GetProgName(GetPar1(), nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL ) );
        break;
    case FIELD_PROP_PAR2:
        {
            //I18N - if the formula contains only "TypeName+1"
            //and it's one of the initially created sequence fields
            //then the localized names has to be replaced by a programmatic name
            OUString sMyFormula = SwXFieldMaster::LocalizeFormula(*this, GetFormula(), true);
            rAny <<= OUString( sMyFormula );
        }
        break;
    case FIELD_PROP_DOUBLE:
        rAny <<= (double)GetValue();
        break;
    case FIELD_PROP_SUBTYPE:
        {
            sal_Int16 nRet = 0;
                nRet = lcl_SubTypeToAPI(GetSubType() & 0xff);
            rAny <<= nRet;
        }
        break;
    case FIELD_PROP_PAR3:
        rAny <<= OUString( aPText );
        break;
    case FIELD_PROP_BOOL3:
        rAny <<= 0 != (nSubType & nsSwExtendedSubType::SUB_CMD);
        break;
    case FIELD_PROP_BOOL1:
        rAny <<= GetInputFlag();
        break;
    case FIELD_PROP_PAR4:
        rAny <<= OUString(GetExpStr());
        break;
    default:
        return SwField::QueryValue(rAny, nWhichId);
    }
    return true;
}

SwInputFieldType::SwInputFieldType( SwDoc* pD )
    : SwFieldType( RES_INPUTFLD )
    , pDoc( pD )
{
}

SwFieldType* SwInputFieldType::Copy() const
{
    SwInputFieldType* pType = new SwInputFieldType( pDoc );
    return pType;
}

SwInputField::SwInputField( SwInputFieldType* pFieldType,
                            const OUString& rContent,
                            const OUString& rPrompt,
                            sal_uInt16 nSub,
                            sal_uLong nFormat,
                            bool bIsFormField )
    : SwField( pFieldType, nFormat, LANGUAGE_SYSTEM, false )
    , aContent(rContent)
    , aPText(rPrompt)
    , nSubType(nSub)
    , mbIsFormField( bIsFormField )
    , mpFormatField( nullptr )
{
}

SwInputField::~SwInputField()
{
}

void SwInputField::SetFormatField( SwFormatField& rFormatField )
{
    mpFormatField = &rFormatField;
}



void SwInputField::LockNotifyContentChange()
{
    if ( GetFormatField() != nullptr )
    {
        SwTextInputField* pTextInputField = dynamic_cast< SwTextInputField* >(GetFormatField()->GetTextField());
        if ( pTextInputField != nullptr )
        {
            pTextInputField->LockNotifyContentChange();
        }
    }
}

void SwInputField::UnlockNotifyContentChange()
{
    if ( GetFormatField() != nullptr )
    {
        SwTextInputField* pTextInputField = dynamic_cast< SwTextInputField* >(GetFormatField()->GetTextField());
        if ( pTextInputField != nullptr )
        {
            pTextInputField->UnlockNotifyContentChange();
        }
    }
}

void SwInputField::applyFieldContent( const OUString& rNewFieldContent )
{
    if ( (nSubType & 0x00ff) == INP_TXT )
    {
        aContent = rNewFieldContent;
    }
    else if( (nSubType & 0x00ff) == INP_USR )
    {
        SwUserFieldType* pUserTyp = static_cast<SwUserFieldType*>(
            static_cast<SwInputFieldType*>(GetTyp())->GetDoc()->getIDocumentFieldsAccess().GetFieldType( RES_USERFLD, getContent(), false ) );
        if( pUserTyp )
        {
            pUserTyp->SetContent( rNewFieldContent );

            // trigger update of the corresponding User Fields and other related Input Fields
            {
                LockNotifyContentChange();
                pUserTyp->UpdateFields();
                UnlockNotifyContentChange();
            }
        }
    }
}

OUString SwInputField::GetFieldName() const
{
    OUString aStr(SwField::GetFieldName());
    if ((nSubType & 0x00ff) == INP_USR)
    {
        aStr += GetTyp()->GetName() + " " + getContent();
    }
    return aStr;
}

SwField* SwInputField::Copy() const
{
    SwInputField* pField =
        new SwInputField(
            static_cast<SwInputFieldType*>(GetTyp()),
            getContent(),
            aPText,
            GetSubType(),
            GetFormat(),
            mbIsFormField );

    pField->SetHelp( aHelp );
    pField->SetToolTip( aToolTip );

    pField->SetAutomaticLanguage(IsAutomaticLanguage());
    return pField;
}

OUString SwInputField::Expand() const
{
    if((nSubType & 0x00ff) == INP_TXT)
    {
        return getContent();
    }

    if( (nSubType & 0x00ff) == INP_USR )
    {
        SwUserFieldType* pUserTyp = static_cast<SwUserFieldType*>(
            static_cast<SwInputFieldType*>(GetTyp())->GetDoc()->getIDocumentFieldsAccess().GetFieldType( RES_USERFLD, getContent(), false ) );
        if( pUserTyp )
            return pUserTyp->GetContent();
    }

    return OUString();
}

bool SwInputField::isFormField() const
{
    return mbIsFormField
           || !aHelp.isEmpty()
           || !aToolTip.isEmpty();
}

bool SwInputField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= getContent();
        break;
    case FIELD_PROP_PAR2:
        rAny <<= aPText;
        break;
    case FIELD_PROP_PAR3:
        rAny <<= aHelp;
        break;
    case FIELD_PROP_PAR4:
        rAny <<= aToolTip;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwInputField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= aContent;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= aPText;
        break;
    case FIELD_PROP_PAR3:
        rAny >>= aHelp;
        break;
    case FIELD_PROP_PAR4:
        rAny >>= aToolTip;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

/// set condition
void SwInputField::SetPar1(const OUString& rStr)
{
    aContent = rStr;
}

OUString SwInputField::GetPar1() const
{
    return getContent();
}

void SwInputField::SetPar2(const OUString& rStr)
{
    aPText = rStr;
}

OUString SwInputField::GetPar2() const
{
    return aPText;
}

void SwInputField::SetHelp(const OUString & rStr)
{
    aHelp = rStr;
}

OUString SwInputField::GetHelp() const
{
    return aHelp;
}

void SwInputField::SetToolTip(const OUString & rStr)
{
    aToolTip = rStr;
}

OUString SwInputField::GetToolTip() const
{
    return aToolTip;
}

sal_uInt16 SwInputField::GetSubType() const
{
    return nSubType;
}

void SwInputField::SetSubType(sal_uInt16 nSub)
{
    nSubType = nSub;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
