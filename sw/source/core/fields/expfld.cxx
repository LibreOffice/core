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
#include <switerator.hxx>

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

OUString ReplacePoint( OUString rTmpName, bool bWithCommandType )
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

SwTxtNode* GetFirstTxtNode( const SwDoc& rDoc, SwPosition& rPos,
                            const SwCntntFrm *pCFrm, Point &rPt )
{
    SwTxtNode* pTxtNode = 0;
    if ( !pCFrm )
    {
        const SwNodes& rNodes = rDoc.GetNodes();
        rPos.nNode = *rNodes.GetEndOfContent().StartOfSectionNode();
        SwCntntNode* pCNd;
        while( 0 != (pCNd = rNodes.GoNext( &rPos.nNode ) ) &&
                0 == ( pTxtNode = pCNd->GetTxtNode() ) )
                        ;
        OSL_ENSURE( pTxtNode, "Where is the 1.TextNode" );
        rPos.nContent.Assign( pTxtNode, 0 );
    }
    else if ( !pCFrm->IsValid() )
    {
        pTxtNode = (SwTxtNode*)pCFrm->GetNode();
        rPos.nNode = *pTxtNode;
        rPos.nContent.Assign( pTxtNode, 0 );
    }
    else
    {
        pCFrm->GetCrsrOfst( &rPos, rPt );
        pTxtNode = rPos.nNode.GetNode().GetTxtNode();
    }
    return pTxtNode;
}

const SwTxtNode* GetBodyTxtNode( const SwDoc& rDoc, SwPosition& rPos,
                                const SwFrm& rFrm )
{
    const SwLayoutFrm* pLayout = (SwLayoutFrm*)rFrm.GetUpper();
    const SwTxtNode* pTxtNode = 0;

    while( pLayout )
    {
        if( pLayout->IsFlyFrm() )
        {
            // hole das FlyFormat
            SwFrmFmt* pFlyFmt = ((SwFlyFrm*)pLayout)->GetFmt();
            OSL_ENSURE( pFlyFmt, "kein FlyFormat gefunden, wo steht das Feld" );

            const SwFmtAnchor &rAnchor = pFlyFmt->GetAnchor();

            if( FLY_AT_FLY == rAnchor.GetAnchorId() )
            {
                // the fly needs to be attached somewhere, so ask it
                pLayout = (SwLayoutFrm*)((SwFlyFrm*)pLayout)->GetAnchorFrm();
                continue;
            }
            else if ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
                     (FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
                     (FLY_AS_CHAR == rAnchor.GetAnchorId()))
            {
                OSL_ENSURE( rAnchor.GetCntntAnchor(), "no valid position" );
                rPos = *rAnchor.GetCntntAnchor();
                pTxtNode = rPos.nNode.GetNode().GetTxtNode();
                if ( FLY_AT_PARA == rAnchor.GetAnchorId() )
                {
                    const_cast<SwTxtNode*>(pTxtNode)->MakeStartIndex(
                            &rPos.nContent );
                }

                // do not break yet, might be as well in Header/Footer/Footnote/Fly
                pLayout = ((SwFlyFrm*)pLayout)->GetAnchorFrm()
                            ? ((SwFlyFrm*)pLayout)->GetAnchorFrm()->GetUpper() : 0;
                continue;
            }
            else
            {
                pLayout->FindPageFrm()->GetCntntPosition(
                                                pLayout->Frm().Pos(), rPos );
                pTxtNode = rPos.nNode.GetNode().GetTxtNode();
            }
        }
        else if( pLayout->IsFtnFrm() )
        {
            // get the anchor's node
            const SwTxtFtn* pFtn = ((SwFtnFrm*)pLayout)->GetAttr();
            pTxtNode = &pFtn->GetTxtNode();
            rPos.nNode = *pTxtNode;
            rPos.nContent = *pFtn->GetStart();
        }
        else if( pLayout->IsHeaderFrm() || pLayout->IsFooterFrm() )
        {
            const SwCntntFrm* pCntFrm;
            const SwPageFrm* pPgFrm = pLayout->FindPageFrm();
            if( pLayout->IsHeaderFrm() )
            {
                const SwTabFrm *pTab;
                if( 0 != ( pCntFrm = pPgFrm->FindFirstBodyCntnt()) &&
                    0 != (pTab = pCntFrm->FindTabFrm()) && pTab->IsFollow() &&
                    pTab->GetTable()->GetRowsToRepeat() > 0 &&
                    pTab->IsInHeadline( *pCntFrm ) )
                {
                    // take the next line
                    const SwLayoutFrm* pRow = pTab->GetFirstNonHeadlineRow();
                    pCntFrm = pRow->ContainsCntnt();
                }
            }
            else
                pCntFrm = pPgFrm->FindLastBodyCntnt();

            if( pCntFrm )
            {
                pTxtNode = pCntFrm->GetNode()->GetTxtNode();
                rPos.nNode = *pTxtNode;
                ((SwTxtNode*)pTxtNode)->MakeEndIndex( &rPos.nContent );
            }
            else
            {
                Point aPt( pLayout->Frm().Pos() );
                aPt.Y()++;      // aus dem Header raus
                pCntFrm = pPgFrm->GetCntntPos( aPt, sal_False, sal_True, sal_False );
                pTxtNode = GetFirstTxtNode( rDoc, rPos, pCntFrm, aPt );
            }
        }
        else
        {
            pLayout = pLayout->GetUpper();
            continue;
        }
        break; // found, so finish loop
    }
    return pTxtNode;
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
        NotifyClients( 0, pNew );
    // do not expand anything else
}

SwGetExpField::SwGetExpField(SwGetExpFieldType* pTyp, const OUString& rFormel,
                            sal_uInt16 nSub, sal_uLong nFmt)
    : SwFormulaField( pTyp, nFmt, 0.0 ),
    bIsInBodyTxt( sal_True ),
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
    SwGetExpField *pTmp = new SwGetExpField((SwGetExpFieldType*)GetTyp(),
                                            GetFormula(), nSubType, GetFormat());
    pTmp->SetLanguage(GetLanguage());
    pTmp->SwValueField::SetValue(GetValue());
    pTmp->sExpand       = sExpand;
    pTmp->bIsInBodyTxt  = bIsInBodyTxt;
    pTmp->SetAutomaticLanguage(IsAutomaticLanguage());
    if( bLateInitialization )
        pTmp->SetLateInitialization();

    return pTmp;
}

void SwGetExpField::ChangeExpansion( const SwFrm& rFrm, const SwTxtFld& rFld )
{
    if( bIsInBodyTxt ) // only fields in Footer, Header, FootNote, Flys
        return;

    OSL_ENSURE( !rFrm.IsInDocBody(), "Flag incorrect, frame is in DocBody" );

    // determine document (or is there an easier way?)
    const SwTxtNode* pTxtNode = &rFld.GetTxtNode();
    SwDoc& rDoc = *(SwDoc*)pTxtNode->GetDoc();

    // create index for determination of the TextNode
    SwPosition aPos( SwNodeIndex( rDoc.GetNodes() ) );
    pTxtNode = GetBodyTxtNode( rDoc, aPos, rFrm );

    // If no layout exists, ChangeExpansion is called for header and
    // footer lines via layout formatting without existing TxtNode.
    if(!pTxtNode)
        return;
    // #i82544#
    if( bLateInitialization )
    {
        SwFieldType* pSetExpFld = rDoc.GetFldType(RES_SETEXPFLD, GetFormula(), sal_False);
        if( pSetExpFld )
        {
            bLateInitialization = false;
            if( !(GetSubType() & nsSwGetSetExpType::GSE_STRING) &&
                static_cast< SwSetExpFieldType* >(pSetExpFld)->GetType() == nsSwGetSetExpType::GSE_STRING )
            SetSubType( nsSwGetSetExpType::GSE_STRING );
        }
    }

    _SetGetExpFld aEndFld( aPos.nNode, &rFld, &aPos.nContent );
    if(GetSubType() & nsSwGetSetExpType::GSE_STRING)
    {
        SwHash** ppHashTbl;
        sal_uInt16 nSize;
        rDoc.FldsToExpand( ppHashTbl, nSize, aEndFld );
        sExpand = LookString( ppHashTbl, nSize, GetFormula() );
        ::DeleteHashTable( ppHashTbl, nSize );
    }
    else
    {
        // fill calculator with values
        SwCalc aCalc( rDoc );
        rDoc.FldsToCalc(aCalc, aEndFld);

        // calculate value
        SetValue(aCalc.Calculate(GetFormula()).GetDouble());

        // analyse based on format
        sExpand = ((SwValueFieldType*)GetTyp())->ExpandValue(
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
        {
            sal_Bool bTmp = 0 != (nSubType & nsSwExtendedSubType::SUB_CMD);
            rAny.setValue(&bTmp, ::getBooleanCppuType());
        }
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
        SwValueField::SetValue(*(double*) rAny.getValue());
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
        if(*(sal_Bool*) rAny.getValue())
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
    pOutlChgNd( 0 ),
    sDelim( "." ),
    nType(nTyp), nLevel( UCHAR_MAX ),
    bDeleted( sal_False )
{
    if( ( nsSwGetSetExpType::GSE_SEQ | nsSwGetSetExpType::GSE_STRING ) & nType )
        EnableFormat(sal_False);    // do not use Numberformatter
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

void SwSetExpFieldType::SetSeqFormat(sal_uLong nFmt)
{
    SwIterator<SwFmtFld,SwFieldType> aIter(*this);
    for( SwFmtFld* pFld = aIter.First(); pFld; pFld = aIter.Next() )
        pFld->GetFld()->ChangeFormat( nFmt );
}

sal_uLong SwSetExpFieldType::GetSeqFormat()
{
    if( !GetDepends() )
        return SVX_NUM_ARABIC;

    SwField *pFld = ((SwFmtFld*)GetDepends())->GetFld();
    return pFld->GetFormat();
}

sal_uInt16 SwSetExpFieldType::SetSeqRefNo( SwSetExpField& rFld )
{
    if( !GetDepends() || !(nsSwGetSetExpType::GSE_SEQ & nType) )
        return USHRT_MAX;

extern void InsertSort( std::vector<sal_uInt16>& rArr, sal_uInt16 nIdx, sal_uInt16* pInsPos = 0 );
    std::vector<sal_uInt16> aArr;

    sal_uInt16 n;

    // check if number is already used and if a new one needs to be created
    SwIterator<SwFmtFld,SwFieldType> aIter( *this );
    const SwTxtNode* pNd;
    for( SwFmtFld* pF = aIter.First(); pF; pF = aIter.Next() )
        if( pF->GetFld() != &rFld && pF->GetTxtFld() &&
            0 != ( pNd = pF->GetTxtFld()->GetpTxtNode() ) &&
            pNd->GetNodes().IsDocNodes() )
            InsertSort( aArr, ((SwSetExpField*)pF->GetFld())->GetSeqNumber() );


    // check first if number already exists
    sal_uInt16 nNum = rFld.GetSeqNumber();
    if( USHRT_MAX != nNum )
    {
        for( n = 0; n < aArr.size(); ++n )
            if( aArr[ n ] > nNum )
                return nNum;            // no -> use it
            else if( aArr[ n ] == nNum )
                break;                  // yes -> create new

        if( n == aArr.size() )
            return nNum;            // no -> use it
    }

    // flagged all numbers, so determine the right number
    for( n = 0; n < aArr.size(); ++n )
        if( n != aArr[ n ] )
            break;

    rFld.SetSeqNumber( n );
    return n;
}

sal_uInt16 SwSetExpFieldType::GetSeqFldList( SwSeqFldList& rList )
{
    rList.Clear();

    SwIterator<SwFmtFld,SwFieldType> aIter( *this );
    const SwTxtNode* pNd;
    for( SwFmtFld* pF = aIter.First(); pF; pF = aIter.Next() )
        if( pF->GetTxtFld() &&
            0 != ( pNd = pF->GetTxtFld()->GetpTxtNode() ) &&
            pNd->GetNodes().IsDocNodes() )
        {
            _SeqFldLstElem* pNew = new _SeqFldLstElem(
                    pNd->GetExpandTxt( 0, USHRT_MAX ),
                    ((SwSetExpField*)pF->GetFld())->GetSeqNumber() );
            rList.InsertSort( pNew );
        }

    return rList.Count();
}

void SwSetExpFieldType::SetChapter( SwSetExpField& rFld, const SwNode& rNd )
{
    const SwTxtNode* pTxtNd = rNd.FindOutlineNodeOfLevel( nLevel );
    if( pTxtNd )
    {
        SwNumRule * pRule = pTxtNd->GetNumRule();

        if (pRule)
        {
            // --> OD 2005-11-02 #i51089 - TUNING#
            if ( pTxtNd->GetNum() )
            {
                const SwNodeNum & aNum = *(pTxtNd->GetNum());

                // nur die Nummer besorgen, ohne Pre-/Post-fixstrings
                OUString sNumber( pRule->MakeNumString(aNum, sal_False ));

                if( !sNumber.isEmpty() )
                    rFld.ChgExpStr( sNumber + sDelim + rFld.GetExpStr() );
            }
            else
            {
                OSL_FAIL( "<SwSetExpFieldType::SetChapter(..)> - text node with numbering rule, but without number. This is a serious defect -> inform OD" );
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

bool SwSeqFldList::InsertSort( _SeqFldLstElem* pNew )
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

    sal_uInt16 nPos;
    bool bRet = SeekEntry( *pNew, &nPos );
    if( !bRet )
        maData.insert( maData.begin() + nPos, pNew );
    return bRet;
}

bool SwSeqFldList::SeekEntry( const _SeqFldLstElem& rNew, sal_uInt16* pP ) const
{
    sal_uInt16 nO = maData.size(), nM, nU = 0;
    if( nO > 0 )
    {
        CollatorWrapper & rCaseColl = ::GetAppCaseCollator(),
                        & rColl = ::GetAppCollator();
        const CharClass& rCC = GetAppCharClass();

        //#59900# Sorting should sort number correctly (e.g. "10" after "9" not after "1")
        const OUString rTmp2 = rNew.sDlgEntry;
        sal_Int32 nFndPos2 = 0;
        const OUString sNum2( rTmp2.getToken( 0, ' ', nFndPos2 ));
        sal_Bool bIsNum2IsNumeric = rCC.isAsciiNumeric( sNum2 );
        sal_Int32 nNum2 = bIsNum2IsNumeric ? sNum2.toInt32() : 0;

        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;

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
                    nCmp = rCaseColl.compareString( rTmp2.copy( nFndPos2 ),
                                                    rTmp1.copy( nFndPos1 ));
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
                                        sal_uLong nFmt)
    : SwFormulaField( pTyp, nFmt, 0.0 ), nSeqNo( USHRT_MAX ),
    nSubType(0)
{
    SetFormula(rFormel);
    // ignore SubType
    bInput = sal_False;
    if( IsSequenceFld() )
    {
        SwValueField::SetValue(1.0);
        if( rFormel.isEmpty() )
        {
            SetFormula(pTyp->GetName() + "+1");
        }
    }
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
    SwFldTypesEnum const nStrType( (IsSequenceFld())
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
    SwSetExpField *pTmp = new SwSetExpField((SwSetExpFieldType*)GetTyp(),
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
    ((SwSetExpFieldType*)GetTyp())->SetType(nSub & 0xff);
    nSubType = nSub & 0xff00;

    OSL_ENSURE( (nSub & 0xff) != 3, "SubType ist illegal!" );
}

sal_uInt16 SwSetExpField::GetSubType() const
{
    return ((SwSetExpFieldType*)GetTyp())->GetType() | nSubType;
}

void SwSetExpField::SetValue( const double& rAny )
{
    SwValueField::SetValue(rAny);

    if( IsSequenceFld() )
        sExpand = FormatNumber( (sal_uInt32)GetValue(), GetFormat() );
    else
        sExpand = ((SwValueFieldType*)GetTyp())->ExpandValue( rAny,
                                                GetFormat(), GetLanguage());
}

void SwGetExpField::SetValue( const double& rAny )
{
    SwValueField::SetValue(rAny);
    sExpand = ((SwValueFieldType*)GetTyp())->ExpandValue( rAny, GetFormat(),
                                                            GetLanguage());
}

/** Find the index of the reference text following the current field
 *
 * @param rFmt
 * @param rDoc
 * @param nHint search starting position after the current field (or 0 if default)
 * @return
 */
xub_StrLen SwGetExpField::GetReferenceTextPos( const SwFmtFld& rFmt, SwDoc& rDoc, unsigned nHint)
{
    //
    const SwTxtFld* pTxtFld = rFmt.GetTxtFld();
    const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
    //
    xub_StrLen nRet = nHint ? nHint : *pTxtFld->GetStart() + 1;
    OUString sNodeText = rTxtNode.GetTxt();

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
        rTxtNode.GetAttr(aSet, nRet, nRet+1);

        if( RTL_TEXTENCODING_SYMBOL != ((SvxFontItem&)aSet.Get(
                GetWhichOfScript( RES_CHRATR_FONT, nSrcpt )) ).GetCharSet() )
        {
            LanguageType eLang = ((SvxLanguageItem&)aSet.Get(
                GetWhichOfScript( RES_CHRATR_LANGUAGE, nSrcpt )) ).GetLanguage();
            LanguageTag aLanguageTag( eLang);
            CharClass aCC( aLanguageTag);
            sal_Unicode c0 = sNodeText[0];
            sal_Bool bIsAlphaNum = aCC.isAlphaNumeric( sNodeText, 0 );
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
    return ((const SwSetExpFieldType*)GetTyp())->GetName();
}

OUString SwSetExpField::GetPar2() const
{
    sal_uInt16 nType = ((SwSetExpFieldType*)GetTyp())->GetType();

    if (nType & nsSwGetSetExpType::GSE_STRING)
        return GetFormula();
    return GetExpandedFormula();
}

void SwSetExpField::SetPar2(const OUString& rStr)
{
    sal_uInt16 nType = ((SwSetExpFieldType*)GetTyp())->GetType();

    if( !(nType & nsSwGetSetExpType::GSE_SEQ) || !rStr.isEmpty() )
    {
        if (nType & nsSwGetSetExpType::GSE_STRING)
            SetFormula(rStr);
        else
            SetExpandedFormula(rStr);
    }
}

SwInputFieldType::SwInputFieldType( SwDoc* pD )
    : SwFieldType( RES_INPUTFLD ), pDoc( pD )
{
}

SwFieldType* SwInputFieldType::Copy() const
{
    SwInputFieldType* pType = new SwInputFieldType( pDoc );
    return pType;
}

SwInputField::SwInputField(SwInputFieldType* pTyp, const OUString& rContent,
                           const OUString& rPrompt, sal_uInt16 nSub, sal_uLong nFmt) :
    SwField(pTyp, nFmt), aContent(rContent), aPText(rPrompt), nSubType(nSub)
{
}

OUString SwInputField::GetFieldName() const
{
    OUString aStr(SwField::GetFieldName());
    if ((nSubType & 0x00ff) == INP_USR)
    {
        aStr += GetTyp()->GetName() + " " + aContent;
    }
    return aStr;
}

SwField* SwInputField::Copy() const
{
    SwInputField* pFld = new SwInputField((SwInputFieldType*)GetTyp(), aContent,
                                          aPText, GetSubType(), GetFormat());

    pFld->SetHelp(aHelp);
    pFld->SetToolTip(aToolTip);

    pFld->SetAutomaticLanguage(IsAutomaticLanguage());
    return pFld;
}

OUString SwInputField::Expand() const
{
    if((nSubType & 0x00ff) == INP_TXT)
        return aContent;

    if( (nSubType & 0x00ff) == INP_USR )
    {
        SwUserFieldType* pUserTyp = (SwUserFieldType*)
                            ((SwInputFieldType*)GetTyp())->GetDoc()->
                            GetFldType( RES_USERFLD, aContent, false );
        if( pUserTyp )
            return pUserTyp->GetContent();
    }

    return OUString();
}

bool SwInputField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
         rAny <<= aContent;
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
    return aContent;
}

/// True/False Text
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

sal_Bool SwInputField::isFormField() const
{
    return !aHelp.isEmpty() || !aToolTip.isEmpty();
}

sal_uInt16 SwInputField::GetSubType() const
{
    return nSubType;
}

void SwInputField::SetSubType(sal_uInt16 nSub)
{
    nSubType = nSub;
}

bool SwSetExpField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL2:
        {
            sal_Bool bVal = 0 == (nSubType & nsSwExtendedSubType::SUB_INVISIBLE);
            rAny.setValue(&bVal, ::getBooleanCppuType());
        }
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
        //I18N - if the formula contains only "TypeName+1"
        //and it's one of the initially created sequence fields
        //then the localized names has to be replaced by a programmatic name
        rAny <<= SwXFieldMaster::LocalizeFormula(*this, GetFormula(), sal_True);
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
        rAny <<= aPText;
        break;
    case FIELD_PROP_BOOL3:
        {
            sal_Bool bTmp = 0 != (nSubType & nsSwExtendedSubType::SUB_CMD);
            rAny.setValue(&bTmp, ::getBooleanCppuType());
        }
        break;
    case FIELD_PROP_BOOL1:
        {
            sal_Bool bTmp = GetInputFlag();
            rAny.setValue(&bTmp, ::getBooleanCppuType());
        }
        break;
    case FIELD_PROP_PAR4:
        rAny <<= GetExpStr();
        break;
    default:
        return SwField::QueryValue(rAny, nWhichId);
    }
    return true;
}

bool SwSetExpField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    sal_Int32 nTmp32 = 0;
    sal_Int16 nTmp16 = 0;
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL2:
        if(*(sal_Bool*)rAny.getValue())
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
            SetFormula( SwXFieldMaster::LocalizeFormula(*this, uTmp, sal_False) );
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
        if(*(sal_Bool*) rAny.getValue())
            nSubType |= nsSwExtendedSubType::SUB_CMD;
        else
            nSubType &= (~nsSwExtendedSubType::SUB_CMD);
        break;
    case FIELD_PROP_BOOL1:
        SetInputFlag(*(sal_Bool*) rAny.getValue());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
