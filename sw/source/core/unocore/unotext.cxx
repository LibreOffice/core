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
#include <set>
#include <utility>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

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
        , m_bIsValid(nullptr != pDoc)
    {
    }

    uno::Reference< text::XTextRange >
        finishOrAppendParagraph(
            const bool bFinish,
            const uno::Sequence< beans::PropertyValue >&
                rCharacterAndParagraphProperties,
            const uno::Reference< text::XTextRange >& xInsertPosition)
        throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception);

    sal_Int16 ComparePositions(
            const uno::Reference<text::XTextRange>& xPos1,
            const uno::Reference<text::XTextRange>& xPos2)
        throw (lang::IllegalArgumentException, uno::RuntimeException);

    bool CheckForOwnMember(const SwPaM & rPaM)
        throw (lang::IllegalArgumentException, uno::RuntimeException);

    void ConvertCell(
            const uno::Sequence< uno::Reference< text::XTextRange > > & rCell,
            ::std::vector<SwNodeRange> & rRowNodes,
            SwNodeRange *const pLastCell,
            bool & rbExcept);

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
    m_pImpl->m_bIsValid = (nullptr != pDoc);
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
SwXText::queryInterface(const uno::Type& rType) throw (uno::RuntimeException, std::exception)
{
    uno::Any aRet;
    if (rType == cppu::UnoType<text::XText>::get())
    {
        aRet <<= uno::Reference< text::XText >(this);
    }
    else if (rType == cppu::UnoType<text::XSimpleText>::get())
    {
        aRet <<= uno::Reference< text::XSimpleText >(this);
    }
    else if (rType == cppu::UnoType<text::XTextRange>::get())
    {
        aRet <<= uno::Reference< text::XTextRange>(this);
    }
    else if (rType == cppu::UnoType<text::XTextRangeCompare>::get())
    {
        aRet <<= uno::Reference< text::XTextRangeCompare >(this);
    }
    else if (rType == cppu::UnoType<lang::XTypeProvider>::get())
    {
        aRet <<= uno::Reference< lang::XTypeProvider >(this);
    }
    else if (rType == cppu::UnoType<text::XRelativeTextContentInsert>::get())
    {
        aRet <<= uno::Reference< text::XRelativeTextContentInsert >(this);
    }
    else if (rType == cppu::UnoType<text::XRelativeTextContentRemove>::get())
    {
        aRet <<= uno::Reference< text::XRelativeTextContentRemove >(this);
    }
    else if (rType == cppu::UnoType<beans::XPropertySet>::get())
    {
        aRet <<= uno::Reference< beans::XPropertySet >(this);
    }
    else if (rType == cppu::UnoType<lang::XUnoTunnel>::get())
    {
        aRet <<= uno::Reference< lang::XUnoTunnel >(this);
    }
    else if (rType == cppu::UnoType<text::XTextAppendAndConvert>::get())
    {
        aRet <<= uno::Reference< text::XTextAppendAndConvert >(this);
    }
    else if (rType == cppu::UnoType<text::XTextAppend>::get())
    {
        aRet <<= uno::Reference< text::XTextAppend >(this);
    }
    else if (rType == cppu::UnoType<text::XTextPortionAppend>::get())
    {
        aRet <<= uno::Reference< text::XTextPortionAppend >(this);
    }
    else if (rType == cppu::UnoType<text::XParagraphAppend>::get())
    {
        aRet <<= uno::Reference< text::XParagraphAppend >(this);
    }
    else if (rType == cppu::UnoType<text::XTextConvert>::get() )
    {
        aRet <<= uno::Reference< text::XTextConvert >(this);
    }
    else if (rType == cppu::UnoType<text::XTextContentAppend>::get())
    {
        aRet <<= uno::Reference< text::XTextContentAppend >(this);
    }
    else if(rType == cppu::UnoType<text::XTextCopy>::get())
    {
        aRet <<= uno::Reference< text::XTextCopy >( this );
    }
    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL
SwXText::getTypes() throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type > aRet(12);
    uno::Type* pTypes = aRet.getArray();
    pTypes[0] = cppu::UnoType<text::XText>::get();
    pTypes[1] = cppu::UnoType<text::XTextRangeCompare>::get();
    pTypes[2] = cppu::UnoType<text::XRelativeTextContentInsert>::get();
    pTypes[3] = cppu::UnoType<text::XRelativeTextContentRemove>::get();
    pTypes[4] = cppu::UnoType<lang::XUnoTunnel>::get();
    pTypes[5] = cppu::UnoType<beans::XPropertySet>::get();
    pTypes[6] = cppu::UnoType<text::XTextPortionAppend>::get();
    pTypes[7] = cppu::UnoType<text::XParagraphAppend>::get();
    pTypes[8] = cppu::UnoType<text::XTextContentAppend>::get();
    pTypes[9] = cppu::UnoType<text::XTextConvert>::get();
    pTypes[10] = cppu::UnoType<text::XTextAppend>::get();
    pTypes[11] = cppu::UnoType<text::XTextAppendAndConvert>::get();

    return aRet;
}

// belongs the range in the text ? insert it then.
void SAL_CALL
SwXText::insertString(const uno::Reference< text::XTextRange >& xTextRange,
    const OUString& rString, sal_Bool bAbsorb)
throw (uno::RuntimeException, std::exception)
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
    if ((!pRange  || &pRange ->GetDoc() != GetDoc()) &&
        (!pCursor || pCursor->GetDoc() != GetDoc()))
    {
        throw uno::RuntimeException();
    }

    const SwStartNode *const pOwnStartNode = GetStartNode();
    SwPaM aPam(GetDoc()->GetNodes());
    const SwPaM * pPam(nullptr);
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

    const SwStartNode* pTmp(pPam->GetNode().StartOfSectionNode());
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
            throw uno::RuntimeException(iae.Message, nullptr);
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
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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

    const SwInsertFlags nInsertFlags =
        bForceExpandHints
        ? ( SwInsertFlags::FORCEHINTEXPAND | SwInsertFlags::EMPTYEXPAND)
        : SwInsertFlags::EMPTYEXPAND;

    if (bAbsorb && aPam.HasMark())
    {
        m_pImpl->m_pDoc->getIDocumentContentOperations().DeleteAndJoin(aPam);
        aPam.DeleteMark();
    }

    sal_Unicode cIns = 0;
    switch (nControlCharacter)
    {
        case text::ControlCharacter::PARAGRAPH_BREAK :
            // a table cell now becomes an ordinary text cell!
            m_pImpl->m_pDoc->ClearBoxNumAttrs(aPam.GetPoint()->nNode);
            m_pImpl->m_pDoc->getIDocumentContentOperations().SplitNode(*aPam.GetPoint(), false);
            break;
        case text::ControlCharacter::APPEND_PARAGRAPH:
        {
            m_pImpl->m_pDoc->ClearBoxNumAttrs(aPam.GetPoint()->nNode);
            m_pImpl->m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPam.GetPoint());

            const uno::Reference<lang::XUnoTunnel> xRangeTunnel(
                    xTextRange, uno::UNO_QUERY);
            SwXTextRange *const pRange =
                ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
            OTextCursorHelper *const pCursor =
                ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(
                            xRangeTunnel);
            if (pRange)
            {
                pRange->SetPositions(aPam);
            }
            else if (pCursor)
            {
                SwPaM *const pCursorPam = pCursor->GetPaM();
                *pCursorPam->GetPoint() = *aPam.GetPoint();
                pCursorPam->DeleteMark();
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
        m_pImpl->m_pDoc->getIDocumentContentOperations().InsertString(
                aPam, OUString(cIns), nInsertFlags);
    }

    if (bAbsorb)
    {
        const uno::Reference<lang::XUnoTunnel> xRangeTunnel(
                xTextRange, uno::UNO_QUERY);
        SwXTextRange *const pRange =
            ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
        OTextCursorHelper *const pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);

        SwCursor aCursor(*aPam.GetPoint(), nullptr, false);
        SwUnoCursorHelper::SelectPam(aCursor, true);
        aCursor.Left(1, CRSR_SKIP_CHARS, false, false);
        // here, the PaM needs to be moved:
        if (pRange)
        {
            pRange->SetPositions(aCursor);
        }
        else
        {
            SwPaM *const pUnoCursor = pCursor->GetPaM();
            *pUnoCursor->GetPoint() = *aCursor.GetPoint();
            if (aCursor.HasMark())
            {
                pUnoCursor->SetMark();
                *pUnoCursor->GetMark() = *aCursor.GetMark();
            }
            else
            {
                pUnoCursor->DeleteMark();
            }
        }
    }
}

void SAL_CALL
SwXText::insertTextContent(
        const uno::Reference< text::XTextRange > & xRange,
        const uno::Reference< text::XTextContent > & xContent,
        sal_Bool bAbsorb)
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
        aIllegal.Message = "second parameter invalid";
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
        aPam.GetNode().FindSttNodeByType(eSearchNodeType);

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
        pTextField = nullptr;

    const bool bAttribute = pBookmark || pDocumentIndexMark
        || pSection || pReferenceMark || pMeta || pTextField;

    if (bAbsorb && !bAttribute)
    {
        xRange->setString(OUString());
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
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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

    bool bRet = false;
    const uno::Reference<lang::XUnoTunnel> xSuccTunnel(xSuccessor,
            uno::UNO_QUERY);
    SwXTextSection *const pXSection =
            ::sw::UnoTunnelGetImplementation<SwXTextSection>(xSuccTunnel);
    SwXTextTable *const pXTable =
            ::sw::UnoTunnelGetImplementation<SwXTextTable>(xSuccTunnel);
    SwFrameFormat *const pTableFormat = (pXTable) ? pXTable->GetFrameFormat() : nullptr;
    SwTextNode * pTextNode = nullptr;
    if(pTableFormat && pTableFormat->GetDoc() == GetDoc())
    {
        SwTable *const pTable = SwTable::FindTable( pTableFormat );
        SwTableNode *const pTableNode = pTable->GetTableNode();

        const SwNodeIndex aTableIdx( *pTableNode, -1 );
        SwPosition aBefore(aTableIdx);
        bRet = GetDoc()->getIDocumentContentOperations().AppendTextNode( aBefore );
        pTextNode = aBefore.nNode.GetNode().GetTextNode();
    }
    else if (pXSection && pXSection->GetFormat() &&
            pXSection->GetFormat()->GetDoc() == GetDoc())
    {
        SwSectionFormat *const pSectFormat = pXSection->GetFormat();
        SwSectionNode *const pSectNode = pSectFormat->GetSectionNode();

        const SwNodeIndex aSectIdx( *pSectNode, -1 );
        SwPosition aBefore(aSectIdx);
        bRet = GetDoc()->getIDocumentContentOperations().AppendTextNode( aBefore );
        pTextNode = aBefore.nNode.GetNode().GetTextNode();
    }
    if (!bRet || !pTextNode)
    {
        throw lang::IllegalArgumentException();
    }
    pPara->attachToText(*this, *pTextNode);
}

void SAL_CALL
SwXText::insertTextContentAfter(
    const uno::Reference< text::XTextContent>& xNewContent,
    const uno::Reference< text::XTextContent>& xPredecessor)
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
    SwFrameFormat *const pTableFormat = (pXTable) ? pXTable->GetFrameFormat() : nullptr;
    bool bRet = false;
    SwTextNode * pTextNode = nullptr;
    if(pTableFormat && pTableFormat->GetDoc() == GetDoc())
    {
        SwTable *const pTable = SwTable::FindTable( pTableFormat );
        SwTableNode *const pTableNode = pTable->GetTableNode();

        SwEndNode *const pTableEnd = pTableNode->EndOfSectionNode();
        SwPosition aTableEnd(*pTableEnd);
        bRet = GetDoc()->getIDocumentContentOperations().AppendTextNode( aTableEnd );
        pTextNode = aTableEnd.nNode.GetNode().GetTextNode();
    }
    else if (pXSection && pXSection->GetFormat() &&
            pXSection->GetFormat()->GetDoc() == GetDoc())
    {
        SwSectionFormat *const pSectFormat = pXSection->GetFormat();
        SwSectionNode *const pSectNode = pSectFormat->GetSectionNode();
        SwEndNode *const pEnd = pSectNode->EndOfSectionNode();
        SwPosition aEnd(*pEnd);
        bRet = GetDoc()->getIDocumentContentOperations().AppendTextNode( aEnd );
        pTextNode = aEnd.nNode.GetNode().GetTextNode();
    }
    if (!bRet || !pTextNode)
    {
        throw lang::IllegalArgumentException();
    }
    pPara->attachToText(*this, *pTextNode);
}

void SAL_CALL
SwXText::removeTextContentBefore(
    const uno::Reference< text::XTextContent>& xSuccessor)
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if(!GetDoc())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }

    bool bRet = false;
    const uno::Reference<lang::XUnoTunnel> xSuccTunnel(xSuccessor,
            uno::UNO_QUERY);
    SwXTextSection *const pXSection =
            ::sw::UnoTunnelGetImplementation<SwXTextSection>(xSuccTunnel);
    SwXTextTable *const pXTable =
            ::sw::UnoTunnelGetImplementation<SwXTextTable>(xSuccTunnel);
    SwFrameFormat *const pTableFormat = (pXTable) ? pXTable->GetFrameFormat() : nullptr;
    if(pTableFormat && pTableFormat->GetDoc() == GetDoc())
    {
        SwTable *const pTable = SwTable::FindTable( pTableFormat );
        SwTableNode *const pTableNode = pTable->GetTableNode();

        const SwNodeIndex aTableIdx( *pTableNode, -1 );
        if(aTableIdx.GetNode().IsTextNode())
        {
            SwPaM aBefore(aTableIdx);
            bRet = GetDoc()->getIDocumentContentOperations().DelFullPara( aBefore );
        }
    }
    else if (pXSection && pXSection->GetFormat() &&
            pXSection->GetFormat()->GetDoc() == GetDoc())
    {
        SwSectionFormat *const pSectFormat = pXSection->GetFormat();
        SwSectionNode *const pSectNode = pSectFormat->GetSectionNode();

        const SwNodeIndex aSectIdx(  *pSectNode, -1 );
        if(aSectIdx.GetNode().IsTextNode())
        {
            SwPaM aBefore(aSectIdx);
            bRet = GetDoc()->getIDocumentContentOperations().DelFullPara( aBefore );
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
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if(!GetDoc())
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }

    bool bRet = false;
    const uno::Reference<lang::XUnoTunnel> xPredTunnel(xPredecessor,
            uno::UNO_QUERY);
    SwXTextSection *const pXSection =
            ::sw::UnoTunnelGetImplementation<SwXTextSection>(xPredTunnel);
    SwXTextTable *const pXTable =
            ::sw::UnoTunnelGetImplementation<SwXTextTable>(xPredTunnel);
    SwFrameFormat *const pTableFormat = (pXTable) ? pXTable->GetFrameFormat() : nullptr;
    if(pTableFormat && pTableFormat->GetDoc() == GetDoc())
    {
        SwTable *const pTable = SwTable::FindTable( pTableFormat );
        SwTableNode *const pTableNode = pTable->GetTableNode();
        SwEndNode *const pTableEnd = pTableNode->EndOfSectionNode();

        const SwNodeIndex aTableIdx( *pTableEnd, 1 );
        if(aTableIdx.GetNode().IsTextNode())
        {
            SwPaM aPaM(aTableIdx);
            bRet = GetDoc()->getIDocumentContentOperations().DelFullPara( aPaM );
        }
    }
    else if (pXSection && pXSection->GetFormat() &&
            pXSection->GetFormat()->GetDoc() == GetDoc())
    {
        SwSectionFormat *const pSectFormat = pXSection->GetFormat();
        SwSectionNode *const pSectNode = pSectFormat->GetSectionNode();
        SwEndNode *const pEnd = pSectNode->EndOfSectionNode();
        const SwNodeIndex aSectIdx(  *pEnd, 1 );
        if(aSectIdx.GetNode().IsTextNode())
        {
            SwPaM aAfter(aSectIdx);
            bRet = GetDoc()->getIDocumentContentOperations().DelFullPara( aAfter );
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
throw (container::NoSuchElementException, uno::RuntimeException, std::exception)
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
SwXText::getText() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const uno::Reference< text::XText > xRet(this);
    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::getStart() throw (uno::RuntimeException, std::exception)
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
SwXText::getEnd() throw (uno::RuntimeException, std::exception)
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

OUString SAL_CALL SwXText::getString() throw (uno::RuntimeException, std::exception)
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
SwXText::setString(const OUString& rString) throw (uno::RuntimeException, std::exception)
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

    GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_START, nullptr);
    //insert an empty paragraph at the start and at the end to ensure that
    //all tables and sections can be removed by the selecting text::XTextCursor
    if (CURSOR_META != m_pImpl->m_eType)
    {
        SwPosition aStartPos(*pStartNode);
        const SwEndNode* pEnd = pStartNode->EndOfSectionNode();
        SwNodeIndex aEndIdx(*pEnd);
        --aEndIdx;
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
            GetDoc()->getIDocumentContentOperations().AppendTextNode( aStartPos );
            SwPosition aEndPos(aEndIdx.GetNode());
            SwPaM aPam(aEndPos);
            GetDoc()->getIDocumentContentOperations().AppendTextNode( *aPam.Start() );
        }
    }

    const uno::Reference< text::XTextCursor > xRet = CreateCursor();
    if(!xRet.is())
    {
        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_END, nullptr);
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }
    xRet->gotoEnd(sal_True);
    xRet->setString(rString);
    GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_END, nullptr);
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
        pOwnCursor->GetPaM()->GetNode().StartOfSectionNode();
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

    const SwNode& rSrcNode = rPaM.GetNode();
    const SwStartNode* pTmp = rSrcNode.FindSttNodeByType(eSearchNodeType);

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
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
SwXText::getPropertySetInfo() throw(uno::RuntimeException, std::exception)
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
    uno::RuntimeException, std::exception)
{
    throw lang::IllegalArgumentException();
}

uno::Any SAL_CALL
SwXText::getPropertyValue(
    const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
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
        aExcept.Message = "Unknown property: " + rPropertyName;
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
            const SwRedlineTable& rRedTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
            const size_t nRedTableCount = rRedTable.size();
            if (nRedTableCount > 0)
            {
                SwStartNode const*const pStartNode = GetStartNode();
                const sal_uLong nOwnIndex = pStartNode->EndOfSectionIndex();
                for (size_t nRed = 0; nRed < nRedTableCount; ++nRed)
                {
                    SwRangeRedline const*const pRedline = rRedTable[nRed];
                    SwPosition const*const pRedStart = pRedline->Start();
                    const SwNodeIndex nRedNode = pRedStart->nNode;
                    if (nOwnIndex == nRedNode.GetIndex())
                    {
                        aRet <<= SwXRedlinePortion::CreateRedlineProperties(
                                *pRedline, true);
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
    uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXText::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXText::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXText::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXText::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    OSL_FAIL("SwXText::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXText::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
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
throw (uno::RuntimeException, std::exception)
{
    return ::sw::UnoTunnelImpl<SwXText>(rId, this);
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::finishParagraph(
        const uno::Sequence< beans::PropertyValue > & rProperties)
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    return m_pImpl->finishOrAppendParagraph(true, rProperties, uno::Reference< text::XTextRange >());
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::finishParagraphInsert(
        const uno::Sequence< beans::PropertyValue > & rProperties,
        const uno::Reference< text::XTextRange >& xInsertPosition)
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    return m_pImpl->finishOrAppendParagraph(true, rProperties, xInsertPosition);
}

uno::Reference< text::XTextRange >
SwXText::Impl::finishOrAppendParagraph(
        const bool bFinish,
        const uno::Sequence< beans::PropertyValue > & rProperties,
        const uno::Reference< text::XTextRange >& xInsertPosition)
    throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
    m_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_START , nullptr);
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
    m_pDoc->getIDocumentContentOperations().AppendTextNode( *aPam.GetPoint() );
    // remove attributes from the previous paragraph
    m_pDoc->ResetAttrs(aPam);
    // in case of finishParagraph the PaM needs to be moved to the
    // previous paragraph
    if (bFinish)
    {
        aPam.Move( fnMoveBackward, fnGoNode );
    }

    try
    {
        SfxItemPropertySet const*const pParaPropSet =
            aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARAGRAPH);

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
    catch (const uno::Exception& rEx)
    {
        sMessage = rEx.Message;
        bRuntimeException = true;
    }

    m_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_END, nullptr);
    if (bIllegalException || bRuntimeException)
    {
        m_pDoc->GetIDocumentUndoRedo().Undo();
        if (bIllegalException)
        {
            lang::IllegalArgumentException aEx;
            aEx.Message = sMessage;
            throw aEx;
        }
        else
        {
            uno::RuntimeException aEx;
            aEx.Message = sMessage;
            throw aEx;
        }
    }
    SwTextNode *const pTextNode( aPam.Start()->nNode.GetNode().GetTextNode() );
    OSL_ENSURE(pTextNode, "no SwTextNode?");
    if (pTextNode)
    {
        xRet.set(SwXParagraph::CreateXParagraph(*m_pDoc, pTextNode, &m_rThis),
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
    throw (lang::IllegalArgumentException, beans::UnknownPropertyException,
           beans::PropertyVetoException,
           uno::RuntimeException, std::exception)
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
    m_pImpl->m_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, nullptr);

    auto& rCursor(pTextCursor->GetCursor());
    m_pImpl->m_pDoc->DontExpandFormat( *rCursor.Start() );

    if (!rText.isEmpty())
    {
        const sal_Int32 nContentPos = rCursor.GetPoint()->nContent.GetIndex();
        SwUnoCursorHelper::DocInsertStringSplitCR(
            *m_pImpl->m_pDoc, rCursor, rText, false);
        SwUnoCursorHelper::SelectPam(rCursor, true);
        rCursor.GetPoint()->nContent = nContentPos;
    }

    try
    {
      SfxItemPropertySet const*const pCursorPropSet =
          aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR);
      SwUnoCursorHelper::SetPropertyValues(rCursor, *pCursorPropSet,
                                           rCharacterAndParagraphProperties,
                                           SetAttrMode::NOFORMATATTR);
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
    m_pImpl->m_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, nullptr);
    if (bIllegalException || bRuntimeException)
    {
        m_pImpl->m_pDoc->GetIDocumentUndoRedo().Undo();
        if (bIllegalException)
        {
            lang::IllegalArgumentException aEx;
            aEx.Message = sMessage;
            throw aEx;
        }
        else
        {
            uno::RuntimeException aEx;
            aEx.Message = sMessage;
            throw aEx;
        }
    }
    xRet = new SwXTextRange(rCursor, this);
    return xRet;
}

// Append text portions at the end of the last paragraph of the text interface.
// Support of import filters.
uno::Reference< text::XTextRange > SAL_CALL
SwXText::appendTextPortion(
        const OUString& rText,
        const uno::Sequence< beans::PropertyValue > &
            rCharacterAndParagraphProperties)
throw (lang::IllegalArgumentException, beans::UnknownPropertyException,
       beans::PropertyVetoException, uno::RuntimeException, std::exception)
{
    // Right now this doesn't need a guard, as it's just calling the insert
    // version, that has it already.
    uno::Reference<text::XTextRange> xInsertPosition = getEnd();
    return insertTextPortion(rText, rCharacterAndParagraphProperties, xInsertPosition);
}

// enable inserting/appending text contents like graphic objects, shapes and so on to
// support import filters
uno::Reference< text::XTextRange > SAL_CALL
SwXText::insertTextContentWithProperties(
    const uno::Reference< text::XTextContent >& xTextContent,
    const uno::Sequence< beans::PropertyValue >&
        rCharacterAndParagraphProperties,
    const uno::Reference< text::XTextRange >& xInsertPosition)
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!IsValid())
    {
        throw  uno::RuntimeException();
    }

    m_pImpl->m_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, nullptr);

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
        catch (const uno::Exception& e)
        {
            m_pImpl->m_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, nullptr);
            lang::WrappedTargetRuntimeException wrapped;
            wrapped.TargetException <<= e;
            throw wrapped;
        }
    }
    m_pImpl->m_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, nullptr);
    return xInsertPosition;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::appendTextContent(
    const uno::Reference< text::XTextContent >& xTextContent,
    const uno::Sequence< beans::PropertyValue >&
        rCharacterAndParagraphProperties)
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
    throw (lang::IllegalArgumentException, beans::UnknownPropertyException,
           beans::PropertyVetoException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
    {
        throw  uno::RuntimeException();
    }
    uno::Reference< text::XTextContent > xRet;
    SwUnoInternalPaM aStartPam(*GetDoc());
    std::unique_ptr< SwUnoInternalPaM > pEndPam(new SwUnoInternalPaM(*GetDoc()));
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

    m_pImpl->m_pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );
    bool bIllegalException = false;
    bool bRuntimeException = false;
    OUString sMessage;
    SwStartNode* pStartStartNode = aStartPam.GetNode().StartOfSectionNode();
    while (pStartStartNode && pStartStartNode->IsSectionNode())
    {
        pStartStartNode = pStartStartNode->StartOfSectionNode();
    }
    SwStartNode* pEndStartNode = pEndPam->GetNode().StartOfSectionNode();
    while (pEndStartNode && pEndStartNode->IsSectionNode())
    {
        pEndStartNode = pEndStartNode->StartOfSectionNode();
    }
    bool bParaAfterInserted = false;
    bool bParaBeforeInserted = false;
    if (
        pStartStartNode && pEndStartNode &&
        (pStartStartNode != pEndStartNode || pStartStartNode != GetStartNode())
       )
    {
        // todo: if the start/end is in a table then insert a paragraph
        // before/after, move the start/end nodes, then convert and
        // remove the additional paragraphs in the end
        SwTableNode * pStartTableNode(nullptr);
        if (pStartStartNode->GetStartNodeType() == SwTableBoxStartNode)
        {
            pStartTableNode = pStartStartNode->FindTableNode();
            // Is it the same table start node than the end?
            SwTableNode *const pEndStartTableNode(pEndStartNode->FindTableNode());
            while (pEndStartTableNode && pStartTableNode &&
                   pEndStartTableNode->GetIndex() < pStartTableNode->GetIndex())
            {
                SwStartNode* pStartStartTableNode = pStartTableNode->StartOfSectionNode();
                pStartTableNode = pStartStartTableNode->FindTableNode();
            }
        }
        if (pStartTableNode)
        {
            const SwNodeIndex aTableIdx(  *pStartTableNode, -1 );
            SwPosition aBefore(aTableIdx);
            bParaBeforeInserted = GetDoc()->getIDocumentContentOperations().AppendTextNode( aBefore );
            aStartPam.DeleteMark();
            *aStartPam.GetPoint() = aBefore;
            pStartStartNode = aStartPam.GetNode().StartOfSectionNode();
        }
        if (pEndStartNode->GetStartNodeType() == SwTableBoxStartNode)
        {
            SwTableNode *const pEndTableNode = pEndStartNode->FindTableNode();
            SwEndNode *const pTableEnd = pEndTableNode->EndOfSectionNode();
            SwPosition aTableEnd(*pTableEnd);
            bParaAfterInserted = GetDoc()->getIDocumentContentOperations().AppendTextNode( aTableEnd );
            pEndPam->DeleteMark();
            *pEndPam->GetPoint() = aTableEnd;
            pEndStartNode = pEndPam->GetNode().StartOfSectionNode();
        }
        // now we should have the positions in the same hierarchy
        if ((pStartStartNode != pEndStartNode) ||
            (pStartStartNode != GetStartNode()))
        {
            // if not - remove the additional paragraphs and throw
            if (bParaBeforeInserted)
            {
                SwCursor aDelete(*aStartPam.GetPoint(), nullptr, false);
                *aStartPam.GetPoint() = // park it because node is deleted
                    SwPosition(GetDoc()->GetNodes().GetEndOfContent());
                aDelete.MovePara(fnParaCurr, fnParaStart);
                aDelete.SetMark();
                aDelete.MovePara(fnParaCurr, fnParaEnd);
                GetDoc()->getIDocumentContentOperations().DelFullPara(aDelete);
            }
            if (bParaAfterInserted)
            {
                SwCursor aDelete(*pEndPam->GetPoint(), nullptr, false);
                *pEndPam->GetPoint() = // park it because node is deleted
                    SwPosition(GetDoc()->GetNodes().GetEndOfContent());
                aDelete.MovePara(fnParaCurr, fnParaStart);
                aDelete.SetMark();
                aDelete.MovePara(fnParaCurr, fnParaEnd);
                GetDoc()->getIDocumentContentOperations().DelFullPara(aDelete);
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
        bParaAfterInserted = GetDoc()->getIDocumentContentOperations().AppendTextNode( aEnd );
        pEndPam->DeleteMark();
        *pEndPam->GetPoint() = aEnd;
    }
    aStartPam.SetMark();
    *aStartPam.End() = *pEndPam->End();
    pEndPam.reset(nullptr);

    // see if there are frames already anchored to this node
    std::set<OUString> aAnchoredFrames;
    // for shapes, we have to work with the SdrObjects, as unique name is not guaranteed in their frame format
    std::set<const SdrObject*> aAnchoredShapes;
    for (size_t i = 0; i < m_pImpl->m_pDoc->GetSpzFrameFormats()->size(); ++i)
    {
        const SwFrameFormat* pFrameFormat = (*m_pImpl->m_pDoc->GetSpzFrameFormats())[i];
        const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
        if ((FLY_AT_PARA == rAnchor.GetAnchorId() || FLY_AT_CHAR == rAnchor.GetAnchorId()) &&
                aStartPam.Start()->nNode.GetIndex() <= rAnchor.GetContentAnchor()->nNode.GetIndex() &&
                aStartPam.End()->nNode.GetIndex() >= rAnchor.GetContentAnchor()->nNode.GetIndex())
        {
            if (pFrameFormat->Which() == RES_DRAWFRMFMT)
                aAnchoredShapes.insert(pFrameFormat->FindSdrObject());
            else
                aAnchoredFrames.insert(pFrameFormat->GetName());
        }
    }

    const uno::Reference<text::XTextFrame> xNewFrame(
            SwXTextFrame::CreateXTextFrame(*m_pImpl->m_pDoc, nullptr));
    SwXTextFrame& rNewFrame = dynamic_cast<SwXTextFrame&>(*xNewFrame.get());
    rNewFrame.SetSelection( aStartPam );
    try
    {
        const beans::PropertyValue* pValues = rFrameProperties.getConstArray();
        for (sal_Int32 nProp = 0; nProp < rFrameProperties.getLength(); ++nProp)
        {
            rNewFrame.SwXFrame::setPropertyValue(
                    pValues[nProp].Name, pValues[nProp].Value);
        }

        {   // has to be in a block to remove the SwIndexes before
            // DelFullPara is called
            const uno::Reference< text::XTextRange> xInsertTextRange =
                new SwXTextRange(aStartPam, this);
            aStartPam.DeleteMark(); // mark position node may be deleted!
            rNewFrame.attach( xInsertTextRange );
            rNewFrame.setName(m_pImpl->m_pDoc->GetUniqueFrameName());
        }

        SwTextNode *const pTextNode(aStartPam.GetNode().GetTextNode());
        OSL_ASSERT(pTextNode);
        if (!pTextNode || !pTextNode->Len()) // don't remove if it contains text!
        {
            {   // has to be in a block to remove the SwIndexes before
                // DelFullPara is called
                SwPaM aMovePam( aStartPam.GetNode() );
                if (aMovePam.Move( fnMoveForward, fnGoContent ))
                {
                    // move the anchor to the next paragraph
                    SwFormatAnchor aNewAnchor(rNewFrame.GetFrameFormat()->GetAnchor());
                    aNewAnchor.SetAnchor( aMovePam.Start() );
                    m_pImpl->m_pDoc->SetAttr(
                        aNewAnchor, *rNewFrame.GetFrameFormat() );

                    // also move frames anchored to us
                    for (size_t i = 0; i < m_pImpl->m_pDoc->GetSpzFrameFormats()->size(); ++i)
                    {
                        SwFrameFormat* pFrameFormat = (*m_pImpl->m_pDoc->GetSpzFrameFormats())[i];
                        if (aAnchoredFrames.find(pFrameFormat->GetName()) != aAnchoredFrames.end() || aAnchoredShapes.find(pFrameFormat->FindSdrObject()) != aAnchoredShapes.end())
                        {
                            // copy the anchor to the next paragraph
                            SwFormatAnchor aAnchor(pFrameFormat->GetAnchor());
                            aAnchor.SetAnchor(aMovePam.Start());
                            m_pImpl->m_pDoc->SetAttr(aAnchor, *pFrameFormat);
                        }
                    }
                }
            }
            m_pImpl->m_pDoc->getIDocumentContentOperations().DelFullPara(aStartPam);
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
    xRet = &rNewFrame;
    if (bParaBeforeInserted || bParaAfterInserted)
    {
        const uno::Reference<text::XTextCursor> xFrameTextCursor =
            rNewFrame.createTextCursor();
        const uno::Reference<XUnoTunnel> xTunnel(xFrameTextCursor,
                uno::UNO_QUERY);
        SwXTextCursor *const pFrameCursor =
            ::sw::UnoTunnelGetImplementation<SwXTextCursor>(xTunnel);
        if (bParaBeforeInserted)
        {
            // todo: remove paragraph before frame
            m_pImpl->m_pDoc->getIDocumentContentOperations().DelFullPara(*pFrameCursor->GetPaM());
        }
        if (bParaAfterInserted)
        {
            xFrameTextCursor->gotoEnd(sal_False);
            if (!bParaBeforeInserted)
                m_pImpl->m_pDoc->getIDocumentContentOperations().DelFullPara(*pFrameCursor->GetPaM());
            else
            {
                // In case the frame has a table only, the cursor points to the end of the first cell of the table.
                SwPaM aPaM(*pFrameCursor->GetPaM()->GetNode().FindSttNodeByType(SwFlyStartNode)->EndOfSectionNode());
                // Now we have the end of the frame -- the node before that will be the paragraph we want to remove.
                --aPaM.GetPoint()->nNode;
                m_pImpl->m_pDoc->getIDocumentContentOperations().DelFullPara(aPaM);
            }
        }
    }

    m_pImpl->m_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_END, nullptr);
    if (bIllegalException || bRuntimeException)
    {
        m_pImpl->m_pDoc->GetIDocumentUndoRedo().Undo();
        if (bIllegalException)
        {
            lang::IllegalArgumentException aEx;
            aEx.Message = sMessage;
            throw aEx;
        }
        else
        {
            uno::RuntimeException aEx;
            aEx.Message = sMessage;
            throw aEx;
        }
    }
    return xRet;
}

// Move previously imported paragraphs into a new text table.
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

void SwXText::Impl::ConvertCell(
    const uno::Sequence< uno::Reference< text::XTextRange > > & rCell,
    ::std::vector<SwNodeRange> & rRowNodes,
    SwNodeRange *const pLastCell,
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
        SwNodes::ExpandRangeForTableBox(aTmpRange);

    if (pCorrectedRange != nullptr)
    {
        SwPaM aNewStartPaM(pCorrectedRange->aStart, 0);
        aStartCellPam = aNewStartPaM;

        sal_Int32 nEndLen = 0;
        SwTextNode * pTextNode = pCorrectedRange->aEnd.GetNode().GetTextNode();
        if (pTextNode != nullptr)
            nEndLen = pTextNode->Len();

        SwPaM aNewEndPaM(pCorrectedRange->aEnd, nEndLen);
        aEndCellPam = aNewEndPaM;

        delete pCorrectedRange;
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
    if (!pLastCell) // first cell?
    {
        // align the beginning - if necessary
        if (aStartCellPam.Start()->nContent.GetIndex())
        {
            m_pDoc->getIDocumentContentOperations().SplitNode(*aStartCellPam.Start(), false);
        }
    }
    else
    {
        // check the predecessor
        const sal_uLong nStartCellNodeIndex =
            aStartCellPam.Start()->nNode.GetIndex();
        const sal_uLong nLastNodeEndIndex = pLastCell->aEnd.GetIndex();
        if (nLastNodeEndIndex == nStartCellNodeIndex)
        {
            // same node as predecessor then equal nContent?
            if (0 != aStartCellPam.Start()->nContent.GetIndex())
            {
                rbExcept = true;
            }
            else
            {
                m_pDoc->getIDocumentContentOperations().SplitNode(*aStartCellPam.Start(), false);
                sal_uLong const nNewIndex(aStartCellPam.Start()->nNode.GetIndex());
                if (nNewIndex != nStartCellNodeIndex)
                {
                    // aStartCellPam now points to the 2nd node
                    // the last cell may *also* point to 2nd node now - fix it!
                    assert(nNewIndex == nStartCellNodeIndex + 1);
                    if (pLastCell->aEnd.GetIndex() == nNewIndex)
                    {
                        --pLastCell->aEnd;
                        if (pLastCell->aStart.GetIndex() == nNewIndex)
                        {
                            --pLastCell->aStart;
                        }
                    }
                }
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
            aEndCellPam.End()->nNode.GetNode().GetTextNode()->Len())
    {
        m_pDoc->getIDocumentContentOperations().SplitNode(*aEndCellPam.End(), false);
        // take care that the new start/endcell is moved to the right position
        // aStartCellPam has to point to the start of the new (previous) node
        // aEndCellPam has to point to the end of the new (previous) node
        aStartCellPam.DeleteMark();
        aStartCellPam.Move(fnMoveBackward, fnGoNode);
        aStartCellPam.GetPoint()->nContent = 0;
        aEndCellPam.DeleteMark();
        aEndCellPam.Move(fnMoveBackward, fnGoNode);
        aEndCellPam.GetPoint()->nContent =
            aEndCellPam.GetNode().GetTextNode()->Len();
    }

    assert(aStartCellPam.Start()->nContent.GetIndex() == 0);
    assert(aEndCellPam.End()->nContent.GetIndex() == aEndCellPam.End()->nNode.GetNode().GetTextNode()->Len());
    SwNodeRange aCellRange(aStartCellPam.Start()->nNode,
            aEndCellPam.End()->nNode);
    rRowNodes.push_back(aCellRange); // note: invalidates pLastCell!
}

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
                sNames += rDebugCellProperties[nDebugProperty].Name + "-";
            }
            sNames += "+";
        }
        sNames += "|";
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
            bool bMerge = false;
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
                try
                {
                    const beans::PropertyState state = xCellTextPropState->getPropertyState(rName);
                    if (state == beans::PropertyState_DEFAULT_VALUE)
                    {
                        const uno::Reference< beans::XPropertySet >
                            xCellTextProps(xCellCurs, uno::UNO_QUERY);
                        xCellTextProps->setPropertyValue(rName, rValue);
                    }
                }
                catch (const uno::Exception& e)
                {
                    SAL_WARN( "sw.uno", "Exception when getting PropertyState: "
                        + rName + ". Message: " + e.Message );
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
                    UNO_NAME_ROW_SPAN,
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
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
    {
        throw  uno::RuntimeException();
    }

    IDocumentRedlineAccess & rIDRA(m_pImpl->m_pDoc->getIDocumentRedlineAccess());
    if (!IDocumentRedlineAccess::IsShowChanges(rIDRA.GetRedlineMode()))
    {
        throw uno::RuntimeException(
            "cannot convertToTable if tracked changes are hidden!");
    }

    //at first collect the text ranges as SwPaMs
    const uno::Sequence< uno::Sequence< uno::Reference< text::XTextRange > > >*
        pTableRanges = rTableRanges.getConstArray();
    std::vector< std::vector<SwNodeRange> > aTableNodes;
    bool bExcept = false;
    for (sal_Int32 nRow = 0; !bExcept && (nRow < rTableRanges.getLength());
            ++nRow)
    {
        std::vector<SwNodeRange> aRowNodes;
        const uno::Sequence< uno::Reference< text::XTextRange > >* pRow =
            pTableRanges[nRow].getConstArray();
        const sal_Int32 nCells(pTableRanges[nRow].getLength());

        for (sal_Int32 nCell = 0; nCell < nCells; ++nCell)
        {
            SwNodeRange *pLastCell;
            if (nCell == 0 && nRow == 0)
            {
                pLastCell = nullptr;
            }
            else
            {
                std::vector<SwNodeRange>& rRowOfPrevCell = nCell ? aRowNodes : *aTableNodes.rbegin();
                pLastCell = !rRowOfPrevCell.empty() ? &*rRowOfPrevCell.rbegin() : nullptr;
            }
            m_pImpl->ConvertCell(pRow[nCell], aRowNodes, pLastCell, bExcept);
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

    uno::Reference<text::XTextTable> const xRet =
        SwXTextTable::CreateXTextTable(pTable->GetFrameFormat());
    uno::Reference<beans::XPropertySet> const xPrSet(xRet, uno::UNO_QUERY);
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

        uno::Reference<table::XCellRange> const xCR(xRet, uno::UNO_QUERY_THROW);
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
                    xCR->getCellByPosition(nCell, nRow),
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

    assert(SwTable::FindTable(pTable->GetFrameFormat()) == pTable);
    assert(pTable->GetFrameFormat() ==
            dynamic_cast<SwXTextTable&>(*xRet.get()).GetFrameFormat());
    return xRet;
}

void SAL_CALL
SwXText::copyText(
    const uno::Reference< text::XTextCopy >& xSource )
throw (uno::RuntimeException, std::exception)
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
    m_pImpl->m_pDoc->getIDocumentContentOperations().CopyRange( *pCursor->GetPaM(), rPos, /*bCopyAll=*/false, /*bCheckPos=*/true );
}

SwXBodyText::SwXBodyText(SwDoc *const pDoc)
    : SwXText(pDoc, CURSOR_BODY)
{
}

SwXBodyText::~SwXBodyText()
{
}

OUString SAL_CALL
SwXBodyText::getImplementationName() throw (uno::RuntimeException, std::exception)
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
throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXBodyText::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesBodyText, g_ServicesBodyText);
}

uno::Any SAL_CALL
SwXBodyText::queryAggregation(const uno::Type& rType)
throw (uno::RuntimeException, std::exception)
{
    uno::Any aRet;
    if (rType == cppu::UnoType<container::XEnumerationAccess>::get())
    {
        aRet <<= uno::Reference< container::XEnumerationAccess >(this);
    }
    else if (rType == cppu::UnoType<container::XElementAccess>::get())
    {
        aRet <<= uno::Reference< container::XElementAccess >(this);
    }
    else if (rType == cppu::UnoType<lang::XServiceInfo>::get())
    {
        aRet <<= uno::Reference< lang::XServiceInfo >(this);
    }
    else
    {
        aRet = SwXText::queryInterface( rType );
    }
    if(aRet.getValueType() == cppu::UnoType<void>::get())
    {
        aRet = OWeakAggObject::queryAggregation( rType );
    }
    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL
SwXBodyText::getTypes() throw (uno::RuntimeException, std::exception)
{
    const uno::Sequence< uno::Type > aTypes = SwXBodyText_Base::getTypes();
    const uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();
    return ::comphelper::concatSequences(aTypes, aTextTypes);
}

uno::Sequence< sal_Int8 > SAL_CALL
SwXBodyText::getImplementationId() throw (uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Any SAL_CALL
SwXBodyText::queryInterface(const uno::Type& rType)
throw (uno::RuntimeException, std::exception)
{
    const uno::Any ret = SwXText::queryInterface(rType);
    return (ret.getValueType() == cppu::UnoType<void>::get())
        ?   SwXBodyText_Base::queryInterface(rType)
        :   ret;
}

SwXTextCursor * SwXBodyText::CreateTextCursor(const bool bIgnoreTables)
{
    if(!IsValid())
    {
        return nullptr;
    }

    // the cursor has to skip tables contained in this text
    SwPaM aPam(GetDoc()->GetNodes().GetEndOfContent());
    aPam.Move( fnMoveBackward, fnGoDoc );
    if (!bIgnoreTables)
    {
        SwTableNode * pTableNode = aPam.GetNode().FindTableNode();
        SwContentNode * pCont = nullptr;
        while (pTableNode)
        {
            aPam.GetPoint()->nNode = *pTableNode->EndOfSectionNode();
            pCont = GetDoc()->GetNodes().GoNext(&aPam.GetPoint()->nNode);
            pTableNode = pCont->FindTableNode();
        }
        if (pCont)
        {
            aPam.GetPoint()->nContent.Assign(pCont, 0);
        }
    }
    return new SwXTextCursor(*GetDoc(), this, CURSOR_BODY, *aPam.GetPoint());
}

uno::Reference< text::XTextCursor > SAL_CALL
SwXBodyText::createTextCursor() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    const uno::Reference< text::XTextCursor > xRef(
            static_cast<text::XWordCursor*>(CreateTextCursor()) );
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
throw (uno::RuntimeException, std::exception)
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
        if ( !aPam.GetNode().GetTextNode() )
            throw uno::RuntimeException("Invalid text range" );

        SwNode& rNode = GetDoc()->GetNodes().GetEndOfContent();

        SwStartNode* p1 = aPam.GetNode().StartOfSectionNode();
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
throw (uno::RuntimeException, std::exception)
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
    auto pUnoCursor(GetDoc()->CreateUnoCursor(aPos));
    pUnoCursor->Move(fnMoveBackward, fnGoDoc);
    return SwXParagraphEnumeration::Create(this, pUnoCursor, CURSOR_BODY);
}

uno::Type SAL_CALL
SwXBodyText::getElementType() throw (uno::RuntimeException, std::exception)
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL
SwXBodyText::hasElements() throw (uno::RuntimeException, std::exception)
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

class SwXHeadFootText::Impl
    : public SwClient
{

public:

    bool                        m_bIsHeader;

    Impl(   SwXHeadFootText & /*rThis*/,
            SwFrameFormat & rHeadFootFormat, const bool bIsHeader)
        : SwClient(& rHeadFootFormat)
        , m_bIsHeader(bIsHeader)
    {
    }

    SwFrameFormat * GetHeadFootFormat() const {
        return static_cast<SwFrameFormat*>(
                const_cast<SwModify*>(GetRegisteredIn()));
    }

    SwFrameFormat & GetHeadFootFormatOrThrow() {
        SwFrameFormat *const pFormat( GetHeadFootFormat() );
        if (!pFormat) {
            throw uno::RuntimeException("SwXHeadFootText: disposed or invalid", nullptr);
        }
        return *pFormat;
    }
protected:
    // SwClient
    virtual void Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew) override;

};

void SwXHeadFootText::Impl::Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

bool SwXHeadFootText::IsXHeadFootText(SwClient *const pClient)
{
    return dynamic_cast<SwXHeadFootText::Impl*>(pClient) !=  nullptr;
}

uno::Reference< text::XText >
SwXHeadFootText::CreateXHeadFootText(
        SwFrameFormat & rHeadFootFormat, const bool bIsHeader)
{
    // re-use existing SwXHeadFootText
    // #i105557#: do not iterate over the registered clients: race condition
    uno::Reference< text::XText > xText(rHeadFootFormat.GetXObject(),
            uno::UNO_QUERY);
    if (!xText.is())
    {
        SwXHeadFootText *const pXHFT(
                new SwXHeadFootText(rHeadFootFormat, bIsHeader));
        xText.set(pXHFT);
        rHeadFootFormat.SetXObject(xText);
    }
    return xText;
}

SwXHeadFootText::SwXHeadFootText(SwFrameFormat & rHeadFootFormat, const bool bIsHeader)
    : SwXText(rHeadFootFormat.GetDoc(),
            (bIsHeader) ? CURSOR_HEADER : CURSOR_FOOTER)
    , m_pImpl( new SwXHeadFootText::Impl(*this, rHeadFootFormat, bIsHeader) )
{
}

SwXHeadFootText::~SwXHeadFootText()
{
}

OUString SAL_CALL
SwXHeadFootText::getImplementationName() throw (uno::RuntimeException, std::exception)
{
    return OUString("SwXHeadFootText");
}

static char const*const g_ServicesHeadFootText[] =
{
    "com.sun.star.text.Text",
};

sal_Bool SAL_CALL SwXHeadFootText::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXHeadFootText::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
{
    return ::sw::GetSupportedServiceNamesImpl(
            SAL_N_ELEMENTS(g_ServicesHeadFootText),
            g_ServicesHeadFootText);
}

const SwStartNode *SwXHeadFootText::GetStartNode() const
{
    const SwStartNode *pSttNd = nullptr;
    SwFrameFormat *const pHeadFootFormat = m_pImpl->GetHeadFootFormat();
    if(pHeadFootFormat)
    {
        const SwFormatContent& rFlyContent = pHeadFootFormat->GetContent();
        if( rFlyContent.GetContentIdx() )
        {
            pSttNd = rFlyContent.GetContentIdx()->GetNode().GetStartNode();
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
SwXHeadFootText::getTypes() throw (uno::RuntimeException, std::exception)
{
    const uno::Sequence< uno::Type > aTypes = SwXHeadFootText_Base::getTypes();
    const uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();
    return ::comphelper::concatSequences(aTypes, aTextTypes);
}

uno::Sequence< sal_Int8 > SAL_CALL
SwXHeadFootText::getImplementationId() throw (uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Any SAL_CALL
SwXHeadFootText::queryInterface(const uno::Type& rType)
throw (uno::RuntimeException, std::exception)
{
    const uno::Any ret = SwXHeadFootText_Base::queryInterface(rType);
    return (ret.getValueType() == cppu::UnoType<void>::get())
        ?   SwXText::queryInterface(rType)
        :   ret;
}

uno::Reference< text::XTextCursor > SAL_CALL
SwXHeadFootText::createTextCursor() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwFrameFormat & rHeadFootFormat( m_pImpl->GetHeadFootFormatOrThrow() );

    uno::Reference< text::XTextCursor > xRet;
    const SwFormatContent& rFlyContent = rHeadFootFormat.GetContent();
    const SwNode& rNode = rFlyContent.GetContentIdx()->GetNode();
    SwPosition aPos(rNode);
    SwXTextCursor *const pXCursor = new SwXTextCursor(*GetDoc(), this,
            (m_pImpl->m_bIsHeader) ? CURSOR_HEADER : CURSOR_FOOTER, aPos);
    auto& rUnoCursor(pXCursor->GetCursor());
    rUnoCursor.Move(fnMoveForward, fnGoNode);

    // save current start node to be able to check if there is content
    // after the table - otherwise the cursor would be in the body text!
    SwStartNode const*const pOwnStartNode = rNode.FindSttNodeByType(
            (m_pImpl->m_bIsHeader) ? SwHeaderStartNode : SwFooterStartNode);
    // is there a table here?
    SwTableNode* pTableNode = rUnoCursor.GetNode().FindTableNode();
    SwContentNode* pCont = nullptr;
    while (pTableNode)
    {
        rUnoCursor.GetPoint()->nNode = *pTableNode->EndOfSectionNode();
        pCont = GetDoc()->GetNodes().GoNext(&rUnoCursor.GetPoint()->nNode);
        pTableNode = pCont->FindTableNode();
    }
    if (pCont)
    {
        rUnoCursor.GetPoint()->nContent.Assign(pCont, 0);
    }
    SwStartNode const*const pNewStartNode =
        rUnoCursor.GetNode().FindSttNodeByType(
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
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwFrameFormat & rHeadFootFormat( m_pImpl->GetHeadFootFormatOrThrow() );

    SwUnoInternalPaM aPam(*GetDoc());
    if (!::sw::XTextRangeToSwPaM(aPam, xTextPosition))
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = cInvalidObject;
        throw aRuntime;
    }

    uno::Reference< text::XTextCursor >  xRet;
    SwNode& rNode = rHeadFootFormat.GetContent().GetContentIdx()->GetNode();
    SwPosition aPos(rNode);
    SwPaM aHFPam(aPos);
    aHFPam.Move(fnMoveForward, fnGoNode);
    SwStartNode *const pOwnStartNode = aHFPam.GetNode().FindSttNodeByType(
            (m_pImpl->m_bIsHeader) ? SwHeaderStartNode : SwFooterStartNode);
    SwStartNode *const p1 = aPam.GetNode().FindSttNodeByType(
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
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwFrameFormat & rHeadFootFormat( m_pImpl->GetHeadFootFormatOrThrow() );

    uno::Reference< container::XEnumeration >  aRef;
    const SwFormatContent& rFlyContent = rHeadFootFormat.GetContent();
    const SwNode& rNode = rFlyContent.GetContentIdx()->GetNode();
    SwPosition aPos(rNode);
    auto pUnoCursor(GetDoc()->CreateUnoCursor(aPos));
    pUnoCursor->Move(fnMoveForward, fnGoNode);
    return SwXParagraphEnumeration::Create(this, pUnoCursor, (m_pImpl->m_bIsHeader) ? CURSOR_HEADER : CURSOR_FOOTER);
}

uno::Type SAL_CALL
SwXHeadFootText::getElementType() throw (uno::RuntimeException, std::exception)
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL SwXHeadFootText::hasElements() throw (uno::RuntimeException, std::exception)
{
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
