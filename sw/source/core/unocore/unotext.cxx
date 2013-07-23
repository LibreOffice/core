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

#include <stdlib.h>

#include <memory>
#include <iostream>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>

#include <cmdid.h>
#include <unotextbodyhf.hxx>
#include <unotext.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unosection.hxx>
#include <unobookmark.hxx>
#include <unorefmark.hxx>
#include <unoport.hxx>
#include <unotbl.hxx>
#include <unoidx.hxx>
#include <unocoll.hxx>
#include <unoframe.hxx>
#include <unofield.hxx>
#include <unometa.hxx>
#include <unodraw.hxx>
#include <unoredline.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unoparagraph.hxx>
#include <unocrsrhelper.hxx>
#include <docsh.hxx>
#include <docary.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <redline.hxx>
#include <swundo.hxx>
#include <section.hxx>
#include <IMark.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <crsskip.hxx>
#include <ndtxt.hxx>


using namespace ::com::sun::star;


const sal_Char cInvalidObject[] = "this object is invalid";

/******************************************************************
 * SwXText
 ******************************************************************/
class SwXText::Impl
{

public:
    SwXText &                   m_rThis;
    SfxItemPropertySet const&   m_rPropSet;
    const enum CursorType       m_eType;
    SwDoc *                     m_pDoc;
    bool                        m_bIsValid;

    Impl(   SwXText & rThis,
            SwDoc *const pDoc, const enum CursorType eType)
        : m_rThis(rThis)
        , m_rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT))
        , m_eType(eType)
        , m_pDoc(pDoc)
        , m_bIsValid(0 != pDoc)
    {
    }

    uno::Reference< text::XTextRange >
        finishOrAppendParagraph(
            const bool bFinish,
            const uno::Sequence< beans::PropertyValue >&
                rCharacterAndParagraphProperties,
            const uno::Reference< text::XTextRange >& xInsertPosition)
        throw (lang::IllegalArgumentException, uno::RuntimeException);

    sal_Int16 ComparePositions(
            const uno::Reference<text::XTextRange>& xPos1,
            const uno::Reference<text::XTextRange>& xPos2)
        throw (lang::IllegalArgumentException, uno::RuntimeException);

    bool CheckForOwnMember(const SwPaM & rPaM)
        throw (lang::IllegalArgumentException, uno::RuntimeException);

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    void ConvertCell(
            const bool bFirstCell,
            const uno::Sequence< uno::Reference< text::XTextRange > > & rCell,
            ::std::vector<SwNodeRange> & rRowNodes,
            ::std::auto_ptr< SwPaM > & rpFirstPaM,
            SwPaM & rLastPaM,
            bool & rbExcept);
    SAL_WNODEPRECATED_DECLARATIONS_POP

};

SwXText::SwXText(SwDoc *const pDoc, const enum CursorType eType)
    : m_pImpl( new SwXText::Impl(*this, pDoc, eType) )
{
}

SwXText::~SwXText()
{
}

const SwDoc * SwXText::GetDoc() const
{
    return m_pImpl->m_pDoc;
}
      SwDoc * SwXText::GetDoc()
{
    return m_pImpl->m_pDoc;
}

bool SwXText::IsValid() const
{
    return m_pImpl->m_bIsValid;
}

void SwXText::Invalidate()
{
    m_pImpl->m_bIsValid = false;
}

void SwXText::SetDoc(SwDoc *const pDoc)
{
    OSL_ENSURE(!m_pImpl->m_pDoc || !pDoc,
        "SwXText::SetDoc: already have a doc?");
    m_pImpl->m_pDoc = pDoc;
    m_pImpl->m_bIsValid = (0 != pDoc);
}

void
SwXText::PrepareForAttach(uno::Reference< text::XTextRange > &, const SwPaM &)
{
}

bool SwXText::CheckForOwnMemberMeta(const SwPaM &, const bool)
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    OSL_ENSURE(CURSOR_META != m_pImpl->m_eType, "should not be called!");
    return false;
}

const SwStartNode *SwXText::GetStartNode() const
{
    return GetDoc()->GetNodes().GetEndOfContent().StartOfSectionNode();
}

uno::Reference< text::XTextCursor >
SwXText::CreateCursor() throw (uno::RuntimeException)
{
    uno::Reference< text::XTextCursor >  xRet;
    if(IsValid())
    {
        SwNode& rNode = GetDoc()->GetNodes().GetEndOfContent();
        SwPosition aPos(rNode);
        xRet = static_cast<text::XWordCursor*>(
                new SwXTextCursor(*GetDoc(), this, m_pImpl->m_eType, aPos));
        xRet->gotoStart(sal_False);
    }
    return xRet;
}

uno::Any SAL_CALL
SwXText::queryInterface(const uno::Type& rType) throw (uno::RuntimeException)
{
    uno::Any aRet;
    if (rType == text::XText::static_type())
    {
        aRet <<= uno::Reference< text::XText >(this);
    }
    else if (rType == text::XSimpleText::static_type())
    {
        aRet <<= uno::Reference< text::XSimpleText >(this);
    }
    else if (rType == text::XTextRange::static_type())
    {
        aRet <<= uno::Reference< text::XTextRange>(this);
    }
    else if (rType == text::XTextRangeCompare::static_type())
    {
        aRet <<= uno::Reference< text::XTextRangeCompare >(this);
    }
    else if (rType == lang::XTypeProvider::static_type())
    {
        aRet <<= uno::Reference< lang::XTypeProvider >(this);
    }
    else if (rType == text::XRelativeTextContentInsert::static_type())
    {
        aRet <<= uno::Reference< text::XRelativeTextContentInsert >(this);
    }
    else if (rType == text::XRelativeTextContentRemove::static_type())
    {
        aRet <<= uno::Reference< text::XRelativeTextContentRemove >(this);
    }
    else if (rType == beans::XPropertySet::static_type())
    {
        aRet <<= uno::Reference< beans::XPropertySet >(this);
    }
    else if (rType == lang::XUnoTunnel::static_type())
    {
        aRet <<= uno::Reference< lang::XUnoTunnel >(this);
    }
    else if (rType == text::XTextAppendAndConvert::static_type())
    {
        aRet <<= uno::Reference< text::XTextAppendAndConvert >(this);
    }
    else if (rType == text::XTextAppend::static_type())
    {
        aRet <<= uno::Reference< text::XTextAppend >(this);
    }
    else if (rType == text::XTextPortionAppend::static_type())
    {
        aRet <<= uno::Reference< text::XTextPortionAppend >(this);
    }
    else if (rType == text::XParagraphAppend::static_type())
    {
        aRet <<= uno::Reference< text::XParagraphAppend >(this);
    }
    else if (rType == text::XTextConvert::static_type() )
    {
        aRet <<= uno::Reference< text::XTextConvert >(this);
    }
    else if (rType == text::XTextContentAppend::static_type())
    {
        aRet <<= uno::Reference< text::XTextContentAppend >(this);
    }
    else if(rType == text::XTextCopy::static_type())
    {
        aRet <<= uno::Reference< text::XTextCopy >( this );
    }
    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL
SwXText::getTypes() throw (uno::RuntimeException)
{
    uno::Sequence< uno::Type > aRet(12);
    uno::Type* pTypes = aRet.getArray();
    pTypes[0] = text::XText::static_type();
    pTypes[1] = text::XTextRangeCompare::static_type();
    pTypes[2] = text::XRelativeTextContentInsert::static_type();
    pTypes[3] = text::XRelativeTextContentRemove::static_type();
    pTypes[4] = lang::XUnoTunnel::static_type();
    pTypes[5] = beans::XPropertySet::static_type();
    pTypes[6] = text::XTextPortionAppend::static_type();
    pTypes[7] = text::XParagraphAppend::static_type();
    pTypes[8] = text::XTextContentAppend::static_type();
    pTypes[9] = text::XTextConvert::static_type();
    pTypes[10] = text::XTextAppend::static_type();
    pTypes[11] = text::XTextAppendAndConvert::static_type();

    return aRet;
}

// belongs the range in the text ? insert it then.
void SAL_CALL
SwXText::insertString(const uno::Reference< text::XTextRange >& xTextRange,
    const OUString& rString, sal_Bool bAbsorb)
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!xTextRange.is())
    {
        throw uno::RuntimeException();
    }
    if (!GetDoc())
    {
        throw uno::RuntimeException();
    }
    const uno::Reference<lang::XUnoTunnel> xRangeTunnel(xTextRange,
            uno::UNO_QUERY);
    SwXTextRange *const pRange =
        ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
    OTextCursorHelper *const pCursor =
        ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);
    if ((!pRange  || pRange ->GetDoc() != GetDoc()) &&
        (!pCursor || pCursor->GetDoc() != GetDoc()))
    {
        throw uno::RuntimeException();
    }

    const SwStartNode *const pOwnStartNode = GetStartNode();
    SwPaM aPam(GetDoc()->GetNodes());
    const SwPaM * pPam(0);
    if (pCursor)
    {
        pPam = pCursor->GetPaM();
    }
    else // pRange
    {
        if (pRange->GetPositions(aPam))
        {
            pPam = &aPam;
        }
    }
    if (!pPam)
    {
        throw uno::RuntimeException();
    }

    const SwStartNode* pTmp(pPam->GetNode()->StartOfSectionNode());
    while (pTmp && pTmp->IsSectionNode())
    {
        pTmp = pTmp->StartOfSectionNode();
    }
    if (!pOwnStartNode || (pOwnStartNode != pTmp))
    {
        throw uno::RuntimeException();
    }

    bool bForceExpandHints( false );
    if (CURSOR_META == m_pImpl->m_eType)
    {
        try
        {
            bForceExpandHints = CheckForOwnMemberMeta(*pPam, bAbsorb);
        }
        catch (const lang::IllegalArgumentException& iae)
        {
            // stupid method not allowed to throw iae
            throw uno::RuntimeException(iae.Message, 0);
        }
    }
    if (bAbsorb)
    {
        //!! scan for CR characters and inserting the paragraph breaks
        //!! has to be done in the called function.
        //!! Implemented in SwXTextRange::DeleteAndInsert
        if (pCursor)
        {
            SwXTextCursor * const pTextCursor(
                dynamic_cast<SwXTextCursor*>(pCursor) );
            if (pTextCursor)
            {
                pTextCursor->DeleteAndInsert(rString, bForceExpandHints);
            }
            else
            {
                xTextRange->setString(rString);
            }
        }
        else
        {
            pRange->DeleteAndInsert(rString, bForceExpandHints);
        }
    }
    else
    {
        // create a PaM positioned before the parameter PaM,
        // so the text is inserted before
        UnoActionContext aContext(GetDoc());
        SwPaM aInsertPam(*pPam->Start());
        ::sw::GroupUndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());
        SwUnoCursorHelper::DocInsertStringSplitCR(
            *GetDoc(), aInsertPam, rString, bForceExpandHints );
    }
}

void SAL_CALL
SwXText::insertControlCharacter(
        const uno::Reference< text::XTextRange > & xTextRange,
        sal_Int16 nControlCharacter, sal_Bool bAbsorb)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!xTextRange.is())
    {
        throw lang::IllegalArgumentException();
    }
    if (!GetDoc())
    {
        throw uno::RuntimeException();
    }

    SwUnoInternalPaM aPam(*GetDoc());
    if (!::sw::XTextRangeToSwPaM(aPam, xTextRange))
    {
        throw uno::RuntimeException();
    }
    const bool bForceExpandHints(CheckForOwnMemberMeta(aPam, bAbsorb));

    const enum IDocumentContentOperations::InsertFlags nInsertFlags =
        (bForceExpandHints)
        ? static_cast<IDocumentContentOperations::InsertFlags>(
                IDocumentContentOperations::INS_FORCEHINTEXPAND |
                IDocumentContentOperations::INS_EMPTYEXPAND)
        : IDocumentContentOperations::INS_EMPTYEXPAND;

    SwPaM aTmp(*aPam.Start());
    if (bAbsorb && aPam.HasMark())
    {
        m_pImpl->m_pDoc->DeleteAndJoin(aPam);
    }

    sal_Unicode cIns = 0;
    switch (nControlCharacter)
    {
        case text::ControlCharacter::PARAGRAPH_BREAK :
            // a table cell now becomes an ordinary text cell!
            m_pImpl->m_pDoc->ClearBoxNumAttrs( aTmp.GetPoint()->nNode );
            m_pImpl->m_pDoc->SplitNode( *aTmp.GetPoint(), sal_False );
            break;
        case text::ControlCharacter::APPEND_PARAGRAPH:
        {
            m_pImpl->m_pDoc->ClearBoxNumAttrs( aTmp.GetPoint()->nNode );
            m_pImpl->m_pDoc->AppendTxtNode( *aTmp.GetPoint() );

            const uno::Reference<lang::XUnoTunnel> xRangeTunnel(
                    xTextRange, uno::UNO_QUERY);
            SwXTextRange *const pRange =
                ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
            OTextCursorHelper *const pCursor =
                ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(
                            xRangeTunnel);
            if (pRange)
            {
                pRange->SetPositions(aTmp);
            }
            else if (pCursor)
            {
                SwPaM *const pCrsr = pCursor->GetPaM();
                *pCrsr->GetPoint() = *aTmp.GetPoint();
                pCrsr->DeleteMark();
            }
        }
        break;
        case text::ControlCharacter::LINE_BREAK:  cIns = 10;              break;
        case text::ControlCharacter::SOFT_HYPHEN: cIns = CHAR_SOFTHYPHEN; break;
        case text::ControlCharacter::HARD_HYPHEN: cIns = CHAR_HARDHYPHEN; break;
        case text::ControlCharacter::HARD_SPACE:  cIns = CHAR_HARDBLANK;  break;
    }
    if (cIns)
    {
        m_pImpl->m_pDoc->InsertString( aTmp, OUString(cIns), nInsertFlags );
    }

    if (bAbsorb)
    {
        const uno::Reference<lang::XUnoTunnel> xRangeTunnel(
                xTextRange, uno::UNO_QUERY);
        SwXTextRange *const pRange =
            ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
        OTextCursorHelper *const pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);

        SwCursor aCrsr(*aTmp.GetPoint(),0,false);
        SwUnoCursorHelper::SelectPam(aCrsr, true);
        aCrsr.Left(1, CRSR_SKIP_CHARS, sal_False, sal_False);
        //hier muss der uebergebene PaM umgesetzt werden:
        if (pRange)
        {
            pRange->SetPositions(aCrsr);
        }
        else
        {
            SwPaM *const pUnoCrsr = pCursor->GetPaM();
            *pUnoCrsr->GetPoint() = *aCrsr.GetPoint();
            if (aCrsr.HasMark())
            {
                pUnoCrsr->SetMark();
                *pUnoCrsr->GetMark() = *aCrsr.GetMark();
            }
            else
            {
                pUnoCrsr->DeleteMark();
            }
        }
    }
}

void SAL_CALL
SwXText::insertTextContent(
        const uno::Reference< text::XTextRange > & xRange,
        const uno::Reference< text::XTextContent > & xContent,
        sal_Bool bAbsorb)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!xRange.is())
    {
        lang::IllegalArgumentException aIllegal;
        aIllegal.Message = "first parameter invalid;";
        throw aIllegal;
    }
    if (!xContent.is())
    {
        lang::IllegalArgumentException aIllegal;
        aIllegal.Message += "second parameter invalid";
        throw aIllegal;
    }
    if(!GetDoc())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }

    SwUnoInternalPaM aPam(*GetDoc());
    if (!::sw::XTextRangeToSwPaM(aPam, xRange))
    {
        lang::IllegalArgumentException aIllegal;
        aIllegal.Message = "first parameter invalid";
        throw aIllegal;
    }
    // first test if the range is at the right position, then call
    // xContent->attach
    const SwStartNode* pOwnStartNode = GetStartNode();
    SwStartNodeType eSearchNodeType = SwNormalStartNode;
    switch (m_pImpl->m_eType)
    {
        case CURSOR_FRAME:      eSearchNodeType = SwFlyStartNode;       break;
        case CURSOR_TBLTEXT:    eSearchNodeType = SwTableBoxStartNode;  break;
        case CURSOR_FOOTNOTE:   eSearchNodeType = SwFootnoteStartNode;  break;
        case CURSOR_HEADER:     eSearchNodeType = SwHeaderStartNode;    break;
        case CURSOR_FOOTER:     eSearchNodeType = SwFooterStartNode;    break;
        //case CURSOR_INVALID:
        //case CURSOR_BODY:
        default:
            break;
    }

    const SwStartNode* pTmp =
        aPam.GetNode()->FindSttNodeByType(eSearchNodeType);

    // ignore SectionNodes
    while (pTmp && pTmp->IsSectionNode())
    {
        pTmp = pTmp->StartOfSectionNode();
    }
    // if the document starts with a section
    while (pOwnStartNode->IsSectionNode())
    {
        pOwnStartNode = pOwnStartNode->StartOfSectionNode();
    }
    // this checks if (this) and xRange are in the same text::XText interface
    if (pOwnStartNode != pTmp)
    {
        uno::RuntimeException aRunException;
        aRunException.Message = "text interface and cursor not related";
        throw aRunException;
    }

    const bool bForceExpandHints(CheckForOwnMemberMeta(aPam, bAbsorb));

    // special treatment for Contents that do not replace the range, but
    // instead are "overlaid"
    const uno::Reference<lang::XUnoTunnel> xContentTunnel(xContent,
            uno::UNO_QUERY);
    if (!xContentTunnel.is())
    {
        lang::IllegalArgumentException aArgException;
        aArgException.Message = "text content does not support lang::XUnoTunnel";
        throw aArgException;
    }
    SwXDocumentIndexMark *const pDocumentIndexMark =
        ::sw::UnoTunnelGetImplementation<SwXDocumentIndexMark>(xContentTunnel);
    SwXTextSection *const pSection =
        ::sw::UnoTunnelGetImplementation<SwXTextSection>(xContentTunnel);
    SwXBookmark *const pBookmark =
        ::sw::UnoTunnelGetImplementation<SwXBookmark>(xContentTunnel);
    SwXReferenceMark *const pReferenceMark =
        ::sw::UnoTunnelGetImplementation<SwXReferenceMark>(xContentTunnel);
    SwXMeta *const pMeta =
        ::sw::UnoTunnelGetImplementation<SwXMeta>(xContentTunnel);
    SwXTextField* pTextField =
        ::sw::UnoTunnelGetImplementation<SwXTextField>(xContentTunnel);
    if (pTextField && pTextField->GetServiceId() != SW_SERVICE_FIELDTYPE_ANNOTATION)
        pTextField = 0;

    const bool bAttribute = pBookmark || pDocumentIndexMark
        || pSection || pReferenceMark || pMeta || pTextField;

    if (bAbsorb && !bAttribute)
    {
        xRange->setString(aEmptyStr);
    }
    uno::Reference< text::XTextRange > xTempRange =
        (bAttribute && bAbsorb) ? xRange : xRange->getStart();
    if (bForceExpandHints)
    {
        // if necessary, replace xTempRange with a new SwXTextCursor
        PrepareForAttach(xTempRange, aPam);
    }
    xContent->attach(xTempRange);
}

void SAL_CALL
SwXText::insertTextContentBefore(
    const uno::Reference< text::XTextContent>& xNewContent,
    const uno::Reference< text::XTextContent>& xSuccessor)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!GetDoc())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }

    const uno::Reference<lang::XUnoTunnel> xParaTunnel(xNewContent,
            uno::UNO_QUERY);
    SwXParagraph *const pPara =
            ::sw::UnoTunnelGetImplementation<SwXParagraph>(xParaTunnel);
    if (!pPara || !pPara->IsDescriptor() || !xSuccessor.is())
    {
        throw lang::IllegalArgumentException();
    }

    sal_Bool bRet = sal_False;
    const uno::Reference<lang::XUnoTunnel> xSuccTunnel(xSuccessor,
            uno::UNO_QUERY);
    SwXTextSection *const pXSection =
            ::sw::UnoTunnelGetImplementation<SwXTextSection>(xSuccTunnel);
    SwXTextTable *const pXTable =
            ::sw::UnoTunnelGetImplementation<SwXTextTable>(xSuccTunnel);
    SwFrmFmt *const pTableFmt = (pXTable) ? pXTable->GetFrmFmt() : 0;
    SwTxtNode * pTxtNode = 0;
    if(pTableFmt && pTableFmt->GetDoc() == GetDoc())
    {
        SwTable *const pTable = SwTable::FindTable( pTableFmt );
        SwTableNode *const pTblNode = pTable->GetTableNode();

        const SwNodeIndex aTblIdx( *pTblNode, -1 );
        SwPosition aBefore(aTblIdx);
        bRet = GetDoc()->AppendTxtNode( aBefore );
        pTxtNode = aBefore.nNode.GetNode().GetTxtNode();
    }
    else if (pXSection && pXSection->GetFmt() &&
            pXSection->GetFmt()->GetDoc() == GetDoc())
    {
        SwSectionFmt *const pSectFmt = pXSection->GetFmt();
        SwSectionNode *const pSectNode = pSectFmt->GetSectionNode();

        const SwNodeIndex aSectIdx( *pSectNode, -1 );
        SwPosition aBefore(aSectIdx);
        bRet = GetDoc()->AppendTxtNode( aBefore );
        pTxtNode = aBefore.nNode.GetNode().GetTxtNode();
    }
    if (!bRet || !pTxtNode)
    {
        throw lang::IllegalArgumentException();
    }
    pPara->attachToText(*this, *pTxtNode);
}

void SAL_CALL
SwXText::insertTextContentAfter(
    const uno::Reference< text::XTextContent>& xNewContent,
    const uno::Reference< text::XTextContent>& xPredecessor)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!GetDoc())
    {
        throw uno::RuntimeException();
    }

    const uno::Reference<lang::XUnoTunnel> xParaTunnel(xNewContent,
            uno::UNO_QUERY);
    SwXParagraph *const pPara =
            ::sw::UnoTunnelGetImplementation<SwXParagraph>(xParaTunnel);
    if(!pPara || !pPara->IsDescriptor() || !xPredecessor.is())
    {
        throw lang::IllegalArgumentException();
    }

    const uno::Reference<lang::XUnoTunnel> xPredTunnel(xPredecessor,
            uno::UNO_QUERY);
    SwXTextSection *const pXSection =
            ::sw::UnoTunnelGetImplementation<SwXTextSection>(xPredTunnel);
    SwXTextTable *const pXTable =
            ::sw::UnoTunnelGetImplementation<SwXTextTable>(xPredTunnel);
    SwFrmFmt *const pTableFmt = (pXTable) ? pXTable->GetFrmFmt() : 0;
    sal_Bool bRet = sal_False;
    SwTxtNode * pTxtNode = 0;
    if(pTableFmt && pTableFmt->GetDoc() == GetDoc())
    {
        SwTable *const pTable = SwTable::FindTable( pTableFmt );
        SwTableNode *const pTblNode = pTable->GetTableNode();

        SwEndNode *const pTableEnd = pTblNode->EndOfSectionNode();
        SwPosition aTableEnd(*pTableEnd);
        bRet = GetDoc()->AppendTxtNode( aTableEnd );
        pTxtNode = aTableEnd.nNode.GetNode().GetTxtNode();
    }
    else if (pXSection && pXSection->GetFmt() &&
            pXSection->GetFmt()->GetDoc() == GetDoc())
    {
        SwSectionFmt *const pSectFmt = pXSection->GetFmt();
        SwSectionNode *const pSectNode = pSectFmt->GetSectionNode();
        SwEndNode *const pEnd = pSectNode->EndOfSectionNode();
        SwPosition aEnd(*pEnd);
        bRet = GetDoc()->AppendTxtNode( aEnd );
        pTxtNode = aEnd.nNode.GetNode().GetTxtNode();
    }
    if (!bRet || !pTxtNode)
    {
        throw lang::IllegalArgumentException();
    }
    pPara->attachToText(*this, *pTxtNode);
}

void SAL_CALL
SwXText::removeTextContentBefore(
    const uno::Reference< text::XTextContent>& xSuccessor)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!GetDoc())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }

    sal_Bool bRet = sal_False;
    const uno::Reference<lang::XUnoTunnel> xSuccTunnel(xSuccessor,
            uno::UNO_QUERY);
    SwXTextSection *const pXSection =
            ::sw::UnoTunnelGetImplementation<SwXTextSection>(xSuccTunnel);
    SwXTextTable *const pXTable =
            ::sw::UnoTunnelGetImplementation<SwXTextTable>(xSuccTunnel);
    SwFrmFmt *const pTableFmt = (pXTable) ? pXTable->GetFrmFmt() : 0;
    if(pTableFmt && pTableFmt->GetDoc() == GetDoc())
    {
        SwTable *const pTable = SwTable::FindTable( pTableFmt );
        SwTableNode *const pTblNode = pTable->GetTableNode();

        const SwNodeIndex aTblIdx( *pTblNode, -1 );
        if(aTblIdx.GetNode().IsTxtNode())
        {
            SwPaM aBefore(aTblIdx);
            bRet = GetDoc()->DelFullPara( aBefore );
        }
    }
    else if (pXSection && pXSection->GetFmt() &&
            pXSection->GetFmt()->GetDoc() == GetDoc())
    {
        SwSectionFmt *const pSectFmt = pXSection->GetFmt();
        SwSectionNode *const pSectNode = pSectFmt->GetSectionNode();

        const SwNodeIndex aSectIdx(  *pSectNode, -1 );
        if(aSectIdx.GetNode().IsTxtNode())
        {
            SwPaM aBefore(aSectIdx);
            bRet = GetDoc()->DelFullPara( aBefore );
        }
    }
    if(!bRet)
    {
        throw lang::IllegalArgumentException();
    }
}

void SAL_CALL
SwXText::removeTextContentAfter(
        const uno::Reference< text::XTextContent>& xPredecessor)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!GetDoc())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }

    sal_Bool bRet = sal_False;
    const uno::Reference<lang::XUnoTunnel> xPredTunnel(xPredecessor,
            uno::UNO_QUERY);
    SwXTextSection *const pXSection =
            ::sw::UnoTunnelGetImplementation<SwXTextSection>(xPredTunnel);
    SwXTextTable *const pXTable =
            ::sw::UnoTunnelGetImplementation<SwXTextTable>(xPredTunnel);
    SwFrmFmt *const pTableFmt = (pXTable) ? pXTable->GetFrmFmt() : 0;
    if(pTableFmt && pTableFmt->GetDoc() == GetDoc())
    {
        SwTable *const pTable = SwTable::FindTable( pTableFmt );
        SwTableNode *const pTblNode = pTable->GetTableNode();
        SwEndNode *const pTableEnd = pTblNode->EndOfSectionNode();

        const SwNodeIndex aTblIdx( *pTableEnd, 1 );
        if(aTblIdx.GetNode().IsTxtNode())
        {
            SwPaM aPaM(aTblIdx);
            bRet = GetDoc()->DelFullPara( aPaM );
        }
    }
    else if (pXSection && pXSection->GetFmt() &&
            pXSection->GetFmt()->GetDoc() == GetDoc())
    {
        SwSectionFmt *const pSectFmt = pXSection->GetFmt();
        SwSectionNode *const pSectNode = pSectFmt->GetSectionNode();
        SwEndNode *const pEnd = pSectNode->EndOfSectionNode();
        const SwNodeIndex aSectIdx(  *pEnd, 1 );
        if(aSectIdx.GetNode().IsTxtNode())
        {
            SwPaM aAfter(aSectIdx);
            bRet = GetDoc()->DelFullPara( aAfter );
        }
    }
    if(!bRet)
    {
        throw lang::IllegalArgumentException();
    }
}

void SAL_CALL
SwXText::removeTextContent(
        const uno::Reference< text::XTextContent > & xContent)
throw (container::NoSuchElementException, uno::RuntimeException)
{
    // forward: need no solar mutex here
    if(!xContent.is())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "first parameter invalid";
        throw aRuntime;
    }
    xContent->dispose();
}

uno::Reference< text::XText > SAL_CALL
SwXText::getText() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const uno::Reference< text::XText > xRet(this);
    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::getStart() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const uno::Reference< text::XTextCursor > xRef = CreateCursor();
    if(!xRef.is())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }
    xRef->gotoStart(sal_False);
    const uno::Reference< text::XTextRange > xRet(xRef, uno::UNO_QUERY);
    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::getEnd() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const uno::Reference< text::XTextCursor > xRef = CreateCursor();
    if(!xRef.is())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }
    xRef->gotoEnd(sal_False);
    const uno::Reference< text::XTextRange >  xRet(xRef, uno::UNO_QUERY);
    return xRet;
}

OUString SAL_CALL SwXText::getString() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const uno::Reference< text::XTextCursor > xRet = CreateCursor();
    if(!xRet.is())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }
    xRet->gotoEnd(sal_True);
    return xRet->getString();
}

void SAL_CALL
SwXText::setString(const OUString& rString) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!GetDoc())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }

    const SwStartNode* pStartNode = GetStartNode();
    if (!pStartNode)
    {
        throw uno::RuntimeException();
    }

    GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_START, NULL);
    //insert an empty paragraph at the start and at the end to ensure that
    //all tables and sections can be removed by the selecting text::XTextCursor
    if (CURSOR_META != m_pImpl->m_eType)
    {
        SwPosition aStartPos(*pStartNode);
        const SwEndNode* pEnd = pStartNode->EndOfSectionNode();
        SwNodeIndex aEndIdx(*pEnd);
        aEndIdx--;
        //the inserting of nodes should only be done if really necessary
        //to prevent #97924# (removes paragraph attributes when setting the text
        //e.g. of a table cell
        bool bInsertNodes = false;
        SwNodeIndex aStartIdx(*pStartNode);
        do
        {
            ++aStartIdx;
            SwNode& rCurrentNode = aStartIdx.GetNode();
            if(rCurrentNode.GetNodeType() == ND_SECTIONNODE
                ||rCurrentNode.GetNodeType() == ND_TABLENODE)
            {
                bInsertNodes = true;
                break;
            }
        }
        while(aStartIdx < aEndIdx);
        if(bInsertNodes)
        {
            GetDoc()->AppendTxtNode( aStartPos );
            SwPosition aEndPos(aEndIdx.GetNode());
            SwPaM aPam(aEndPos);
            GetDoc()->AppendTxtNode( *aPam.Start() );
        }
    }

    const uno::Reference< text::XTextCursor > xRet = CreateCursor();
    if(!xRet.is())
    {
        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_END, NULL);
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }
    xRet->gotoEnd(sal_True);
    xRet->setString(rString);
    GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_END, NULL);
}

//FIXME why is CheckForOwnMember duplicated in some insert methods?
//  Description: Checks if pRange/pCursor are member of the same text interface.
//              Only one of the pointers has to be set!
bool SwXText::Impl::CheckForOwnMember(
    const SwPaM & rPaM)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    const uno::Reference<text::XTextCursor> xOwnCursor(m_rThis.CreateCursor());

    const uno::Reference<lang::XUnoTunnel> xTunnel(xOwnCursor, uno::UNO_QUERY);
    OTextCursorHelper *const pOwnCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xTunnel);
    OSL_ENSURE(pOwnCursor, "OTextCursorHelper::getUnoTunnelId() ??? ");
    const SwStartNode* pOwnStartNode =
        pOwnCursor->GetPaM()->GetNode()->StartOfSectionNode();
    SwStartNodeType eSearchNodeType = SwNormalStartNode;
    switch (m_eType)
    {
        case CURSOR_FRAME:      eSearchNodeType = SwFlyStartNode;       break;
        case CURSOR_TBLTEXT:    eSearchNodeType = SwTableBoxStartNode;  break;
        case CURSOR_FOOTNOTE:   eSearchNodeType = SwFootnoteStartNode;  break;
        case CURSOR_HEADER:     eSearchNodeType = SwHeaderStartNode;    break;
        case CURSOR_FOOTER:     eSearchNodeType = SwFooterStartNode;    break;
        //case CURSOR_INVALID:
        //case CURSOR_BODY:
        default:
            ;
    }

    SwNode const*const pSrcNode(rPaM.GetNode());
    if (!pSrcNode) { return false; }
    const SwStartNode* pTmp = pSrcNode->FindSttNodeByType(eSearchNodeType);

    //SectionNodes ueberspringen
    while(pTmp && pTmp->IsSectionNode())
    {
        pTmp = pTmp->StartOfSectionNode();
    }

    //if the document starts with a section
    while(pOwnStartNode->IsSectionNode())
    {
        pOwnStartNode = pOwnStartNode->StartOfSectionNode();
    }

    //this checks if (this) and xRange are in the same text::XText interface
    return (pOwnStartNode == pTmp);
}

sal_Int16
SwXText::Impl::ComparePositions(
    const uno::Reference<text::XTextRange>& xPos1,
    const uno::Reference<text::XTextRange>& xPos2)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SwUnoInternalPaM aPam1(*m_pDoc);
    SwUnoInternalPaM aPam2(*m_pDoc);

    if (!::sw::XTextRangeToSwPaM(aPam1, xPos1) ||
        !::sw::XTextRangeToSwPaM(aPam2, xPos2))
    {
        throw lang::IllegalArgumentException();
    }
    if (!CheckForOwnMember(aPam1) || !CheckForOwnMember(aPam2))
    {
        throw lang::IllegalArgumentException();
    }

    sal_Int16 nCompare = 0;
    SwPosition const*const pStart1 = aPam1.Start();
    SwPosition const*const pStart2 = aPam2.Start();
    if (*pStart1 < *pStart2)
    {
        nCompare = 1;
    }
    else if (*pStart1 > *pStart2)
    {
        nCompare = -1;
    }
    else
    {
        OSL_ENSURE(*pStart1 == *pStart2,
                "SwPositions should be equal here");
        nCompare = 0;
    }

    return nCompare;
}

sal_Int16 SAL_CALL
SwXText::compareRegionStarts(
    const uno::Reference<text::XTextRange>& xRange1,
    const uno::Reference<text::XTextRange>& xRange2)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!xRange1.is() || !xRange2.is())
    {
        throw lang::IllegalArgumentException();
    }
    const uno::Reference<text::XTextRange> xStart1 = xRange1->getStart();
    const uno::Reference<text::XTextRange> xStart2 = xRange2->getStart();

    return m_pImpl->ComparePositions(xStart1, xStart2);
}

sal_Int16 SAL_CALL
SwXText::compareRegionEnds(
    const uno::Reference<text::XTextRange>& xRange1,
    const uno::Reference<text::XTextRange>& xRange2)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!xRange1.is() || !xRange2.is())
    {
        throw lang::IllegalArgumentException();
    }
    uno::Reference<text::XTextRange> xEnd1 = xRange1->getEnd();
    uno::Reference<text::XTextRange> xEnd2 = xRange2->getEnd();

    return m_pImpl->ComparePositions(xEnd1, xEnd2);
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXText::getPropertySetInfo() throw(uno::RuntimeException)
{
    SolarMutexGuard g;

    static uno::Reference< beans::XPropertySetInfo > xInfo =
        m_pImpl->m_rPropSet.getPropertySetInfo();
    return xInfo;
}

void SAL_CALL
SwXText::setPropertyValue(const OUString& /*aPropertyName*/,
        const uno::Any& /*aValue*/)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
    lang::IllegalArgumentException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    throw lang::IllegalArgumentException();
}

uno::Any SAL_CALL
SwXText::getPropertyValue(
    const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
    {
        throw  uno::RuntimeException();
    }

    SfxItemPropertySimpleEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
    {
        beans::UnknownPropertyException aExcept;
        aExcept.Message = "Unknown property: ";
        aExcept.Message += rPropertyName;
        throw aExcept;
    }

    uno::Any aRet;
    switch (pEntry->nWID)
    {
//          no code necessary - the redline is always located at the end node
//            case FN_UNO_REDLINE_NODE_START:
//            break;
        case FN_UNO_REDLINE_NODE_END:
        {
            const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
            const sal_uInt16 nRedTblCount = rRedTbl.size();
            if (nRedTblCount > 0)
            {
                SwStartNode const*const pStartNode = GetStartNode();
                const sal_uLong nOwnIndex = pStartNode->EndOfSectionIndex();
                for (sal_uInt16 nRed = 0; nRed < nRedTblCount; nRed++)
                {
                    SwRedline const*const pRedline = rRedTbl[nRed];
                    SwPosition const*const pRedStart = pRedline->Start();
                    const SwNodeIndex nRedNode = pRedStart->nNode;
                    if (nOwnIndex == nRedNode.GetIndex())
                    {
                        aRet <<= SwXRedlinePortion::CreateRedlineProperties(
                                *pRedline, sal_True);
                        break;
                    }
                }
            }
        }
        break;
    }
    return aRet;
}

void SAL_CALL
SwXText::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_FAIL("SwXText::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXText::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_FAIL("SwXText::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXText::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    OSL_FAIL("SwXText::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXText::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    OSL_FAIL("SwXText::removeVetoableChangeListener(): not implemented");
}

namespace
{
    class theSwXTextUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXText::getUnoTunnelId()
{
    return theSwXTextUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL
SwXText::getSomething(const uno::Sequence< sal_Int8 >& rId)
throw (uno::RuntimeException)
{
    return ::sw::UnoTunnelImpl<SwXText>(rId, this);
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::finishParagraph(
        const uno::Sequence< beans::PropertyValue > & rProperties)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard g;

    return m_pImpl->finishOrAppendParagraph(true, rProperties, uno::Reference< text::XTextRange >());
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::finishParagraphInsert(
        const uno::Sequence< beans::PropertyValue > & rProperties,
        const uno::Reference< text::XTextRange >& xInsertPosition)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard g;

    return m_pImpl->finishOrAppendParagraph(true, rProperties, xInsertPosition);
}

uno::Reference< text::XTextRange >
SwXText::Impl::finishOrAppendParagraph(
        const bool bFinish,
        const uno::Sequence< beans::PropertyValue > & rProperties,
        const uno::Reference< text::XTextRange >& xInsertPosition)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!m_bIsValid)
    {
        throw  uno::RuntimeException();
    }

    const SwStartNode* pStartNode = m_rThis.GetStartNode();
    if(!pStartNode)
    {
        throw  uno::RuntimeException();
    }

    uno::Reference< text::XTextRange > xRet;
    bool bIllegalException = false;
    bool bRuntimeException = false;
    OUString sMessage;
    m_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_START , NULL);
    // find end node, go backward - don't skip tables because the new
    // paragraph has to be the last node
    //aPam.Move( fnMoveBackward, fnGoNode );
    SwPosition aInsertPosition(
            SwNodeIndex( *pStartNode->EndOfSectionNode(), -1 ) );
    SwPaM aPam(aInsertPosition);
    // If we got a position reference, then the insert point is not the end of
    // the document.
    if (xInsertPosition.is())
    {
        SwUnoInternalPaM aStartPam(*m_rThis.GetDoc());
        ::sw::XTextRangeToSwPaM(aStartPam, xInsertPosition);
        aPam = aStartPam;
        aPam.SetMark();
    }
    m_pDoc->AppendTxtNode( *aPam.GetPoint() );
    // remove attributes from the previous paragraph
    m_pDoc->ResetAttrs(aPam);
    // in case of finishParagraph the PaM needs to be moved to the
    // previous paragraph
    if (bFinish)
    {
        aPam.Move( fnMoveBackward, fnGoNode );
    }
#if 1 // This section should be removed in favour of the one below when it works.
    if (rProperties.getLength())
    {
        // now set the properties
        SfxItemPropertySet const*const pParaPropSet =
            aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARAGRAPH);
        const SfxItemPropertyMap &rParagraphMap =
            pParaPropSet->getPropertyMap();

        const beans::PropertyValue* pValues = rProperties.getConstArray();

        for (sal_Int32 nProp = 0; nProp < rProperties.getLength(); ++nProp)
        {
            if (!rParagraphMap.getByName(pValues[nProp].Name))
            {
                bIllegalException = true;
                break;
            }
            try
            {
                SwUnoCursorHelper::SetPropertyValue(aPam, *pParaPropSet,
                    pValues[nProp].Name, pValues[nProp].Value);
            }
            catch (const lang::IllegalArgumentException& rIllegal)
            {
                sMessage = rIllegal.Message;
                bIllegalException = true;
                break;
            }
            catch (const uno::RuntimeException& rRuntime)
            {
                sMessage = rRuntime.Message;
                bRuntimeException = true;
                break;
            }
        }
    }
#else
    try
    {
        SfxItemPropertySet const*const pParaPropSet =
            aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARAGRAPH);
        if (!bIllegalException)
            SwUnoCursorHelper::SetPropertyValues(aPam, *pParaPropSet, rProperties);
    }
    catch (const lang::IllegalArgumentException& rIllegal)
    {
        sMessage = rIllegal.Message;
        bIllegalException = true;
    }
    catch (const uno::RuntimeException& rRuntime)
    {
        sMessage = rRuntime.Message;
        bRuntimeException = true;
    }
#endif

    m_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_END, NULL);
    if (bIllegalException || bRuntimeException)
    {
        m_pDoc->GetIDocumentUndoRedo().Undo();
        if (bIllegalException)
        {
            lang::IllegalArgumentException aEx;
            aEx.Message = sMessage;
            throw aEx;
        }
        else // if(bRuntimeException)
        {
            uno::RuntimeException aEx;
            aEx.Message = sMessage;
            throw aEx;
        }
    }
    SwTxtNode *const pTxtNode( aPam.Start()->nNode.GetNode().GetTxtNode() );
    OSL_ENSURE(pTxtNode, "no SwTxtNode?");
    if (pTxtNode)
    {
        xRet.set(SwXParagraph::CreateXParagraph(*m_pDoc, *pTxtNode, &m_rThis),
                uno::UNO_QUERY);
    }

    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::insertTextPortion(
        const OUString& rText,
        const uno::Sequence< beans::PropertyValue > &
            rCharacterAndParagraphProperties,
        const uno::Reference<text::XTextRange>& xInsertPosition)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
    {
        throw  uno::RuntimeException();
    }
    uno::Reference< text::XTextRange > xRet;
    const uno::Reference< text::XTextCursor > xTextCursor = CreateCursor();
    xTextCursor->gotoRange(xInsertPosition, sal_False);

    const uno::Reference< lang::XUnoTunnel > xRangeTunnel(
            xTextCursor, uno::UNO_QUERY_THROW );
    SwXTextCursor *const pTextCursor =
        ::sw::UnoTunnelGetImplementation<SwXTextCursor>(xRangeTunnel);

    bool bIllegalException = false;
    bool bRuntimeException = false;
    OUString sMessage;
    m_pImpl->m_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, NULL);

//        SwPaM aPam(*pStartNode->EndOfSectionNode());
    //aPam.Move( fnMoveBackward, fnGoNode );
    SwUnoCrsr *const pCursor = pTextCursor->GetCursor();
    m_pImpl->m_pDoc->DontExpandFmt( *pCursor->Start() );

    if (!rText.isEmpty())
    {
        const xub_StrLen nContentPos = pCursor->GetPoint()->nContent.GetIndex();
        SwUnoCursorHelper::DocInsertStringSplitCR(
            *m_pImpl->m_pDoc, *pCursor, rText, false);
        SwUnoCursorHelper::SelectPam(*pCursor, true);
        pCursor->GetPoint()->nContent = nContentPos;
    }

    try
    {
      SfxItemPropertySet const*const pCursorPropSet =
          aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR);
      SwUnoCursorHelper::SetPropertyValues(*pCursor, *pCursorPropSet,
                                           rCharacterAndParagraphProperties,
                                           nsSetAttrMode::SETATTR_NOFORMATATTR);
    }
    catch (const lang::IllegalArgumentException& rIllegal)
    {
        sMessage = rIllegal.Message;
        bIllegalException = true;
    }
    catch (const uno::RuntimeException& rRuntime)
    {
        sMessage = rRuntime.Message;
        bRuntimeException = true;
    }
    m_pImpl->m_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, NULL);
    if (bIllegalException || bRuntimeException)
    {
        m_pImpl->m_pDoc->GetIDocumentUndoRedo().Undo();
        if (bIllegalException)
        {
            lang::IllegalArgumentException aEx;
            aEx.Message = sMessage;
            throw aEx;
        }
        else //if(bRuntimeException)
        {
            uno::RuntimeException aEx;
            aEx.Message = sMessage;
            throw aEx;
        }
    }
    xRet = new SwXTextRange(*pCursor, this);
    return xRet;
}

/*-------------------------------------------------------------------------
    Append text portions at the end of the last paragraph of the text
    interface. Support of import filters.
  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange > SAL_CALL
SwXText::appendTextPortion(
        const OUString& rText,
        const uno::Sequence< beans::PropertyValue > &
            rCharacterAndParagraphProperties)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    // Right now this doesn't need a guard, as it's just calling the insert
    // version, that has it already.
    uno::Reference<text::XTextRange> xInsertPosition = getEnd();
    return insertTextPortion(rText, rCharacterAndParagraphProperties, xInsertPosition);
}

/*-------------------------------------------------------------------------
    enable inserting/appending text contents like graphic objects, shapes and so on
    to support import filters
  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange > SAL_CALL
SwXText::insertTextContentWithProperties(
    const uno::Reference< text::XTextContent >& xTextContent,
    const uno::Sequence< beans::PropertyValue >&
        rCharacterAndParagraphProperties,
    const uno::Reference< text::XTextRange >& xInsertPosition)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!IsValid())
    {
        throw  uno::RuntimeException();
    }

    m_pImpl->m_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, NULL);

    // now attach the text content here
    insertTextContent( xInsertPosition, xTextContent, false );
    // now apply the properties to the anchor
    if (rCharacterAndParagraphProperties.getLength())
    {
        try
        {
            const sal_Int32 nLen(rCharacterAndParagraphProperties.getLength());
            const uno::Reference< beans::XPropertySet > xAnchor(
                xTextContent->getAnchor(), uno::UNO_QUERY);
            if (xAnchor.is())
            {
                for (sal_Int32 nElement = 0; nElement < nLen; ++nElement)
                {
                    xAnchor->setPropertyValue(
                        rCharacterAndParagraphProperties[nElement].Name,
                        rCharacterAndParagraphProperties[nElement].Value);
                }
            }
        }
        catch (const uno::Exception&)
        {
            throw uno::RuntimeException();
        }
    }
    m_pImpl->m_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, NULL);
    return xInsertPosition;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::appendTextContent(
    const uno::Reference< text::XTextContent >& xTextContent,
    const uno::Sequence< beans::PropertyValue >&
        rCharacterAndParagraphProperties)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    // Right now this doesn't need a guard, as it's just calling the insert
    // version, that has it already.
    uno::Reference<text::XTextRange> xInsertPosition = getEnd();
    return insertTextContentWithProperties(xTextContent, rCharacterAndParagraphProperties, xInsertPosition);
}

// move previously appended paragraphs into a text frames
// to support import filters
uno::Reference< text::XTextContent > SAL_CALL
SwXText::convertToTextFrame(
    const uno::Reference< text::XTextRange >& xStart,
    const uno::Reference< text::XTextRange >& xEnd,
    const uno::Sequence< beans::PropertyValue >& rFrameProperties)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
    {
        throw  uno::RuntimeException();
    }
    uno::Reference< text::XTextContent > xRet;
    SwUnoInternalPaM aStartPam(*GetDoc());
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< SwUnoInternalPaM > pEndPam(new SwUnoInternalPaM(*GetDoc()));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (!::sw::XTextRangeToSwPaM(aStartPam, xStart) ||
        !::sw::XTextRangeToSwPaM(*pEndPam, xEnd))
    {
        throw lang::IllegalArgumentException();
    }

    const uno::Reference<lang::XUnoTunnel> xStartRangeTunnel(xStart,
            uno::UNO_QUERY);
    SwXTextRange *const pStartRange =
        ::sw::UnoTunnelGetImplementation<SwXTextRange>(xStartRangeTunnel);
    const uno::Reference<lang::XUnoTunnel> xEndRangeTunnel(xEnd,
            uno::UNO_QUERY);
    SwXTextRange *const pEndRange   =
        ::sw::UnoTunnelGetImplementation<SwXTextRange>(xEndRangeTunnel);
    // bookmarks have to be removed before the referenced text node
    // is deleted in DelFullPara
    if (pStartRange)
    {
        pStartRange->Invalidate();
    }
    if (pEndRange)
    {
        pEndRange->Invalidate();
    }

    m_pImpl->m_pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
    bool bIllegalException = false;
    bool bRuntimeException = false;
    OUString sMessage;
    SwStartNode* pStartStartNode = aStartPam.GetNode()->StartOfSectionNode();
    while (pStartStartNode && pStartStartNode->IsSectionNode())
    {
        pStartStartNode = pStartStartNode->StartOfSectionNode();
    }
    SwStartNode* pEndStartNode = pEndPam->GetNode()->StartOfSectionNode();
    while (pEndStartNode && pEndStartNode->IsSectionNode())
    {
        pEndStartNode = pEndStartNode->StartOfSectionNode();
    }
    bool bParaAfterInserted = false;
    bool bParaBeforeInserted = false;
    if (pStartStartNode != pEndStartNode || pStartStartNode != GetStartNode())
    {
        // todo: if the start/end is in a table then insert a paragraph
        // before/after, move the start/end nodes, then convert and
        // remove the additional paragraphs in the end
        if (pStartStartNode->GetStartNodeType() == SwTableBoxStartNode)
        {
            SwTableNode * pStartTableNode(pStartStartNode->FindTableNode());
            // Is it the same table start node than the end?
            SwTableNode *const pEndStartTableNode(pEndStartNode->FindTableNode());
            while (pEndStartTableNode && pStartTableNode &&
                   pEndStartTableNode->GetIndex() < pStartTableNode->GetIndex())
            {
                SwStartNode* pStartStartTableNode = pStartTableNode->StartOfSectionNode();
                pStartTableNode = pStartStartTableNode->FindTableNode();
            }
            const SwNodeIndex aTblIdx(  *pStartTableNode, -1 );
            SwPosition aBefore(aTblIdx);
            bParaBeforeInserted = GetDoc()->AppendTxtNode( aBefore );
            aStartPam.DeleteMark();
            *aStartPam.GetPoint() = aBefore;
            pStartStartNode = aStartPam.GetNode()->StartOfSectionNode();
        }
        if (pEndStartNode->GetStartNodeType() == SwTableBoxStartNode)
        {
            SwTableNode *const pEndTableNode = pEndStartNode->FindTableNode();
            SwEndNode *const pTableEnd = pEndTableNode->EndOfSectionNode();
            SwPosition aTableEnd(*pTableEnd);
            bParaAfterInserted = GetDoc()->AppendTxtNode( aTableEnd );
            pEndPam->DeleteMark();
            *pEndPam->GetPoint() = aTableEnd;
            pEndStartNode = pEndPam->GetNode()->StartOfSectionNode();
        }
        // now we should have the positions in the same hierarchy
        if ((pStartStartNode != pEndStartNode) ||
            (pStartStartNode != GetStartNode()))
        {
            // if not - remove the additional paragraphs and throw
            if (bParaBeforeInserted)
            {
                SwCursor aDelete(*aStartPam.GetPoint(), 0, false);
                *aStartPam.GetPoint() = // park it because node is deleted
                    SwPosition(GetDoc()->GetNodes().GetEndOfContent());
                aDelete.MovePara(fnParaCurr, fnParaStart);
                aDelete.SetMark();
                aDelete.MovePara(fnParaCurr, fnParaEnd);
                GetDoc()->DelFullPara(aDelete);
            }
            if (bParaAfterInserted)
            {
                SwCursor aDelete(*pEndPam->GetPoint(), 0, false);
                *pEndPam->GetPoint() = // park it because node is deleted
                    SwPosition(GetDoc()->GetNodes().GetEndOfContent());
                aDelete.MovePara(fnParaCurr, fnParaStart);
                aDelete.SetMark();
                aDelete.MovePara(fnParaCurr, fnParaEnd);
                GetDoc()->DelFullPara(aDelete);
            }
            throw lang::IllegalArgumentException();
        }
    }

    // make a selection from aStartPam to a EndPam
    // If there is no content in the frame the shape is in
    // it gets deleted in the DelFullPara call below,
    // In this case insert a tmp text node ( we delete it later )
    if ( aStartPam.Start()->nNode == pEndPam->Start()->nNode
    && aStartPam.End()->nNode == pEndPam->End()->nNode )
    {
        SwPosition aEnd(*aStartPam.End());
        bParaAfterInserted = GetDoc()->AppendTxtNode( aEnd );
        pEndPam->DeleteMark();
        *pEndPam->GetPoint() = aEnd;
    }
    aStartPam.SetMark();
    *aStartPam.End() = *pEndPam->End();
    pEndPam.reset(0);

    // see if there are frames already anchored to this node
    std::vector<SwFrmFmt*> aAnchoredFrames;
    for (size_t i = 0; i < m_pImpl->m_pDoc->GetSpzFrmFmts()->size(); ++i)
    {
        SwFrmFmt* pFrmFmt = (*m_pImpl->m_pDoc->GetSpzFrmFmts())[i];
        const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
        if (FLY_AT_PARA == rAnchor.GetAnchorId() &&
                aStartPam.GetNode()->GetIndex() == rAnchor.GetCntntAnchor()->nNode.GetIndex())
            aAnchoredFrames.push_back(pFrmFmt);
    }

    SwXTextFrame *const pNewFrame = new SwXTextFrame(m_pImpl->m_pDoc);
    const uno::Reference< text::XTextFrame > xNewFrame = pNewFrame;
    pNewFrame->SetSelection( aStartPam );
    try
    {
        const beans::PropertyValue* pValues = rFrameProperties.getConstArray();
        for (sal_Int32 nProp = 0; nProp < rFrameProperties.getLength(); ++nProp)
        {
            pNewFrame->SwXFrame::setPropertyValue(
                    pValues[nProp].Name, pValues[nProp].Value);
        }

        {   // has to be in a block to remove the SwIndexes before
            // DelFullPara is called
            const uno::Reference< text::XTextRange> xInsertTextRange =
                new SwXTextRange(aStartPam, this);
            aStartPam.DeleteMark(); // mark position node may be deleted!
            pNewFrame->attach( xInsertTextRange );
            pNewFrame->setName(m_pImpl->m_pDoc->GetUniqueFrameName());
        }

        SwTxtNode *const pTxtNode(aStartPam.GetNode()->GetTxtNode());
        OSL_ASSERT(pTxtNode);
        if (!pTxtNode || !pTxtNode->Len()) // don't remove if it contains text!
        {
            {   // has to be in a block to remove the SwIndexes before
                // DelFullPara is called
                SwPaM aMovePam( *aStartPam.GetNode() );
                if (aMovePam.Move( fnMoveForward, fnGoCntnt ))
                {
                    // move the anchor to the next paragraph
                    SwFmtAnchor aNewAnchor(pNewFrame->GetFrmFmt()->GetAnchor());
                    aNewAnchor.SetAnchor( aMovePam.Start() );
                    m_pImpl->m_pDoc->SetAttr(
                        aNewAnchor, *pNewFrame->GetFrmFmt() );

                    // also move frames anchored to us
                    for (std::vector<SwFrmFmt*>::iterator i = aAnchoredFrames.begin(); i != aAnchoredFrames.end(); ++i)
                    {
                        // copy the anchor to the next paragraph
                        SwFmtAnchor aAnchor((*i)->GetAnchor());
                        aAnchor.SetAnchor(aMovePam.Start());
                        m_pImpl->m_pDoc->SetAttr(aAnchor, *(*i));
                    }
                }
            }
            m_pImpl->m_pDoc->DelFullPara(aStartPam);
        }
    }
    catch (const lang::IllegalArgumentException& rIllegal)
    {
        sMessage = rIllegal.Message;
        bIllegalException = true;
    }
    catch (const uno::RuntimeException& rRuntime)
    {
        sMessage = rRuntime.Message;
        bRuntimeException = true;
    }
    xRet = pNewFrame;
    if (bParaBeforeInserted || bParaAfterInserted)
    {
        const uno::Reference<text::XTextCursor> xFrameTextCursor =
            pNewFrame->createTextCursor();
        const uno::Reference<XUnoTunnel> xTunnel(xFrameTextCursor,
                uno::UNO_QUERY);
        SwXTextCursor *const pFrameCursor =
            ::sw::UnoTunnelGetImplementation<SwXTextCursor>(xTunnel);
        if (bParaBeforeInserted)
        {
            // todo: remove paragraph before frame
            m_pImpl->m_pDoc->DelFullPara(*pFrameCursor->GetPaM());
        }
        if (bParaAfterInserted)
        {
            xFrameTextCursor->gotoEnd(sal_False);
            if (!bParaBeforeInserted)
                m_pImpl->m_pDoc->DelFullPara(*pFrameCursor->GetPaM());
            else
            {
                // In case the frame has a table only, the cursor points to the end of the first cell of the table.
                SwPaM aPaM(*pFrameCursor->GetPaM()->GetNode()->FindSttNodeByType(SwFlyStartNode)->EndOfSectionNode());
                // Now we have the end of the frame -- the node before that will be the paragraph we want to remove.
                aPaM.GetPoint()->nNode--;
                m_pImpl->m_pDoc->DelFullPara(aPaM);
            }
        }
    }

    m_pImpl->m_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_END, NULL);
    if (bIllegalException || bRuntimeException)
    {
        m_pImpl->m_pDoc->GetIDocumentUndoRedo().Undo();
        if (bIllegalException)
        {
            lang::IllegalArgumentException aEx;
            aEx.Message = sMessage;
            throw aEx;
        }
        else //if(bRuntimeException)
        {
            uno::RuntimeException aEx;
            aEx.Message = sMessage;
            throw aEx;
        }
    }
    return xRet;
}

/*-------------------------------------------------------------------------
    Move previously imported paragraphs into a new text table.
  -----------------------------------------------------------------------*/
struct VerticallyMergedCell
{
    std::vector<uno::Reference< beans::XPropertySet > > aCells;
    sal_Int32                                           nLeftPosition;
    bool                                                bOpen;

    VerticallyMergedCell(uno::Reference< beans::XPropertySet > const& rxCell,
            const sal_Int32 nLeft)
        : nLeftPosition( nLeft )
        , bOpen( true )
    {
        aCells.push_back( rxCell );
    }
};

#define COL_POS_FUZZY 2

static bool lcl_SimilarPosition( const sal_Int32 nPos1, const sal_Int32 nPos2 )
{
    return abs( nPos1 - nPos2 ) < COL_POS_FUZZY;
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
void SwXText::Impl::ConvertCell(
    const bool bFirstCell,
    const uno::Sequence< uno::Reference< text::XTextRange > > & rCell,
    ::std::vector<SwNodeRange> & rRowNodes,
    ::std::auto_ptr< SwPaM > & rpFirstPaM,
    SwPaM & rLastPaM,
    bool & rbExcept)
{
    if (rCell.getLength() != 2)
    {
        throw lang::IllegalArgumentException(
                "rCell needs to contain 2 elements",
                uno::Reference< text::XTextCopy >( &m_rThis ), sal_Int16( 2 ) );
    }
    const uno::Reference<text::XTextRange> xStartRange = rCell[0];
    const uno::Reference<text::XTextRange> xEndRange = rCell[1];
    SwUnoInternalPaM aStartCellPam(*m_pDoc);
    SwUnoInternalPaM aEndCellPam(*m_pDoc);

    // !!! TODO - PaMs in tables and sections do not work here -
    //     the same applies to PaMs in frames !!!

    if (!::sw::XTextRangeToSwPaM(aStartCellPam, xStartRange) ||
        !::sw::XTextRangeToSwPaM(aEndCellPam, xEndRange))
    {
        throw lang::IllegalArgumentException(
                "Start or End range cannot be resolved to a SwPaM",
                uno::Reference< text::XTextCopy >( &m_rThis ), sal_Int16( 2 ) );
    }

    SwNodeRange aTmpRange(aStartCellPam.Start()->nNode,
                          aEndCellPam.End()->nNode);
    SwNodeRange * pCorrectedRange =
        m_pDoc->GetNodes().ExpandRangeForTableBox(aTmpRange);

    if (pCorrectedRange != NULL)
    {
        SwPaM aNewStartPaM(pCorrectedRange->aStart, 0);
        aStartCellPam = aNewStartPaM;

        xub_StrLen nEndLen = 0;
        SwTxtNode * pTxtNode = pCorrectedRange->aEnd.GetNode().GetTxtNode();
        if (pTxtNode != NULL)
            nEndLen = pTxtNode->Len();

        SwPaM aNewEndPaM(pCorrectedRange->aEnd, nEndLen);
        aEndCellPam = aNewEndPaM;
    }

    /** check the nodes between start and end
        it is allowed to have pairs of StartNode/EndNodes
     */
    if (aStartCellPam.Start()->nNode < aEndCellPam.End()->nNode)
    {
        // increment on each StartNode and decrement on each EndNode
        // we must reach zero at the end and must not go below zero
        long nOpenNodeBlock = 0;
        SwNodeIndex aCellIndex = aStartCellPam.Start()->nNode;
        while (aCellIndex < aEndCellPam.End()->nNode.GetIndex())
        {
            if (aCellIndex.GetNode().IsStartNode())
            {
                ++nOpenNodeBlock;
            }
            else if (aCellIndex.GetNode().IsEndNode())
            {
                --nOpenNodeBlock;
            }
            if (nOpenNodeBlock < 0)
            {
                rbExcept = true;
                break;
            }
            ++aCellIndex;
        }
        if (nOpenNodeBlock != 0)
        {
            rbExcept = true;
            return;
        }
    }

    /** The vector<vector> NodeRanges has to contain consecutive nodes.
        In rTableRanges the ranges don't need to be full paragraphs but
        they have to follow each other. To process the ranges they
        have to be aligned on paragraph borders by inserting paragraph
        breaks. Non-consecutive ranges must initiate an exception.
     */
    if (bFirstCell)
    {
        // align the beginning - if necessary
        if (aStartCellPam.Start()->nContent.GetIndex())
        {
            m_pDoc->SplitNode(*aStartCellPam.Start(), sal_False);
        }
    }
    else
    {
        // check the predecessor
        const sal_uLong nLastNodeIndex = rLastPaM.End()->nNode.GetIndex();
        const sal_uLong nStartCellNodeIndex =
            aStartCellPam.Start()->nNode.GetIndex();
        const sal_uLong nLastNodeEndIndex = rLastPaM.End()->nNode.GetIndex();
        if (nLastNodeIndex == nStartCellNodeIndex)
        {
            // same node as predecessor then equal nContent?
            if (rLastPaM.End()->nContent != aStartCellPam.Start()->nContent)
            {
                rbExcept = true;
            }
            else
            {
                m_pDoc->SplitNode(*aStartCellPam.Start(), sal_False);
            }
        }
        else if (nStartCellNodeIndex == (nLastNodeEndIndex + 1))
        {
            // next paragraph - now the content index of the new should be 0
            // and of the old one should be equal to the text length
            // but if it isn't we don't care - the cell is being inserted on
            // the node border anyway
        }
        else
        {
            rbExcept = true;
        }
    }
    // now check if there's a need to insert another paragraph break
    if (aEndCellPam.End()->nContent.GetIndex() <
            aEndCellPam.End()->nNode.GetNode().GetTxtNode()->Len())
    {
        m_pDoc->SplitNode(*aEndCellPam.End(), sal_False);
        // take care that the new start/endcell is moved to the right position
        // aStartCellPam has to point to the start of the new (previous) node
        // aEndCellPam has to point to the end of the new (previous) node
        aStartCellPam.DeleteMark();
        aStartCellPam.Move(fnMoveBackward, fnGoNode);
        aStartCellPam.GetPoint()->nContent = 0;
        aEndCellPam.DeleteMark();
        aEndCellPam.Move(fnMoveBackward, fnGoNode);
        aEndCellPam.GetPoint()->nContent =
            aEndCellPam.GetNode()->GetTxtNode()->Len();
    }

    *rLastPaM.GetPoint() = *aEndCellPam.Start();
    if (aStartCellPam.HasMark())
    {
        rLastPaM.SetMark();
        *rLastPaM.GetMark() = *aEndCellPam.End();
    }
    else
    {
        rLastPaM.DeleteMark();
    }

    SwNodeRange aCellRange(aStartCellPam.Start()->nNode,
            aEndCellPam.End()->nNode);
    rRowNodes.push_back(aCellRange);
    if (bFirstCell)
    {
        rpFirstPaM.reset(new SwPaM(*aStartCellPam.Start()));
    }
}
SAL_WNODEPRECATED_DECLARATIONS_POP

typedef uno::Sequence< text::TableColumnSeparator > TableColumnSeparators;

static void
lcl_ApplyRowProperties(
    uno::Sequence<beans::PropertyValue> const& rRowProperties,
    uno::Any const& rRow,
    TableColumnSeparators & rRowSeparators)
{
    uno::Reference< beans::XPropertySet > xRow;
    rRow >>= xRow;
    const beans::PropertyValue* pProperties = rRowProperties.getConstArray();
    for (sal_Int32 nProperty = 0; nProperty < rRowProperties.getLength();
         ++nProperty)
    {
        if ( pProperties[ nProperty ].Name == "TableColumnSeparators" )
        {
            // add the separators to access the cell's positions
            // for vertical merging later
            TableColumnSeparators aSeparators;
            pProperties[ nProperty ].Value >>= aSeparators;
            rRowSeparators = aSeparators;
        }
        xRow->setPropertyValue(
            pProperties[ nProperty ].Name, pProperties[ nProperty ].Value);
    }
}

#if OSL_DEBUG_LEVEL > 0
//-->debug cell properties of all rows
static void
lcl_DebugCellProperties(
    const uno::Sequence< uno::Sequence< uno::Sequence<
        beans::PropertyValue > > >& rCellProperties)
{
    OUString sNames;
    for (sal_Int32  nDebugRow = 0; nDebugRow < rCellProperties.getLength();
         ++nDebugRow)
    {
        const uno::Sequence< beans::PropertyValues > aDebugCurrentRow =
            rCellProperties[nDebugRow];
        sal_Int32 nDebugCells = aDebugCurrentRow.getLength();
        (void) nDebugCells;
        for (sal_Int32  nDebugCell = 0; nDebugCell < nDebugCells;
             ++nDebugCell)
        {
            const uno::Sequence< beans::PropertyValue >&
                rDebugCellProperties = aDebugCurrentRow[nDebugCell];
            const sal_Int32 nDebugCellProperties =
                rDebugCellProperties.getLength();
            for (sal_Int32  nDebugProperty = 0;
                 nDebugProperty < nDebugCellProperties; ++nDebugProperty)
            {
                const OUString sName =
                    rDebugCellProperties[nDebugProperty].Name;
                sNames += sName;
                sNames += OUString('-');
            }
            sNames += OUString('+');
        }
        sNames += OUString('|');
    }
    (void)sNames;
}
//--<
#endif

static void
lcl_ApplyCellProperties(
    const sal_Int32 nCell,
    TableColumnSeparators const& rRowSeparators,
    const uno::Sequence< beans::PropertyValue >& rCellProperties,
    uno::Reference< uno::XInterface > xCell,
    ::std::vector<VerticallyMergedCell> & rMergedCells)
{
    const sal_Int32 nCellProperties = rCellProperties.getLength();
    const uno::Reference< beans::XPropertySet > xCellPS(xCell, uno::UNO_QUERY);
    for (sal_Int32 nProperty = 0; nProperty < nCellProperties; ++nProperty)
    {
        const OUString & rName  = rCellProperties[nProperty].Name;
        const uno::Any & rValue = rCellProperties[nProperty].Value;
        if ( rName == "VerticalMerge" )
        {
            // determine left border position
            // add the cell to a queue of merged cells
            sal_Bool bMerge = sal_False;
            rValue >>= bMerge;
            sal_Int32 nLeftPos = -1;
            if (!nCell)
            {
                nLeftPos = 0;
            }
            else if (rRowSeparators.getLength() >= nCell)
            {
                const text::TableColumnSeparator* pSeparators =
                    rRowSeparators.getConstArray();
                nLeftPos = pSeparators[nCell - 1].Position;
            }
            if (bMerge)
            {
                // 'close' all the cell with the same left position
                // if separate vertical merges in the same column exist
                if (rMergedCells.size())
                {
                    std::vector<VerticallyMergedCell>::iterator aMergedIter =
                        rMergedCells.begin();
                    while (aMergedIter != rMergedCells.end())
                    {
                        if (lcl_SimilarPosition(aMergedIter->nLeftPosition,
                                    nLeftPos))
                        {
                            aMergedIter->bOpen = false;
                        }
                        ++aMergedIter;
                    }
                }
                // add the new group of merged cells
                rMergedCells.push_back(VerticallyMergedCell(xCellPS, nLeftPos));
            }
            else
            {
                // find the cell that
                OSL_ENSURE(rMergedCells.size(),
                        "the first merged cell is missing");
                if (rMergedCells.size())
                {
                    std::vector<VerticallyMergedCell>::iterator aMergedIter =
                        rMergedCells.begin();
#if OSL_DEBUG_LEVEL > 0
                    bool bDbgFound = false;
#endif
                    while (aMergedIter != rMergedCells.end())
                    {
                        if (aMergedIter->bOpen &&
                            lcl_SimilarPosition(aMergedIter->nLeftPosition,
                                nLeftPos))
                        {
                            aMergedIter->aCells.push_back( xCellPS );
#if OSL_DEBUG_LEVEL > 0
                            bDbgFound = true;
#endif
                        }
                        ++aMergedIter;
                    }
#if OSL_DEBUG_LEVEL > 0
                    OSL_ENSURE( bDbgFound,
                            "couldn't find first vertically merged cell" );
#endif
                }
            }
        }
        else
        {
            try
            {
                xCellPS->setPropertyValue(rName, rValue);
            }
            catch (const uno::Exception&)
            {
                // Apply the paragraph and char properties to the cell's content
                const uno::Reference< text::XText > xCellText(xCell,
                        uno::UNO_QUERY);
                const uno::Reference< text::XTextCursor > xCellCurs =
                    xCellText->createTextCursor();
                xCellCurs->gotoStart( sal_False );
                xCellCurs->gotoEnd( sal_True );
                const uno::Reference< beans::XPropertyState >
                    xCellTextPropState(xCellCurs, uno::UNO_QUERY);
                const beans::PropertyState state = xCellTextPropState->getPropertyState(rName);
                if (state == beans::PropertyState_DEFAULT_VALUE)
                {
                    const uno::Reference< beans::XPropertySet >
                        xCellTextProps(xCellCurs, uno::UNO_QUERY);
                    xCellTextProps->setPropertyValue(rName, rValue);
                }
            }
        }
    }
}

static void
lcl_MergeCells(::std::vector<VerticallyMergedCell> & rMergedCells)
{
    if (rMergedCells.size())
    {
        std::vector<VerticallyMergedCell>::iterator aMergedIter =
            rMergedCells.begin();
        while (aMergedIter != rMergedCells.end())
        {
            sal_Int32 nCellCount =
                static_cast<sal_Int32>(aMergedIter->aCells.size());
            std::vector<uno::Reference< beans::XPropertySet > >::iterator
                aCellIter = aMergedIter->aCells.begin();
            bool bFirstCell = true;
            // the first of the cells gets the number of cells set as RowSpan
            // the others get the inverted number of remaining merged cells
            // (3,-2,-1)
            while (aCellIter != aMergedIter->aCells.end())
            {
                (*aCellIter)->setPropertyValue(
                    OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_ROW_SPAN)),
                    uno::makeAny(nCellCount));
                if (bFirstCell)
                {
                    nCellCount *= -1;
                    bFirstCell = false;
                }
                ++nCellCount;
                ++aCellIter;
            }
            ++aMergedIter;
        }
    }
}

uno::Reference< text::XTextTable > SAL_CALL
SwXText::convertToTable(
    const uno::Sequence< uno::Sequence< uno::Sequence<
        uno::Reference< text::XTextRange > > > >& rTableRanges,
    const uno::Sequence< uno::Sequence< uno::Sequence<
        beans::PropertyValue > > >& rCellProperties,
    const uno::Sequence< uno::Sequence< beans::PropertyValue > >&
        rRowProperties,
    const uno::Sequence< beans::PropertyValue >& rTableProperties)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
    {
        throw  uno::RuntimeException();
    }

    //at first collect the text ranges as SwPaMs
    const uno::Sequence< uno::Sequence< uno::Reference< text::XTextRange > > >*
        pTableRanges = rTableRanges.getConstArray();
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr < SwPaM > pFirstPaM;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    std::vector< std::vector<SwNodeRange> > aTableNodes;
    bool bExcept = false;
    SwPaM aLastPaM(m_pImpl->m_pDoc->GetNodes());
    for (sal_Int32 nRow = 0; !bExcept && (nRow < rTableRanges.getLength());
            ++nRow)
    {
        std::vector<SwNodeRange> aRowNodes;
        const uno::Sequence< uno::Reference< text::XTextRange > >* pRow =
            pTableRanges[nRow].getConstArray();
        const sal_Int32 nCells(pTableRanges[nRow].getLength());

        for (sal_Int32 nCell = 0; nCell < nCells; ++nCell)
        {
            m_pImpl->ConvertCell((nCell == 0) && (nRow == 0), pRow[nCell],
                aRowNodes, pFirstPaM, aLastPaM, bExcept);
        }
        aTableNodes.push_back(aRowNodes);
    }

    if(bExcept)
    {
        m_pImpl->m_pDoc->GetIDocumentUndoRedo().Undo();
        throw lang::IllegalArgumentException();
    }

    std::vector< TableColumnSeparators >
        aRowSeparators(rRowProperties.getLength());
    std::vector<VerticallyMergedCell> aMergedCells;

    SwTable const*const pTable = m_pImpl->m_pDoc->TextToTable( aTableNodes );

    if (!pTable)
        return uno::Reference< text::XTextTable >();

    SwXTextTable *const pTextTable = new SwXTextTable( *pTable->GetFrmFmt() );
    const uno::Reference< text::XTextTable > xRet = pTextTable;
    const uno::Reference< beans::XPropertySet > xPrSet = pTextTable;
    // set properties to the table
    // catch lang::WrappedTargetException and lang::IndexOutOfBoundsException
    try
    {
        //apply table properties
        const beans::PropertyValue* pTableProperties =
            rTableProperties.getConstArray();
        for (sal_Int32 nProperty = 0; nProperty < rTableProperties.getLength();
             ++nProperty)
        {
            try
            {
                xPrSet->setPropertyValue( pTableProperties[nProperty].Name,
                        pTableProperties[nProperty].Value );
            }
            catch (const uno::Exception& e)
            {
                SAL_WARN( "sw.uno", "Exception when setting property: "
                    + pTableProperties[nProperty].Name + ". Message: " + e.Message );
            }
        }

        //apply row properties
        const uno::Reference< table::XTableRows >  xRows = xRet->getRows();

        const beans::PropertyValues* pRowProperties =
            rRowProperties.getConstArray();
        for (sal_Int32 nRow = 0; nRow < xRows->getCount(); ++nRow)
        {
            if( nRow >= rRowProperties.getLength())
            {
                break;
            }
            lcl_ApplyRowProperties(pRowProperties[nRow],
                xRows->getByIndex(nRow), aRowSeparators[nRow]);
        }

#if OSL_DEBUG_LEVEL > 0
        lcl_DebugCellProperties(rCellProperties);
#endif

        //apply cell properties
        for (sal_Int32 nRow = 0; nRow < rCellProperties.getLength(); ++nRow)
        {
            const uno::Sequence< beans::PropertyValues > aCurrentRow =
                rCellProperties[nRow];
            sal_Int32 nCells = aCurrentRow.getLength();
            for (sal_Int32  nCell = 0; nCell < nCells; ++nCell)
            {
                lcl_ApplyCellProperties(nCell,
                    aRowSeparators[nRow], aCurrentRow[nCell],
                    pTextTable->getCellByPosition(nCell, nRow),
                    aMergedCells);
            }
        }
        // now that the cell properties are set the vertical merge values
        // have to be applied
        lcl_MergeCells(aMergedCells);
    }
    catch (const lang::WrappedTargetException&)
    {
    }
    catch (const lang::IndexOutOfBoundsException&)
    {
    }

    return xRet;
}


void SAL_CALL
SwXText::copyText(
    const uno::Reference< text::XTextCopy >& xSource )
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference< text::XText > const xText(xSource, uno::UNO_QUERY_THROW);
    uno::Reference< text::XTextCursor > const xCursor =
        xText->createTextCursor();
    xCursor->gotoEnd( sal_True );

    uno::Reference< lang::XUnoTunnel > const xCursorTunnel(xCursor,
        uno::UNO_QUERY_THROW);

    OTextCursorHelper *const pCursor =
        ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xCursorTunnel);
    if (!pCursor)
    {
        throw uno::RuntimeException();
    }

    SwNodeIndex rNdIndex( *GetStartNode( ), 1 );
    SwPosition rPos( rNdIndex );
    m_pImpl->m_pDoc->CopyRange( *pCursor->GetPaM(), rPos, false );
}


/******************************************************************
 * SwXBodyText
 ******************************************************************/
SwXBodyText::SwXBodyText(SwDoc *const pDoc)
    : SwXText(pDoc, CURSOR_BODY)
{
}

SwXBodyText::~SwXBodyText()
{
}

OUString SAL_CALL
SwXBodyText::getImplementationName() throw (uno::RuntimeException)
{
    return OUString("SwXBodyText");
}

static char const*const g_ServicesBodyText[] =
{
    "com.sun.star.text.Text",
};

static const size_t g_nServicesBodyText(
    sizeof(g_ServicesBodyText)/sizeof(g_ServicesBodyText[0]));

sal_Bool SAL_CALL SwXBodyText::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            g_nServicesBodyText, g_ServicesBodyText, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXBodyText::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesBodyText, g_ServicesBodyText);
}

uno::Any SAL_CALL
SwXBodyText::queryAggregation(const uno::Type& rType)
throw (uno::RuntimeException)
{
    uno::Any aRet;
    if (rType == container::XEnumerationAccess::static_type())
    {
        aRet <<= uno::Reference< container::XEnumerationAccess >(this);
    }
    else if (rType == container::XElementAccess::static_type())
    {
        aRet <<= uno::Reference< container::XElementAccess >(this);
    }
    else if (rType == lang::XServiceInfo::static_type())
    {
        aRet <<= uno::Reference< lang::XServiceInfo >(this);
    }
    else
    {
        aRet = SwXText::queryInterface( rType );
    }
    if(aRet.getValueType() == ::getCppuVoidType())
    {
        aRet = OWeakAggObject::queryAggregation( rType );
    }
    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL
SwXBodyText::getTypes() throw (uno::RuntimeException)
{
    const uno::Sequence< uno::Type > aTypes = SwXBodyText_Base::getTypes();
    const uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();
    return ::comphelper::concatSequences(aTypes, aTextTypes);
}

namespace
{
    class theSwXBodyTextImplementationId : public rtl::Static< UnoTunnelIdInit, theSwXBodyTextImplementationId> {};
}

uno::Sequence< sal_Int8 > SAL_CALL
SwXBodyText::getImplementationId() throw (uno::RuntimeException)
{
    return theSwXBodyTextImplementationId::get().getSeq();
}

uno::Any SAL_CALL
SwXBodyText::queryInterface(const uno::Type& rType)
throw (uno::RuntimeException)
{
    const uno::Any ret = SwXText::queryInterface(rType);
    return (ret.getValueType() == ::getCppuVoidType())
        ?   SwXBodyText_Base::queryInterface(rType)
        :   ret;
}

SwXTextCursor * SwXBodyText::CreateTextCursor(const bool bIgnoreTables)
{
    if(!IsValid())
    {
        return 0;
    }

    // the cursor has to skip tables contained in this text
    SwPaM aPam(GetDoc()->GetNodes().GetEndOfContent());
    aPam.Move( fnMoveBackward, fnGoDoc );
    if (!bIgnoreTables)
    {
        SwTableNode * pTblNode = aPam.GetNode()->FindTableNode();
        SwCntntNode * pCont = 0;
        while (pTblNode)
        {
            aPam.GetPoint()->nNode = *pTblNode->EndOfSectionNode();
            pCont = GetDoc()->GetNodes().GoNext(&aPam.GetPoint()->nNode);
            pTblNode = pCont->FindTableNode();
        }
        if (pCont)
        {
            aPam.GetPoint()->nContent.Assign(pCont, 0);
        }
    }
    return new SwXTextCursor(*GetDoc(), this, CURSOR_BODY, *aPam.GetPoint());
}

uno::Reference< text::XTextCursor > SAL_CALL
SwXBodyText::createTextCursor() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const uno::Reference< text::XTextCursor > xRef(
            static_cast<text::XWordCursor*>(CreateTextCursor(false)) );
    if (!xRef.is())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }
    return xRef;
}

uno::Reference< text::XTextCursor > SAL_CALL
SwXBodyText::createTextCursorByRange(
    const uno::Reference< text::XTextRange > & xTextPosition)
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }

    uno::Reference< text::XTextCursor >  aRef;
    SwUnoInternalPaM aPam(*GetDoc());
    if (::sw::XTextRangeToSwPaM(aPam, xTextPosition))
    {
        if ( !aPam.GetNode()->GetTxtNode() )
            throw uno::RuntimeException("Invalid text range", uno::Reference< uno::XInterface >() );

        SwNode& rNode = GetDoc()->GetNodes().GetEndOfContent();

        SwStartNode* p1 = aPam.GetNode()->StartOfSectionNode();
        //document starts with a section?
        while(p1->IsSectionNode())
        {
            p1 = p1->StartOfSectionNode();
        }
        SwStartNode *const p2 = rNode.StartOfSectionNode();

        if(p1 == p2)
        {
            aRef = static_cast<text::XWordCursor*>(
                    new SwXTextCursor(*GetDoc(), this, CURSOR_BODY,
                        *aPam.GetPoint(), aPam.GetMark()));
        }
    }
    if(!aRef.is())
    {
        throw uno::RuntimeException( "End of content node doesn't have the proper start node",
               uno::Reference< uno::XInterface >( *this ) );
    }
    return aRef;
}

uno::Reference< container::XEnumeration > SAL_CALL
SwXBodyText::createEnumeration()
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!IsValid())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }

    SwNode& rNode = GetDoc()->GetNodes().GetEndOfContent();
    SwPosition aPos(rNode);
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwUnoCrsr> pUnoCursor(
        GetDoc()->CreateUnoCrsr(aPos, false));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    pUnoCursor->Move(fnMoveBackward, fnGoDoc);
    const uno::Reference< container::XEnumeration > xRet
        = new SwXParagraphEnumeration(this, pUnoCursor, CURSOR_BODY);
    return xRet;
}

uno::Type SAL_CALL
SwXBodyText::getElementType() throw (uno::RuntimeException)
{
    return text::XTextRange::static_type();
}

sal_Bool SAL_CALL
SwXBodyText::hasElements() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!IsValid())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }

    return sal_True;
}

/******************************************************************
 *  SwXHeadFootText
 ******************************************************************/
class SwXHeadFootText::Impl
    : public SwClient
{

public:

    bool                        m_bIsHeader;

    Impl(   SwXHeadFootText & /*rThis*/,
            SwFrmFmt & rHeadFootFmt, const bool bIsHeader)
        : SwClient(& rHeadFootFmt)
        , m_bIsHeader(bIsHeader)
    {
    }

    SwFrmFmt * GetHeadFootFmt() const {
        return static_cast<SwFrmFmt*>(
                const_cast<SwModify*>(GetRegisteredIn()));
    }

    SwFrmFmt & GetHeadFootFmtOrThrow() {
        SwFrmFmt *const pFmt( GetHeadFootFmt() );
        if (!pFmt) {
            throw uno::RuntimeException(OUString(
                    "SwXHeadFootText: disposed or invalid"), 0);
        }
        return *pFmt;
    }
protected:
    // SwClient
    virtual void Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew);

};

void SwXHeadFootText::Impl::Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

bool SwXHeadFootText::IsXHeadFootText(SwClient *const pClient)
{
    return 0 != dynamic_cast<SwXHeadFootText::Impl*>(pClient);
}

uno::Reference< text::XText >
SwXHeadFootText::CreateXHeadFootText(
        SwFrmFmt & rHeadFootFmt, const bool bIsHeader)
{
    // re-use existing SwXHeadFootText
    // #i105557#: do not iterate over the registered clients: race condition
    uno::Reference< text::XText > xText(rHeadFootFmt.GetXObject(),
            uno::UNO_QUERY);
    if (!xText.is())
    {
        SwXHeadFootText *const pXHFT(
                new SwXHeadFootText(rHeadFootFmt, bIsHeader));
        xText.set(pXHFT);
        rHeadFootFmt.SetXObject(xText);
    }
    return xText;
}

SwXHeadFootText::SwXHeadFootText(SwFrmFmt & rHeadFootFmt, const bool bIsHeader)
    : SwXText(rHeadFootFmt.GetDoc(),
            (bIsHeader) ? CURSOR_HEADER : CURSOR_FOOTER)
    , m_pImpl( new SwXHeadFootText::Impl(*this, rHeadFootFmt, bIsHeader) )
{
}

SwXHeadFootText::~SwXHeadFootText()
{
}

OUString SAL_CALL
SwXHeadFootText::getImplementationName() throw (uno::RuntimeException)
{
    return OUString("SwXHeadFootText");
}

static char const*const g_ServicesHeadFootText[] =
{
    "com.sun.star.text.Text",
};

sal_Bool SAL_CALL SwXHeadFootText::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return ::sw::SupportsServiceImpl(
            SAL_N_ELEMENTS(g_ServicesHeadFootText),
            g_ServicesHeadFootText, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXHeadFootText::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return ::sw::GetSupportedServiceNamesImpl(
            SAL_N_ELEMENTS(g_ServicesHeadFootText),
            g_ServicesHeadFootText);
}

const SwStartNode *SwXHeadFootText::GetStartNode() const
{
    const SwStartNode *pSttNd = 0;
    SwFrmFmt *const pHeadFootFmt = m_pImpl->GetHeadFootFmt();
    if(pHeadFootFmt)
    {
        const SwFmtCntnt& rFlyCntnt = pHeadFootFmt->GetCntnt();
        if( rFlyCntnt.GetCntntIdx() )
        {
            pSttNd = rFlyCntnt.GetCntntIdx()->GetNode().GetStartNode();
        }
    }
    return pSttNd;
}

uno::Reference< text::XTextCursor >
SwXHeadFootText::CreateCursor() throw (uno::RuntimeException)
{
    return createTextCursor();
}

uno::Sequence< uno::Type > SAL_CALL
SwXHeadFootText::getTypes() throw (uno::RuntimeException)
{
    const uno::Sequence< uno::Type > aTypes = SwXHeadFootText_Base::getTypes();
    const uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();
    return ::comphelper::concatSequences(aTypes, aTextTypes);
}

namespace
{
    class theSwXHeadFootTextImplementationId : public rtl::Static< UnoTunnelIdInit, theSwXHeadFootTextImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL
SwXHeadFootText::getImplementationId() throw (uno::RuntimeException)
{
    return theSwXHeadFootTextImplementationId::get().getSeq();
}

uno::Any SAL_CALL
SwXHeadFootText::queryInterface(const uno::Type& rType)
throw (uno::RuntimeException)
{
    const uno::Any ret = SwXHeadFootText_Base::queryInterface(rType);
    return (ret.getValueType() == ::getCppuVoidType())
        ?   SwXText::queryInterface(rType)
        :   ret;
}

uno::Reference< text::XTextCursor > SAL_CALL
SwXHeadFootText::createTextCursor() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwFrmFmt & rHeadFootFmt( m_pImpl->GetHeadFootFmtOrThrow() );

    uno::Reference< text::XTextCursor > xRet;
    const SwFmtCntnt& rFlyCntnt = rHeadFootFmt.GetCntnt();
    const SwNode& rNode = rFlyCntnt.GetCntntIdx()->GetNode();
    SwPosition aPos(rNode);
    SwXTextCursor *const pXCursor = new SwXTextCursor(*GetDoc(), this,
            (m_pImpl->m_bIsHeader) ? CURSOR_HEADER : CURSOR_FOOTER, aPos);
    SwUnoCrsr *const pUnoCrsr = pXCursor->GetCursor();
    pUnoCrsr->Move(fnMoveForward, fnGoNode);

    // save current start node to be able to check if there is content
    // after the table - otherwise the cursor would be in the body text!
    SwStartNode const*const pOwnStartNode = rNode.FindSttNodeByType(
            (m_pImpl->m_bIsHeader) ? SwHeaderStartNode : SwFooterStartNode);
    // is there a table here?
    SwTableNode* pTblNode = pUnoCrsr->GetNode()->FindTableNode();
    SwCntntNode* pCont = 0;
    while (pTblNode)
    {
        pUnoCrsr->GetPoint()->nNode = *pTblNode->EndOfSectionNode();
        pCont = GetDoc()->GetNodes().GoNext(&pUnoCrsr->GetPoint()->nNode);
        pTblNode = pCont->FindTableNode();
    }
    if (pCont)
    {
        pUnoCrsr->GetPoint()->nContent.Assign(pCont, 0);
    }
    SwStartNode const*const pNewStartNode =
        pUnoCrsr->GetNode()->FindSttNodeByType(
            (m_pImpl->m_bIsHeader) ? SwHeaderStartNode : SwFooterStartNode);
    if (!pNewStartNode || (pNewStartNode != pOwnStartNode))
    {
        uno::RuntimeException aExcept;
        aExcept.Message = "no text available";
        throw aExcept;
    }
    xRet = static_cast<text::XWordCursor*>(pXCursor);
    return xRet;
}

uno::Reference< text::XTextCursor > SAL_CALL
SwXHeadFootText::createTextCursorByRange(
    const uno::Reference< text::XTextRange > & xTextPosition)
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwFrmFmt & rHeadFootFmt( m_pImpl->GetHeadFootFmtOrThrow() );

    SwUnoInternalPaM aPam(*GetDoc());
    if (!::sw::XTextRangeToSwPaM(aPam, xTextPosition))
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }

    uno::Reference< text::XTextCursor >  xRet;
    SwNode& rNode = rHeadFootFmt.GetCntnt().GetCntntIdx()->GetNode();
    SwPosition aPos(rNode);
    SwPaM aHFPam(aPos);
    aHFPam.Move(fnMoveForward, fnGoNode);
    SwStartNode *const pOwnStartNode = aHFPam.GetNode()->FindSttNodeByType(
            (m_pImpl->m_bIsHeader) ? SwHeaderStartNode : SwFooterStartNode);
    SwStartNode *const p1 = aPam.GetNode()->FindSttNodeByType(
            (m_pImpl->m_bIsHeader) ? SwHeaderStartNode : SwFooterStartNode);
    if (p1 == pOwnStartNode)
    {
        xRet = static_cast<text::XWordCursor*>(
                new SwXTextCursor(*GetDoc(), this,
                    (m_pImpl->m_bIsHeader) ? CURSOR_HEADER : CURSOR_FOOTER,
                    *aPam.GetPoint(), aPam.GetMark()));
    }
    return xRet;
}

uno::Reference< container::XEnumeration > SAL_CALL
SwXHeadFootText::createEnumeration()
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwFrmFmt & rHeadFootFmt( m_pImpl->GetHeadFootFmtOrThrow() );

    uno::Reference< container::XEnumeration >  aRef;
    const SwFmtCntnt& rFlyCntnt = rHeadFootFmt.GetCntnt();
    const SwNode& rNode = rFlyCntnt.GetCntntIdx()->GetNode();
    SwPosition aPos(rNode);
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwUnoCrsr> pUnoCursor(
        GetDoc()->CreateUnoCrsr(aPos, false));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    pUnoCursor->Move(fnMoveForward, fnGoNode);
    aRef = new SwXParagraphEnumeration(this, pUnoCursor,
                (m_pImpl->m_bIsHeader) ? CURSOR_HEADER : CURSOR_FOOTER);

    return aRef;
}

uno::Type SAL_CALL
SwXHeadFootText::getElementType() throw (uno::RuntimeException)
{
    return text::XTextRange::static_type();
}

sal_Bool SAL_CALL SwXHeadFootText::hasElements() throw (uno::RuntimeException)
{
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
