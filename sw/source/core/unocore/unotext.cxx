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
#include <set>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>

#include <svl/listener.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/profilezone.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>

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
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unoparagraph.hxx>
#include <unocrsrhelper.hxx>
#include <docary.hxx>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <bookmark.hxx>
#include <redline.hxx>
#include <swundo.hxx>
#include <section.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <ndtxt.hxx>
#include <SwRewriter.hxx>
#include <strings.hrc>
#include <frameformats.hxx>
#include <unocontentcontrol.hxx>

using namespace ::com::sun::star;

constexpr OUString cInvalidObject = u"this object is invalid"_ustr;

class SwXText::Impl
{

public:
    SwXText &                   m_rThis;
    SfxItemPropertySet const&   m_rPropSet;
    const CursorType            m_eType;
    SwDoc *                     m_pDoc;
    bool                        m_bIsValid;

    Impl(   SwXText & rThis,
            SwDoc *const pDoc, const CursorType eType)
        : m_rThis(rThis)
        , m_rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT))
        , m_eType(eType)
        , m_pDoc(pDoc)
        , m_bIsValid(nullptr != pDoc)
    {
    }

    /// @throws lang::IllegalArgumentException
    /// @throws uno::RuntimeException
    rtl::Reference<SwXParagraph>
        finishOrAppendParagraph(
            const uno::Sequence< beans::PropertyValue >&
                rCharacterAndParagraphProperties,
            const uno::Reference< text::XTextRange >& xInsertPosition);

    /// @throws lang::IllegalArgumentException
    /// @throws uno::RuntimeException
    sal_Int16 ComparePositions(
            const uno::Reference<text::XTextRange>& xPos1,
            const uno::Reference<text::XTextRange>& xPos2);

    /// @throws lang::IllegalArgumentException
    /// @throws uno::RuntimeException
    bool CheckForOwnMember(const SwPaM & rPaM);

    void ConvertCell(
            const uno::Sequence< uno::Reference< text::XTextRange > > & rCell,
            std::vector<SwNodeRange> & rRowNodes,
            SwNodeRange *const pLastCell);

};

SwXText::SwXText(SwDoc *const pDoc, const CursorType eType)
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
{
    OSL_ENSURE(CursorType::Meta != m_pImpl->m_eType, "should not be called!");
    return false;
}

const SwStartNode *SwXText::GetStartNode() const
{
    return GetDoc()->GetNodes().GetEndOfContent().StartOfSectionNode();
}

uno::Reference< text::XTextCursor > SAL_CALL SwXText::createTextCursor()
{
    SolarMutexGuard aGuard;
    rtl::Reference<SwXTextCursor> xCursor = createXTextCursor();
    if (!xCursor.is())
        throw uno::RuntimeException(cInvalidObject);
    return static_cast<text::XWordCursor*>(xCursor.get());
}

rtl::Reference< SwXTextCursor >
SwXText::createXTextCursor()
{
    rtl::Reference< SwXTextCursor > xRet;
    if(IsValid())
    {
        SwNode& rNode = GetDoc()->GetNodes().GetEndOfContent();
        SwPosition aPos(rNode);
        xRet = new SwXTextCursor(*GetDoc(), this, m_pImpl->m_eType, aPos);
        xRet->gotoStart(false);
    }
    return xRet;
}

css::uno::Reference< css::text::XTextCursor > SAL_CALL SwXText::createTextCursorByRange(
        const ::css::uno::Reference< ::css::text::XTextRange >& aTextPosition )
{
    SolarMutexGuard aGuard;
    return static_cast<text::XWordCursor*>(createXTextCursorByRange(aTextPosition).get());
}


uno::Any SAL_CALL
SwXText::queryInterface(const uno::Type& rType)
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
SwXText::getTypes()
{
    static const uno::Sequence< uno::Type > aTypes {
        cppu::UnoType<text::XText>::get(),
        cppu::UnoType<text::XTextRangeCompare>::get(),
        cppu::UnoType<text::XRelativeTextContentInsert>::get(),
        cppu::UnoType<text::XRelativeTextContentRemove>::get(),
        cppu::UnoType<lang::XUnoTunnel>::get(),
        cppu::UnoType<beans::XPropertySet>::get(),
        cppu::UnoType<text::XTextPortionAppend>::get(),
        cppu::UnoType<text::XParagraphAppend>::get(),
        cppu::UnoType<text::XTextContentAppend>::get(),
        cppu::UnoType<text::XTextConvert>::get(),
        cppu::UnoType<text::XTextAppend>::get(),
        cppu::UnoType<text::XTextAppendAndConvert>::get()
    };
    return aTypes;
}

// belongs the range in the text ? insert it then.
void SAL_CALL
SwXText::insertString(const uno::Reference< text::XTextRange >& xTextRange,
    const OUString& rString, sal_Bool bAbsorb)
{
    SolarMutexGuard aGuard;
    comphelper::ProfileZone aZone("SwXText::insertString");

    if (!xTextRange.is())
    {
        throw uno::RuntimeException();
    }
    if (!GetDoc())
    {
        throw uno::RuntimeException();
    }
    SwXTextRange *const pRange = dynamic_cast<SwXTextRange*>(xTextRange.get());
    OTextCursorHelper *const pCursor = dynamic_cast<OTextCursorHelper*>(xTextRange.get());
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

    const SwStartNode* pTmp(pPam->GetPointNode().StartOfSectionNode());
    while (pTmp && pTmp->IsSectionNode())
    {
        pTmp = pTmp->StartOfSectionNode();
    }
    if (!pOwnStartNode || (pOwnStartNode != pTmp))
    {
        throw uno::RuntimeException();
    }

    bool bForceExpandHints( false );
    if (CursorType::Meta == m_pImpl->m_eType)
    {
        try
        {
            bForceExpandHints = CheckForOwnMemberMeta(*pPam, bAbsorb);
        }
        catch (const lang::IllegalArgumentException& iae)
        {
            // stupid method not allowed to throw iae
            css::uno::Any anyEx = cppu::getCaughtException();
            throw lang::WrappedTargetRuntimeException( iae.Message,
                            uno::Reference< uno::XInterface >(), anyEx );
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
                pTextCursor->DeleteAndInsert(rString, ::sw::DeleteAndInsertMode::ForceReplace
                    | (bForceExpandHints ? ::sw::DeleteAndInsertMode::ForceExpandHints : ::sw::DeleteAndInsertMode::Default));
            }
            else
            {
                xTextRange->setString(rString);
            }
        }
        else
        {
            pRange->DeleteAndInsert(rString, ::sw::DeleteAndInsertMode::ForceReplace
                | (bForceExpandHints ? ::sw::DeleteAndInsertMode::ForceExpandHints : ::sw::DeleteAndInsertMode::Default));
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
            m_pImpl->m_pDoc->ClearBoxNumAttrs(aPam.GetPoint()->GetNode());
            m_pImpl->m_pDoc->getIDocumentContentOperations().SplitNode(*aPam.GetPoint(), false);
            break;
        case text::ControlCharacter::APPEND_PARAGRAPH:
        {
            m_pImpl->m_pDoc->ClearBoxNumAttrs(aPam.GetPoint()->GetNode());
            m_pImpl->m_pDoc->getIDocumentContentOperations().AppendTextNode(*aPam.GetPoint());

            SwXTextRange *const pRange =
                dynamic_cast<SwXTextRange*>(xTextRange.get());
            OTextCursorHelper *const pCursor =
                dynamic_cast<OTextCursorHelper*>(xTextRange.get());
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

    if (!bAbsorb)
        return;

    SwXTextRange *const pRange =
        dynamic_cast<SwXTextRange*>(xTextRange.get());
    OTextCursorHelper *const pCursor =
        dynamic_cast<OTextCursorHelper*>(xTextRange.get());

    SwCursor aCursor(*aPam.GetPoint(), nullptr);
    SwUnoCursorHelper::SelectPam(aCursor, true);
    aCursor.Left(1);
    // here, the PaM needs to be moved:
    if (pRange)
    {
        pRange->SetPositions(aCursor);
    }
    else if (pCursor)
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

void SAL_CALL
SwXText::insertTextContent(
        const uno::Reference< text::XTextRange > & xRange,
        const uno::Reference< text::XTextContent > & xContent,
        sal_Bool bAbsorb)
{
    SolarMutexGuard aGuard;
    comphelper::ProfileZone aZone("SwXText::insertTextContent");

    if (!xRange.is())
        throw lang::IllegalArgumentException(u"first parameter invalid"_ustr, nullptr, 0);
    if (!xContent.is())
        throw lang::IllegalArgumentException(u"second parameter invalid"_ustr, nullptr, 1);
    if (!GetDoc())
        throw uno::RuntimeException(cInvalidObject);

    SwUnoInternalPaM aPam(*GetDoc());
    if (!::sw::XTextRangeToSwPaM(aPam, xRange))
        throw lang::IllegalArgumentException(u"first parameter invalid"_ustr, nullptr, 0);

    // first test if the range is at the right position, then call
    // xContent->attach
    const SwStartNode* pOwnStartNode = GetStartNode();
    SwStartNodeType eSearchNodeType = SwNormalStartNode;
    switch (m_pImpl->m_eType)
    {
        case CursorType::Frame:      eSearchNodeType = SwFlyStartNode;       break;
        case CursorType::TableText:    eSearchNodeType = SwTableBoxStartNode;  break;
        case CursorType::Footnote:   eSearchNodeType = SwFootnoteStartNode;  break;
        case CursorType::Header:     eSearchNodeType = SwHeaderStartNode;    break;
        case CursorType::Footer:     eSearchNodeType = SwFooterStartNode;    break;
        //case CURSOR_INVALID:
        //case CursorType::Body:
        default:
            break;
    }

    const SwStartNode* pTmp =
        aPam.GetPointNode().FindSttNodeByType(eSearchNodeType);

    // ignore SectionNodes
    while (pTmp && pTmp->IsSectionNode())
    {
        pTmp = pTmp->StartOfSectionNode();
    }
    // if the document starts with a section
    while (pOwnStartNode && pOwnStartNode->IsSectionNode())
    {
        pOwnStartNode = pOwnStartNode->StartOfSectionNode();
    }
    // this checks if (this) and xRange are in the same text::XText interface
    if (pOwnStartNode != pTmp)
        throw uno::RuntimeException(u"text interface and cursor not related"_ustr);

    const bool bForceExpandHints(CheckForOwnMemberMeta(aPam, bAbsorb));

    // special treatment for Contents that do not replace the range, but
    // instead are "overlaid"
    SwXDocumentIndexMark *const pDocumentIndexMark =
        dynamic_cast<SwXDocumentIndexMark*>(xContent.get());
    SwXTextSection *const pSection =
        dynamic_cast<SwXTextSection*>(xContent.get());
    SwXBookmark *const pBookmark =
        dynamic_cast<SwXBookmark*>(xContent.get());
    SwXReferenceMark *const pReferenceMark =
        dynamic_cast<SwXReferenceMark*>(xContent.get());
    SwXMeta *const pMeta = dynamic_cast<SwXMeta*>(xContent.get());
    auto* pContentControl = dynamic_cast<SwXContentControl*>(xContent.get());
    SwXTextField* pTextField = dynamic_cast<SwXTextField*>(xContent.get());
    if (pTextField && pTextField->GetServiceId() != SwServiceType::FieldTypeAnnotation)
        pTextField = nullptr;

    const bool bAttribute = pBookmark || pDocumentIndexMark
        || pSection || pReferenceMark || pMeta || pContentControl || pTextField;

    if (bAbsorb && !bAttribute)
    {
        if (SwXTextRange *const pRange = dynamic_cast<SwXTextRange*>(xRange.get()))
        {
            pRange->DeleteAndInsert(u"", ::sw::DeleteAndInsertMode::ForceReplace
                | (bForceExpandHints ? ::sw::DeleteAndInsertMode::ForceExpandHints : ::sw::DeleteAndInsertMode::Default));
        }
        else if (SwXTextCursor *const pCursor = dynamic_cast<SwXTextCursor*>(dynamic_cast<OTextCursorHelper*>(xRange.get())))
        {
            pCursor->DeleteAndInsert(u"", ::sw::DeleteAndInsertMode::ForceReplace
                | (bForceExpandHints ? ::sw::DeleteAndInsertMode::ForceExpandHints : ::sw::DeleteAndInsertMode::Default));
        }
        else
        {
            xRange->setString(OUString());
        }
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
{
    SolarMutexGuard aGuard;

    if(!GetDoc())
        throw uno::RuntimeException(cInvalidObject);

    SwXParagraph *const pPara = dynamic_cast<SwXParagraph*>(xNewContent.get());
    if (!pPara || !pPara->IsDescriptor() || !xSuccessor.is())
    {
        throw lang::IllegalArgumentException();
    }

    bool bRet = false;
    SwXTextSection *const pXSection = dynamic_cast<SwXTextSection*>(xSuccessor.get());
    SwXTextTable *const pXTable = dynamic_cast<SwXTextTable*>(xSuccessor.get());
    SwFrameFormat *const pTableFormat = pXTable ? pXTable->GetFrameFormat() : nullptr;
    SwTextNode * pTextNode = nullptr;
    if(pTableFormat && pTableFormat->GetDoc() == GetDoc())
    {
        SwTable *const pTable = SwTable::FindTable( pTableFormat );
        SwTableNode *const pTableNode = pTable->GetTableNode();

        const SwNodeIndex aTableIdx( *pTableNode, -1 );
        SwPosition aBefore(aTableIdx);
        bRet = GetDoc()->getIDocumentContentOperations().AppendTextNode( aBefore );
        pTextNode = aBefore.GetNode().GetTextNode();
    }
    else if (pXSection && pXSection->GetFormat() &&
            pXSection->GetFormat()->GetDoc() == GetDoc())
    {
        SwSectionFormat *const pSectFormat = pXSection->GetFormat();
        SwSectionNode *const pSectNode = pSectFormat->GetSectionNode();

        const SwNodeIndex aSectIdx( *pSectNode, -1 );
        SwPosition aBefore(aSectIdx);
        bRet = GetDoc()->getIDocumentContentOperations().AppendTextNode( aBefore );
        pTextNode = aBefore.GetNode().GetTextNode();
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
{
    SolarMutexGuard aGuard;

    if(!GetDoc())
    {
        throw uno::RuntimeException();
    }

    SwXParagraph *const pPara = dynamic_cast<SwXParagraph*>(xNewContent.get());
    if(!pPara || !pPara->IsDescriptor() || !xPredecessor.is())
    {
        throw lang::IllegalArgumentException();
    }

    SwXTextSection *const pXSection = dynamic_cast<SwXTextSection*>(xPredecessor.get());
    SwXTextTable *const pXTable = dynamic_cast<SwXTextTable*>(xPredecessor.get());
    SwFrameFormat *const pTableFormat = pXTable ? pXTable->GetFrameFormat() : nullptr;
    bool bRet = false;
    SwTextNode * pTextNode = nullptr;
    if(pTableFormat && pTableFormat->GetDoc() == GetDoc())
    {
        SwTable *const pTable = SwTable::FindTable( pTableFormat );
        SwTableNode *const pTableNode = pTable->GetTableNode();

        SwEndNode *const pTableEnd = pTableNode->EndOfSectionNode();
        SwPosition aTableEnd(*pTableEnd);
        bRet = GetDoc()->getIDocumentContentOperations().AppendTextNode( aTableEnd );
        pTextNode = aTableEnd.GetNode().GetTextNode();
    }
    else if (pXSection && pXSection->GetFormat() &&
            pXSection->GetFormat()->GetDoc() == GetDoc())
    {
        SwSectionFormat *const pSectFormat = pXSection->GetFormat();
        SwSectionNode *const pSectNode = pSectFormat->GetSectionNode();
        SwEndNode *const pEnd = pSectNode->EndOfSectionNode();
        SwPosition aEnd(*pEnd);
        bRet = GetDoc()->getIDocumentContentOperations().AppendTextNode( aEnd );
        pTextNode = aEnd.GetNode().GetTextNode();
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
{
    SolarMutexGuard aGuard;

    if(!GetDoc())
        throw uno::RuntimeException(cInvalidObject);

    bool bRet = false;
    SwXTextSection *const pXSection = dynamic_cast<SwXTextSection*>(xSuccessor.get());
    SwXTextTable *const pXTable = dynamic_cast<SwXTextTable*>(xSuccessor.get());
    SwFrameFormat *const pTableFormat = pXTable ? pXTable->GetFrameFormat() : nullptr;
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
{
    SolarMutexGuard aGuard;

    if(!GetDoc())
        throw uno::RuntimeException(cInvalidObject);

    bool bRet = false;
    SwXTextSection *const pXSection = dynamic_cast<SwXTextSection*>(xPredecessor.get());
    SwXTextTable *const pXTable = dynamic_cast<SwXTextTable*>(xPredecessor.get());
    SwFrameFormat *const pTableFormat = pXTable ? pXTable->GetFrameFormat() : nullptr;
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
{
    // forward: need no solar mutex here
    if(!xContent.is())
        throw uno::RuntimeException(u"first parameter invalid"_ustr);
    xContent->dispose();
}

uno::Reference< text::XText > SAL_CALL
SwXText::getText()
{
    SolarMutexGuard aGuard;
    comphelper::ProfileZone aZone("SwXText::getText");

    const uno::Reference< text::XText > xRet(this);
    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::getStart()
{
    SolarMutexGuard aGuard;

    const rtl::Reference< SwXTextCursor > xRef = createXTextCursor();
    if(!xRef.is())
        throw uno::RuntimeException(cInvalidObject);
    xRef->gotoStart(false);
    return static_cast<text::XWordCursor*>(xRef.get());
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::getEnd()
{
    SolarMutexGuard aGuard;

    const rtl::Reference< SwXTextCursor > xRef = createXTextCursor();
    if(!xRef.is())
        throw uno::RuntimeException(cInvalidObject);
    xRef->gotoEnd(false);
    return static_cast<text::XWordCursor*>(xRef.get());
}

OUString SAL_CALL SwXText::getString()
{
    SolarMutexGuard aGuard;

    const rtl::Reference< SwXTextCursor > xRet = createXTextCursor();
    if(!xRet.is())
    {
        SAL_WARN("sw.uno", "cursor was not created in getString() call. Returning empty string.");
        return OUString();
    }
    xRet->gotoEnd(true);
    return xRet->getString();
}

void SAL_CALL
SwXText::setString(const OUString& rString)
{
    SolarMutexGuard aGuard;

    if (!GetDoc())
        throw uno::RuntimeException(cInvalidObject);

    const SwStartNode* pStartNode = GetStartNode();
    if (!pStartNode)
    {
        throw uno::RuntimeException();
    }

    GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::START, nullptr);
    //insert an empty paragraph at the start and at the end to ensure that
    //all tables and sections can be removed by the selecting text::XTextCursor
    if (CursorType::Meta != m_pImpl->m_eType)
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
            if(rCurrentNode.GetNodeType() == SwNodeType::Section
                ||rCurrentNode.GetNodeType() == SwNodeType::Table)
            {
                bInsertNodes = true;
                break;
            }
        }
        while(aStartIdx < aEndIdx);
        if(bInsertNodes)
        {
            GetDoc()->getIDocumentContentOperations().AppendTextNode( aStartPos );
            SwPaM aPam(aEndIdx.GetNode());
            GetDoc()->getIDocumentContentOperations().AppendTextNode( *aPam.Start() );
        }
    }

    const rtl::Reference< SwXTextCursor > xRet = createXTextCursor();
    if(!xRet.is())
    {
        GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::END, nullptr);
        throw uno::RuntimeException(cInvalidObject);
    }
    xRet->gotoEnd(true);
    xRet->setString(rString);
    GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::END, nullptr);
}

//FIXME why is CheckForOwnMember duplicated in some insert methods?
//  Description: Checks if pRange/pCursor are member of the same text interface.
//              Only one of the pointers has to be set!
bool SwXText::Impl::CheckForOwnMember(
    const SwPaM & rPaM)
{
    const rtl::Reference< SwXTextCursor > xOwnCursor(m_rThis.createXTextCursor());
    const SwStartNode* pOwnStartNode =
        xOwnCursor->GetPaM()->GetPointNode().StartOfSectionNode();
    SwStartNodeType eSearchNodeType = SwNormalStartNode;
    switch (m_eType)
    {
        case CursorType::Frame:      eSearchNodeType = SwFlyStartNode;       break;
        case CursorType::TableText:    eSearchNodeType = SwTableBoxStartNode;  break;
        case CursorType::Footnote:   eSearchNodeType = SwFootnoteStartNode;  break;
        case CursorType::Header:     eSearchNodeType = SwHeaderStartNode;    break;
        case CursorType::Footer:     eSearchNodeType = SwFooterStartNode;    break;
        //case CURSOR_INVALID:
        //case CursorType::Body:
        default:
            ;
    }

    const SwNode& rSrcNode = rPaM.GetPointNode();
    const SwStartNode* pTmp = rSrcNode.FindSttNodeByType(eSearchNodeType);

    // skip SectionNodes / TableNodes to be able to compare across table/section boundaries
    while (pTmp
           && (pTmp->IsSectionNode() || pTmp->IsTableNode()
               || (m_eType != CursorType::TableText
                   && pTmp->GetStartNodeType() == SwTableBoxStartNode)))
    {
        pTmp = pTmp->StartOfSectionNode();
    }

    while (pOwnStartNode->IsSectionNode() || pOwnStartNode->IsTableNode()
           || (m_eType != CursorType::TableText
               && pOwnStartNode->GetStartNodeType() == SwTableBoxStartNode))
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
SwXText::getPropertySetInfo()
{
    SolarMutexGuard g;

    static uno::Reference< beans::XPropertySetInfo > xInfo =
        m_pImpl->m_rPropSet.getPropertySetInfo();
    return xInfo;
}

void SAL_CALL
SwXText::setPropertyValue(const OUString& /*aPropertyName*/,
        const uno::Any& /*aValue*/)
{
    throw lang::IllegalArgumentException();
}

uno::Any SAL_CALL
SwXText::getPropertyValue(
    const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
    {
        throw  uno::RuntimeException();
    }

    SfxItemPropertyMapEntry const*const pEntry =
        m_pImpl->m_rPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
        throw beans::UnknownPropertyException("Unknown property: " + rPropertyName);

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
                const SwNode& rOwnIndex = *pStartNode->EndOfSectionNode();
                for (size_t nRed = 0; nRed < nRedTableCount; ++nRed)
                {
                    SwRangeRedline const*const pRedline = rRedTable[nRed];
                    SwPosition const*const pRedStart = pRedline->Start();
                    const SwNode& rRedNode = pRedStart->GetNode();
                    if (rOwnIndex == rRedNode)
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
{
    OSL_FAIL("SwXText::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXText::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXText::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXText::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXText::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXText::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXText::removeVetoableChangeListener(): not implemented");
}

namespace
{
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::finishParagraph(
        const uno::Sequence< beans::PropertyValue > & rProperties)
{
    SolarMutexGuard g;

    return m_pImpl->finishOrAppendParagraph(rProperties, uno::Reference< text::XTextRange >());
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::finishParagraphInsert(
        const uno::Sequence< beans::PropertyValue > & rProperties,
        const uno::Reference< text::XTextRange >& xInsertPosition)
{
    SolarMutexGuard g;

    return m_pImpl->finishOrAppendParagraph(rProperties, xInsertPosition);
}

rtl::Reference<SwXParagraph>
SwXText::Impl::finishOrAppendParagraph(
        const uno::Sequence< beans::PropertyValue > & rProperties,
        const uno::Reference< text::XTextRange >& xInsertPosition)
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

    rtl::Reference<SwXParagraph> xRet;
    bool bIllegalException = false;
    bool bRuntimeException = false;
    OUString sMessage;
    m_pDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::START , nullptr);
    // find end node, go backward - don't skip tables because the new
    // paragraph has to be the last node
    //aPam.Move( fnMoveBackward, GoInNode );
    SwPaM aPam(*pStartNode->EndOfSectionNode(), SwNodeOffset(-1));
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
    aPam.Move( fnMoveBackward, GoInNode );

    try
    {
        SfxItemPropertySet const*const pParaPropSet =
            aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARAGRAPH);

        SwUnoCursorHelper::SetPropertyValues(aPam, *pParaPropSet, rProperties);

        // tdf#127616 keep direct character formatting of empty paragraphs,
        // if character style of the paragraph sets also the same attributes
        if (aPam.Start()->GetNode().GetTextNode()->Len() == 0)
        {
            auto itCharStyle = std::find_if(rProperties.begin(), rProperties.end(), [](const beans::PropertyValue& rValue)
            {
                return rValue.Name == "CharStyleName";
            });
            if ( itCharStyle != rProperties.end() )
            {
                for (const auto& rValue : rProperties)
                {
                    if ( rValue != *itCharStyle && rValue.Name.startsWith("Char") )
                    {
                        SwUnoCursorHelper::SetPropertyValue(aPam, *pParaPropSet, rValue.Name, rValue.Value);
                    }
                }
            }
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
    catch (const uno::Exception& rEx)
    {
        sMessage = rEx.Message;
        bRuntimeException = true;
    }

    m_pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::END, nullptr);
    if (bIllegalException || bRuntimeException)
    {
        m_pDoc->GetIDocumentUndoRedo().Undo();
        if (bIllegalException)
        {
            throw lang::IllegalArgumentException(sMessage, nullptr, 0);
        }
        else
        {
            throw uno::RuntimeException(sMessage);
        }
    }
    SwTextNode *const pTextNode( aPam.Start()->GetNode().GetTextNode() );
    OSL_ENSURE(pTextNode, "no SwTextNode?");
    if (pTextNode)
    {
        xRet = SwXParagraph::CreateXParagraph(*m_pDoc, pTextNode, &m_rThis);
    }

    return xRet;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::insertTextPortion(
        const OUString& rText,
        const uno::Sequence< beans::PropertyValue > &
            rCharacterAndParagraphProperties,
        const uno::Reference<text::XTextRange>& xInsertPosition)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
    {
        throw  uno::RuntimeException();
    }
    uno::Reference< text::XTextRange > xRet;
    const rtl::Reference<SwXTextCursor> xTextCursor = createXTextCursorByRange(xInsertPosition);

    bool bIllegalException = false;
    bool bRuntimeException = false;
    OUString sMessage;
    m_pImpl->m_pDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT, nullptr);

    auto& rCursor(xTextCursor->GetCursor());
    m_pImpl->m_pDoc->DontExpandFormat( *rCursor.Start() );

    if (!rText.isEmpty())
    {
        SwNodeIndex const nodeIndex(rCursor.GetPoint()->GetNode(), -1);
        const sal_Int32 nContentPos = rCursor.GetPoint()->GetContentIndex();
        SwUnoCursorHelper::DocInsertStringSplitCR(
            *m_pImpl->m_pDoc, rCursor, rText, false);
        SwUnoCursorHelper::SelectPam(rCursor, true);
        rCursor.GetPoint()->Assign(nodeIndex.GetNode(), SwNodeOffset(+1), nContentPos);
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
    m_pImpl->m_pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT, nullptr);
    if (bIllegalException || bRuntimeException)
    {
        m_pImpl->m_pDoc->GetIDocumentUndoRedo().Undo();
        if (bIllegalException)
        {
            throw lang::IllegalArgumentException(sMessage, nullptr, 0);
        }
        else
        {
            throw uno::RuntimeException(sMessage);
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
{
    SolarMutexGuard aGuard;

    if (!IsValid())
    {
        throw  uno::RuntimeException();
    }

    SwUnoInternalPaM aPam(*GetDoc());
    if (!::sw::XTextRangeToSwPaM(aPam, xInsertPosition))
    {
        throw lang::IllegalArgumentException(u"invalid position"_ustr, nullptr, 2);
    }

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, SwResId(STR_UNDO_INSERT_TEXTBOX));

    m_pImpl->m_pDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT, &aRewriter);

    // Any direct formatting ending at the insert position (xRange) should not
    // be expanded to cover the inserted content (xContent)
    // (insertTextContent() shouldn't do this, only ...WithProperties()!)
    GetDoc()->DontExpandFormat( *aPam.Start() );

    // now attach the text content here
    insertTextContent( xInsertPosition, xTextContent, false );
    // now apply the properties to the anchor
    if (rCharacterAndParagraphProperties.hasElements())
    {
        try
        {
            const uno::Reference< beans::XPropertySet > xAnchor(
                xTextContent->getAnchor(), uno::UNO_QUERY);
            if (xAnchor.is())
            {
                for (const auto& rProperty : rCharacterAndParagraphProperties)
                {
                    xAnchor->setPropertyValue(rProperty.Name, rProperty.Value);
                }
            }
        }
        catch (const uno::Exception& e)
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            m_pImpl->m_pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT, &aRewriter);
            throw lang::WrappedTargetRuntimeException( e.Message,
                            uno::Reference< uno::XInterface >(), anyEx );
        }
    }
    m_pImpl->m_pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT, &aRewriter);
    return xInsertPosition;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXText::appendTextContent(
    const uno::Reference< text::XTextContent >& xTextContent,
    const uno::Sequence< beans::PropertyValue >& rCharacterAndParagraphProperties
    )
{
    // Right now this doesn't need a guard, as it's just calling the insert
    // version, that has it already.
    uno::Reference<text::XTextRange> xInsertPosition = getEnd();
    return insertTextContentWithProperties(xTextContent, rCharacterAndParagraphProperties, xInsertPosition);
}

// determine whether SwFrameFormat is a graphic node
static bool isGraphicNode(const SwFrameFormat* pFrameFormat)
{
    // safety
    if( !pFrameFormat->GetContent().GetContentIdx() )
    {
        return false;
    }
    auto index = *pFrameFormat->GetContent().GetContentIdx();
    // consider the next node -> there is the graphic stored
    ++index;
    return index.GetNode().IsGrfNode();
}

/// Determines if the at-para rAnchor is anchored at the start or end of rAnchorCheckPam.
static bool IsAtParaMatch(const SwPaM& rAnchorCheckPam, const SwFormatAnchor& rAnchor)
{
    if (rAnchor.GetAnchorId() != RndStdIds::FLY_AT_PARA)
    {
        return false;
    }

    if (rAnchorCheckPam.Start()->GetNode() == *rAnchor.GetAnchorNode())
    {
        return true;
    }

    if (rAnchorCheckPam.End()->GetNode() == *rAnchor.GetAnchorNode())
    {
        SwTextNode* pEndTextNode = rAnchorCheckPam.End()->GetNode().GetTextNode();
        if (pEndTextNode && rAnchorCheckPam.End()->GetContentIndex() == pEndTextNode->Len())
        {
            // rAnchorCheckPam covers the entire last text node, rAnchor is at-para, consider this
            // as "inside pam" rather than "at the end of pam".
            return false;
        }
        return true;
    }

    return false;
}

// move previously appended paragraphs into a text frames
// to support import filters
uno::Reference< text::XTextContent > SAL_CALL
SwXText::convertToTextFrame(
    const uno::Reference< text::XTextRange >& xStart,
    const uno::Reference< text::XTextRange >& xEnd,
    const uno::Sequence< beans::PropertyValue >& rFrameProperties)
{
    SolarMutexGuard aGuard;

    if(!IsValid())
    {
        throw  uno::RuntimeException();
    }
    // tdf#143384 recognize dummy property, that was set to make createTextCursor
    // to not ignore tables.
    // It is enough to use this hack only for the range start,
    // because as far as I know, the range cannot end with table when this property is set.
    ::sw::TextRangeMode eMode = ::sw::TextRangeMode::RequireTextNode;
    for (const auto& rCellProperty : rFrameProperties)
    {
        if (rCellProperty.Name == "CursorNotIgnoreTables")
        {
            bool bAllowNonTextNode = false;
            rCellProperty.Value >>= bAllowNonTextNode;
            if (bAllowNonTextNode)
                eMode = ::sw::TextRangeMode::AllowTableNode;
            break;
        }
    }
    uno::Reference< text::XTextContent > xRet;
    std::optional<SwUnoInternalPaM> pTempStartPam(*GetDoc());
    std::optional<SwUnoInternalPaM> pEndPam(*GetDoc());
    if (!::sw::XTextRangeToSwPaM(*pTempStartPam, xStart, eMode)
        || !::sw::XTextRangeToSwPaM(*pEndPam, xEnd))
    {
        throw lang::IllegalArgumentException();
    }

    auto pStartPam(GetDoc()->CreateUnoCursor(*pTempStartPam->GetPoint()));
    if (pTempStartPam->HasMark())
    {
        pStartPam->SetMark();
        *pStartPam->GetMark() = *pTempStartPam->GetMark();
    }
    pTempStartPam.reset();

    SwXTextRange *const pStartRange = dynamic_cast<SwXTextRange*>(xStart.get());
    SwXTextRange *const pEndRange   = dynamic_cast<SwXTextRange*>(xEnd.get());
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

    m_pImpl->m_pDoc->GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );
    bool bIllegalException = false;
    bool bRuntimeException = false;
    OUString sMessage;
    SwStartNode* pStartStartNode = pStartPam->GetPointNode().StartOfSectionNode();
    while (pStartStartNode && pStartStartNode->IsSectionNode())
    {
        pStartStartNode = pStartStartNode->StartOfSectionNode();
    }
    SwStartNode* pEndStartNode = pEndPam->GetPointNode().StartOfSectionNode();
    while (pEndStartNode && pEndStartNode->IsSectionNode())
    {
        pEndStartNode = pEndStartNode->StartOfSectionNode();
    }
    bool bParaAfterInserted = false;
    bool bParaBeforeInserted = false;
    ::std::optional<SwPaM> oAnchorCheckPam;
    oAnchorCheckPam.emplace(*pStartPam->Start(), *pEndPam->End());
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
            pStartPam->DeleteMark();
            *pStartPam->GetPoint() = aBefore;
            pStartStartNode = pStartPam->GetPointNode().StartOfSectionNode();
        }
        if (pEndStartNode->GetStartNodeType() == SwTableBoxStartNode)
        {
            SwTableNode *const pEndTableNode = pEndStartNode->FindTableNode();
            SwEndNode *const pTableEnd = pEndTableNode->EndOfSectionNode();
            SwPosition aTableEnd(*pTableEnd);
            bParaAfterInserted = GetDoc()->getIDocumentContentOperations().AppendTextNode( aTableEnd );
            pEndPam->DeleteMark();
            *pEndPam->GetPoint() = aTableEnd;
            pEndStartNode = pEndPam->GetPointNode().StartOfSectionNode();
        }
        // now we should have the positions in the same hierarchy
        if ((pStartStartNode != pEndStartNode) ||
            (pStartStartNode != GetStartNode()))
        {
            // if not - remove the additional paragraphs and throw
            oAnchorCheckPam.reset(); // clear SwContentIndex before deleting nodes
            if (bParaBeforeInserted)
            {
                SwCursor aDelete(*pStartPam->GetPoint(), nullptr);
                *pStartPam->GetPoint() = // park it because node is deleted
                    SwPosition(GetDoc()->GetNodes().GetEndOfContent());
                aDelete.MovePara(GoCurrPara, fnParaStart);
                aDelete.SetMark();
                aDelete.MovePara(GoCurrPara, fnParaEnd);
                GetDoc()->getIDocumentContentOperations().DelFullPara(aDelete);
            }
            if (bParaAfterInserted)
            {
                SwCursor aDelete(*pEndPam->GetPoint(), nullptr);
                *pEndPam->GetPoint() = // park it because node is deleted
                    SwPosition(GetDoc()->GetNodes().GetEndOfContent());
                aDelete.MovePara(GoCurrPara, fnParaStart);
                aDelete.SetMark();
                aDelete.MovePara(GoCurrPara, fnParaEnd);
                GetDoc()->getIDocumentContentOperations().DelFullPara(aDelete);
            }
            throw lang::IllegalArgumentException();
        }
    }

    // make a selection from pStartPam to pEndPam
    // If there is no content in the frame the shape is in
    // it gets deleted in the DelFullPara call below,
    // In this case insert a tmp text node ( we delete it later )
    if (pStartPam->Start()->GetNode() == pEndPam->Start()->GetNode()
        && pStartPam->End()->GetNode() == pEndPam->End()->GetNode())
    {
        SwPosition aEnd(*pStartPam->End());
        bParaAfterInserted = GetDoc()->getIDocumentContentOperations().AppendTextNode( aEnd );
        pEndPam->DeleteMark();
        *pEndPam->GetPoint() = aEnd;
        *oAnchorCheckPam->End() = aEnd;
    }
    pStartPam->SetMark();
    *pStartPam->End() = *pEndPam->End();
    pEndPam.reset();

    // see if there are frames already anchored to this node
    // we have to work with the SdrObjects, as unique name is not guaranteed in their frame format
    // tdf#115094: do nothing if we have a graphic node
    o3tl::sorted_vector<const SdrObject*> aAnchoredObjectsByPtr;
    std::set<OUString> aAnchoredObjectsByName;
    for (size_t i = 0; i < m_pImpl->m_pDoc->GetSpzFrameFormats()->size(); ++i)
    {
        const SwFrameFormat* pFrameFormat = (*m_pImpl->m_pDoc->GetSpzFrameFormats())[i];
        const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
        // note: Word can do at-char anchors in text frames - sometimes!
        // see testFlyInFly for why this checks only the edges of the selection,
        // and testFloatingTablesAnchor for why it excludes pre/post table
        // added nodes
        // TODO: isGraphicNode here looks dubious; see also tdf#47036 fix;
        // this needs more investigation when exactly Word considers something
        // anchored in text frame vs. anchored in body.
        if (!isGraphicNode(pFrameFormat)
            && (IsAtParaMatch(*oAnchorCheckPam, rAnchor)
                || (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId()
                    && (    *oAnchorCheckPam->Start() == *rAnchor.GetContentAnchor()
                        ||  *oAnchorCheckPam->End() == *rAnchor.GetContentAnchor()))))
        {
            if (pFrameFormat->GetName().isEmpty())
            {
                aAnchoredObjectsByPtr.insert(pFrameFormat->FindSdrObject());
            }
            else
            {
                aAnchoredObjectsByName.insert(pFrameFormat->GetName());
            }
        }
    }
    oAnchorCheckPam.reset(); // clear SwContentIndex before deleting nodes

    const rtl::Reference<SwXTextFrame> xNewFrame =
            SwXTextFrame::CreateXTextFrame(*m_pImpl->m_pDoc, nullptr);
    try
    {
        for (const beans::PropertyValue& rValue : rFrameProperties)
        {
            xNewFrame->SwXFrame::setPropertyValue(rValue.Name, rValue.Value);
        }

        {   // has to be in a block to remove the SwContentIndexes before
            // DelFullPara is called
            const uno::Reference< text::XTextRange> xInsertTextRange =
                new SwXTextRange(*pStartPam, this);
            assert(xNewFrame->IsDescriptor());
            xNewFrame->attachToRange(xInsertTextRange, pStartPam.get());
            assert(!xNewFrame->getName().isEmpty());
        }

        SwTextNode *const pTextNode(pStartPam->GetPointNode().GetTextNode());
        assert(pTextNode);
        if (!pTextNode || !pTextNode->Len()) // don't remove if it contains text!
        {
            {   // has to be in a block to remove the SwContentIndexes before
                // DelFullPara is called
                SwPaM aMovePam( pStartPam->GetPointNode() );
                if (aMovePam.Move( fnMoveForward, GoInContent ))
                {
                    // move the anchor to the next paragraph
                    SwFormatAnchor aNewAnchor(xNewFrame->GetFrameFormat()->GetAnchor());
                    aNewAnchor.SetAnchor( aMovePam.Start() );
                    m_pImpl->m_pDoc->SetAttr(
                        aNewAnchor, *xNewFrame->GetFrameFormat() );

                    // also move frames anchored to us
                    for (size_t i = 0; i < m_pImpl->m_pDoc->GetSpzFrameFormats()->size(); ++i)
                    {
                        SwFrameFormat* pFrameFormat = (*m_pImpl->m_pDoc->GetSpzFrameFormats())[i];
                        if ((!pFrameFormat->GetName().isEmpty() && aAnchoredObjectsByName.find(pFrameFormat->GetName()) != aAnchoredObjectsByName.end() ) ||
                            ( pFrameFormat->GetName().isEmpty() && aAnchoredObjectsByPtr.find(pFrameFormat->FindSdrObject()) != aAnchoredObjectsByPtr.end()) )
                        {
                            // copy the anchor to the next paragraph
                            SwFormatAnchor aAnchor(pFrameFormat->GetAnchor());
                            aAnchor.SetAnchor(aMovePam.Start());
                            m_pImpl->m_pDoc->SetAttr(aAnchor, *pFrameFormat);
                        }
                        else
                        {
                            // if this frame is a textbox of a shape anchored to us, move this textbox too.
                            const auto& pTextBoxes = pFrameFormat->GetOtherTextBoxFormats();
                            if (pFrameFormat->Which() == RES_FLYFRMFMT && pTextBoxes
                                && pTextBoxes->GetOwnerShape())
                            {
                                const auto& rShapeAnchor = pTextBoxes->GetOwnerShape()->GetAnchor();
                                if (rShapeAnchor.GetAnchorId() == RndStdIds::FLY_AS_CHAR
                                    && rShapeAnchor.GetContentAnchor() && pFrameFormat->GetAnchor().GetContentAnchor()
                                    && pStartPam->ContainsPosition(*pFrameFormat->GetAnchor().GetContentAnchor()))
                                {
                                    const SwNode& rAnchorNode
                                        = *pFrameFormat->GetAnchor().GetAnchorNode();
                                    if (!(rAnchorNode.FindFooterStartNode() || rAnchorNode.FindHeaderStartNode()))
                                    {
                                        SwFormatAnchor aAnchor(pFrameFormat->GetAnchor());
                                        aAnchor.SetAnchor(aMovePam.Start());
                                        m_pImpl->m_pDoc->SetAttr(aAnchor, *pFrameFormat);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            m_pImpl->m_pDoc->getIDocumentContentOperations().DelFullPara(*pStartPam);
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
    xRet = static_cast<SwXFrame*>(xNewFrame.get());
    if (bParaBeforeInserted || bParaAfterInserted)
    {
        const rtl::Reference<SwXTextCursor> xFrameTextCursor =
            xNewFrame->createXTextCursor();
        if (bParaBeforeInserted)
        {
            // todo: remove paragraph before frame
            m_pImpl->m_pDoc->getIDocumentContentOperations().DelFullPara(*xFrameTextCursor->GetPaM());
        }
        if (bParaAfterInserted)
        {
            xFrameTextCursor->gotoEnd(false);
            if (!bParaBeforeInserted)
                m_pImpl->m_pDoc->getIDocumentContentOperations().DelFullPara(*xFrameTextCursor->GetPaM());
            else
            {
                // In case the frame has a table only, the cursor points to the end of the first cell of the table.
                SwPaM aPaM(*xFrameTextCursor->GetPaM()->GetPointNode().FindSttNodeByType(SwFlyStartNode)->EndOfSectionNode());
                // Now we have the end of the frame -- the node before that will be the paragraph we want to remove.
                aPaM.GetPoint()->Adjust(SwNodeOffset(-1));
                m_pImpl->m_pDoc->getIDocumentContentOperations().DelFullPara(aPaM);
            }
        }
    }

    m_pImpl->m_pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::END, nullptr);
    if (bIllegalException || bRuntimeException)
    {
        m_pImpl->m_pDoc->GetIDocumentUndoRedo().Undo();
        if (bIllegalException)
        {
            throw lang::IllegalArgumentException(sMessage, nullptr, 0);
        }
        else
        {
            throw uno::RuntimeException(sMessage);
        }
    }
    return xRet;
}

namespace {

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

}

#define COL_POS_FUZZY 2

static bool lcl_SimilarPosition( const sal_Int32 nPos1, const sal_Int32 nPos2 )
{
    return abs( nPos1 - nPos2 ) < COL_POS_FUZZY;
}

void SwXText::Impl::ConvertCell(
    const uno::Sequence< uno::Reference< text::XTextRange > > & rCell,
    std::vector<SwNodeRange> & rRowNodes,
    SwNodeRange *const pLastCell)
{
    if (rCell.getLength() != 2)
    {
        throw lang::IllegalArgumentException(
                u"rCell needs to contain 2 elements"_ustr,
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
                u"Start or End range cannot be resolved to a SwPaM"_ustr,
                uno::Reference< text::XTextCopy >( &m_rThis ), sal_Int16( 2 ) );
    }

    SwNodeRange aTmpRange(aStartCellPam.Start()->GetNode(),
                          aEndCellPam.End()->GetNode());
    std::optional<SwNodeRange> oCorrectedRange;
    m_pDoc->GetNodes().ExpandRangeForTableBox(aTmpRange, oCorrectedRange);

    if (oCorrectedRange)
    {
        SwPaM aNewStartPaM(oCorrectedRange->aStart, 0);
        aStartCellPam = aNewStartPaM;

        sal_Int32 nEndLen = 0;
        SwTextNode * pTextNode = oCorrectedRange->aEnd.GetNode().GetTextNode();
        if (pTextNode != nullptr)
            nEndLen = pTextNode->Len();

        SwPaM aNewEndPaM(oCorrectedRange->aEnd, nEndLen);
        aEndCellPam = aNewEndPaM;

        oCorrectedRange.reset();
    }

    /** check the nodes between start and end
        it is allowed to have pairs of StartNode/EndNodes
     */
    if (aStartCellPam.Start()->GetNode() < aEndCellPam.End()->GetNode())
    {
        // increment on each StartNode and decrement on each EndNode
        // we must reach zero at the end and must not go below zero
        tools::Long nOpenNodeBlock = 0;
        SwNodeIndex aCellIndex(aStartCellPam.Start()->GetNode());
        while (aCellIndex < aEndCellPam.End()->GetNodeIndex())
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
                throw lang::IllegalArgumentException();
            }
            ++aCellIndex;
        }
        if (nOpenNodeBlock != 0)
        {
            throw lang::IllegalArgumentException();
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
        if (aStartCellPam.Start()->GetContentIndex())
        {
            m_pDoc->getIDocumentContentOperations().SplitNode(*aStartCellPam.Start(), false);
        }
    }
    else
    {
        // check the predecessor
        const SwNodeOffset nStartCellNodeIndex =
            aStartCellPam.Start()->GetNodeIndex();
        const SwNodeOffset nLastNodeEndIndex = pLastCell->aEnd.GetIndex();
        if (nLastNodeEndIndex == nStartCellNodeIndex)
        {
            // same node as predecessor then equal nContent?
            if (0 != aStartCellPam.Start()->GetContentIndex())
            {
                throw lang::IllegalArgumentException();
            }

            m_pDoc->getIDocumentContentOperations().SplitNode(*aStartCellPam.Start(), false);
            SwNodeOffset const nNewIndex(aStartCellPam.Start()->GetNodeIndex());
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
        else if (nStartCellNodeIndex == (nLastNodeEndIndex + 1))
        {
            // next paragraph - now the content index of the new should be 0
            // and of the old one should be equal to the text length
            // but if it isn't we don't care - the cell is being inserted on
            // the node border anyway
        }
        else
        {
            throw lang::IllegalArgumentException();
        }
    }
    // now check if there's a need to insert another paragraph break
    if (aEndCellPam.End()->GetContentIndex() <
            aEndCellPam.End()->GetNode().GetTextNode()->Len())
    {
        m_pDoc->getIDocumentContentOperations().SplitNode(*aEndCellPam.End(), false);
        // take care that the new start/endcell is moved to the right position
        // aStartCellPam has to point to the start of the new (previous) node
        // aEndCellPam has to point to the end of the new (previous) node
        aStartCellPam.DeleteMark();
        aStartCellPam.Move(fnMoveBackward, GoInNode);
        aStartCellPam.GetPoint()->SetContent(0);
        aEndCellPam.DeleteMark();
        aEndCellPam.Move(fnMoveBackward, GoInNode);
        aEndCellPam.GetPoint()->SetContent(
            aEndCellPam.GetPointNode().GetTextNode()->Len() );
    }

    assert(aStartCellPam.Start()->GetContentIndex() == 0);
    assert(aEndCellPam.End()->GetContentIndex() == aEndCellPam.End()->GetNode().GetTextNode()->Len());
    SwNodeRange aCellRange(aStartCellPam.Start()->GetNode(),
            aEndCellPam.End()->GetNode());
    rRowNodes.push_back(aCellRange); // note: invalidates pLastCell!

    // tdf#149649 delete any fieldmarks overlapping the cell
    IDocumentMarkAccess & rIDMA(*m_pDoc->getIDocumentMarkAccess());
    while (sw::mark::IFieldmark *const pMark = rIDMA.getInnerFieldmarkFor(*aStartCellPam.Start()))
    {
        if (pMark->GetMarkEnd() <= *aEndCellPam.End())
        {
            if (pMark->GetMarkStart() < *aStartCellPam.Start())
            {
                SAL_INFO("sw.uno", "deleting fieldmark overlapping table cell");
                rIDMA.deleteMark(pMark);
            }
            else
            {
                break;
            }
        }
        else
        {
            SwPosition const sepPos(::sw::mark::FindFieldSep(*pMark));
            if (*aStartCellPam.Start() <= sepPos && sepPos <= *aEndCellPam.End())
            {
                SAL_INFO("sw.uno", "deleting fieldmark with separator in table cell");
                rIDMA.deleteMark(pMark);
            }
            else
            {
                break;
            }
        }
    }
    while (sw::mark::IFieldmark *const pMark = rIDMA.getInnerFieldmarkFor(*aEndCellPam.End()))
    {
        if (*aStartCellPam.Start() <= pMark->GetMarkStart())
        {
            if (*aEndCellPam.End() < pMark->GetMarkEnd())
            {
                SAL_INFO("sw.uno", "deleting fieldmark overlapping table cell");
                rIDMA.deleteMark(pMark);
            }
            else
            {
                break;
            }
        }
        else
        {
            SwPosition const sepPos(::sw::mark::FindFieldSep(*pMark));
            if (*aStartCellPam.Start() <= sepPos && sepPos <= *aEndCellPam.End())
            {
                SAL_INFO("sw.uno", "deleting fieldmark with separator in table cell");
                rIDMA.deleteMark(pMark);
            }
            else
            {
                break;
            }
       }
    }
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
    for (const beans::PropertyValue& rProperty : rRowProperties)
    {
        if ( rProperty.Name == "TableColumnSeparators" )
        {
            // add the separators to access the cell's positions
            // for vertical merging later
            TableColumnSeparators aSeparators;
            rProperty.Value >>= aSeparators;
            rRowSeparators = aSeparators;
        }
        xRow->setPropertyValue(rProperty.Name, rProperty.Value);
    }
}

static sal_Int32 lcl_GetLeftPos(sal_Int32 nCell, TableColumnSeparators const& rRowSeparators)
{
    if(!nCell)
        return 0;
    if (rRowSeparators.getLength() < nCell)
        return -1;
    return rRowSeparators[nCell - 1].Position;
}

static void
lcl_ApplyCellProperties(
    const sal_Int32 nLeftPos,
    const uno::Sequence< beans::PropertyValue >& rCellProperties,
    const uno::Reference< uno::XInterface >& xCell,
    std::vector<VerticallyMergedCell> & rMergedCells)
{
    const uno::Reference< beans::XPropertySet > xCellPS(xCell, uno::UNO_QUERY);
    for (const auto& rCellProperty : rCellProperties)
    {
        const OUString & rName  = rCellProperty.Name;
        const uno::Any & rValue = rCellProperty.Value;
        if ( rName == "VerticalMerge" )
        {
            // determine left border position
            // add the cell to a queue of merged cells
            bool bMerge = false;
            rValue >>= bMerge;
            if (bMerge)
            {
                // 'close' all the cell with the same left position
                // if separate vertical merges in the same column exist
                for(auto& aMergedCell : rMergedCells)
                {
                    if(lcl_SimilarPosition(aMergedCell.nLeftPosition, nLeftPos))
                    {
                        aMergedCell.bOpen = false;
                    }
                }
                // add the new group of merged cells
                rMergedCells.emplace_back(xCellPS, nLeftPos);
            }
            else
            {
                bool bFound = false;
                SAL_WARN_IF(rMergedCells.empty(), "sw.uno", "the first merged cell is missing");
                for(auto& aMergedCell : rMergedCells)
                {
                    if (aMergedCell.bOpen && lcl_SimilarPosition(aMergedCell.nLeftPosition, nLeftPos))
                    {
                        aMergedCell.aCells.push_back( xCellPS );
                        bFound = true;
                    }
                }
                SAL_WARN_IF(!bFound, "sw.uno", "couldn't find first vertically merged cell" );
            }
        }
        else
        {
            try
            {
                static const std::initializer_list<std::u16string_view> vDenylist = {
                    u"LeftMargin",
                    u"ParaTopBorder",
                    u"ParaTopBorderDistance",
                    u"ParaTopBorderComplexColor",
                    u"ParaLeftBorder",
                    u"ParaLeftBorderDistance",
                    u"ParaLeftBorderComplexColor",
                    u"ParaBottomBorder",
                    u"ParaBottomBorderDistance",
                    u"ParaBottomBorderComplexColor",
                    u"ParaRightBorder",
                    u"ParaRightBorderDistance",
                    u"ParaRightBorderComplexColor",
                };
                if (std::find(vDenylist.begin(), vDenylist.end(), rName) == vDenylist.end())
                {
                    xCellPS->setPropertyValue(rName, rValue);
                }
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION( "sw.uno", "Exception when setting cell property " << rName );
            }
        }
    }
}

static void
lcl_MergeCells(std::vector<VerticallyMergedCell> & rMergedCells)
{
    for(auto& aMergedCell : rMergedCells)
    {
        // the first of the cells gets the number of cells set as RowSpan
        // the others get the inverted number of remaining merged cells
        // (3,-2,-1)
        sal_Int32 nCellCount = static_cast<sal_Int32>(aMergedCell.aCells.size());
        if(nCellCount<2)
        {
            SAL_WARN("sw.uno", "incomplete vertical cell merge");
            continue;
        }
        aMergedCell.aCells.front()->setPropertyValue(UNO_NAME_ROW_SPAN, uno::Any(nCellCount--));
        nCellCount*=-1;
        for(auto pxPSet = aMergedCell.aCells.begin()+1; nCellCount<0; ++pxPSet, ++nCellCount)
        {
            (*pxPSet)->setPropertyValue(UNO_NAME_ROW_SPAN, uno::Any(nCellCount));
            (*pxPSet)->setPropertyValue(u"VerticalMerge"_ustr, uno::Any(true));
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
{
    SolarMutexGuard aGuard;

    if(!IsValid())
    {
        throw  uno::RuntimeException();
    }

    IDocumentRedlineAccess & rIDRA(m_pImpl->m_pDoc->getIDocumentRedlineAccess());
    if (!IDocumentRedlineAccess::IsShowChanges(rIDRA.GetRedlineFlags()))
    {
        throw uno::RuntimeException(
            u"cannot convertToTable if tracked changes are hidden!"_ustr);
    }

    //at first collect the text ranges as SwPaMs
    const uno::Sequence< uno::Sequence< uno::Reference< text::XTextRange > > >*
        pTableRanges = rTableRanges.getConstArray();
    std::vector< std::vector<SwNodeRange> > aTableNodes;
    for (sal_Int32 nRow = 0; nRow < rTableRanges.getLength(); ++nRow)
    {
        std::vector<SwNodeRange> aRowNodes;
        const uno::Sequence< uno::Reference< text::XTextRange > >* pRow =
            pTableRanges[nRow].getConstArray();
        const sal_Int32 nCells(pTableRanges[nRow].getLength());

        if (0 == nCells) // this would lead to no pLastCell below
        {                // and make it impossible to detect node gaps
            throw lang::IllegalArgumentException();
        }

        for (sal_Int32 nCell = 0; nCell < nCells; ++nCell)
        {
            SwNodeRange *const pLastCell(
                (nCell == 0)
                    ? ((nRow == 0)
                        ? nullptr
                        : &*aTableNodes.rbegin()->rbegin())
                    : &*aRowNodes.rbegin());
            m_pImpl->ConvertCell(pRow[nCell], aRowNodes, pLastCell);
        }
        assert(!aRowNodes.empty());
        aTableNodes.push_back(aRowNodes);
    }

    std::vector< TableColumnSeparators >
        aRowSeparators(rRowProperties.getLength());
    std::vector<VerticallyMergedCell> aMergedCells;

    SwTable const*const pTable = m_pImpl->m_pDoc->TextToTable( aTableNodes );

    if (!pTable)
        return uno::Reference< text::XTextTable >();

    rtl::Reference<SwXTextTable> const xRet =
        SwXTextTable::CreateXTextTable(pTable->GetFrameFormat());
    // set properties to the table
    // catch lang::WrappedTargetException and lang::IndexOutOfBoundsException
    try
    {
        //apply table properties
        for(const auto& rTableProperty : rTableProperties)
        {
            try
            {
                static const std::initializer_list<std::u16string_view> vDenylist = {
                    u"BottomBorder",
                    u"CharAutoKerning",
                    u"CharFontName",
                    u"CharFontNameAsian",
                    u"CharFontNameComplex",
                    u"CharHeight",
                    u"CharHeightAsian",
                    u"CharHeightComplex",
                    u"CharInteropGrabBag",
                    u"CharLocale",
                    u"CharLocaleAsian",
                    u"CharLocaleComplex",
                    u"HorizontalBorder",
                    u"LeftBorder",
                    u"ParaAdjust",
                    u"ParaBottomMargin",
                    u"ParaIsHyphenation",
                    u"ParaLineSpacing",
                    u"ParaOrphans",
                    u"ParaTopMargin",
                    u"ParaWidows",
                    u"RightBorder",
                    u"TopBorder",
                    u"VerticalBorder",
                    u"ParaTopBorder",
                    u"ParaTopBorderDistance",
                    u"ParaTopBorderComplexColor",
                    u"ParaLeftBorder",
                    u"ParaLeftBorderDistance",
                    u"ParaLeftBorderComplexColor",
                    u"ParaBottomBorder",
                    u"ParaBottomBorderDistance",
                    u"ParaBottomBorderComplexColor",
                    u"ParaRightBorder",
                    u"ParaRightBorderDistance",
                    u"ParaRightBorderComplexColor",
                };
                if (std::find(vDenylist.begin(), vDenylist.end(), rTableProperty.Name) == vDenylist.end())
                {
                    xRet->setPropertyValue(rTableProperty.Name, rTableProperty.Value);
                }
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION( "sw.uno", "Exception when setting property: " << rTableProperty.Name  );
            }
        }

        //apply row properties
        const auto xRows = xRet->getRows();
        const sal_Int32 nLast = std::min(xRows->getCount(), rRowProperties.getLength());
        SAL_WARN_IF(nLast != rRowProperties.getLength(), "sw.uno", "not enough rows for properties");
        for(sal_Int32 nCnt = 0; nCnt < nLast; ++nCnt)
            lcl_ApplyRowProperties(rRowProperties[nCnt], xRows->getByIndex(nCnt), aRowSeparators[nCnt]);

        //apply cell properties
        sal_Int32 nRow = 0;
        for(const auto& rCellPropertiesForRow : rCellProperties)
        {
            sal_Int32 nCell = 0;
            for(const auto& rCellProps : rCellPropertiesForRow)
            {
                lcl_ApplyCellProperties(lcl_GetLeftPos(nCell, aRowSeparators[nRow]),
                    rCellProps,
                    xRet->getCellByPosition(nCell, nRow),
                    aMergedCells);
                ++nCell;
            }
            ++nRow;
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
    assert(pTable->GetFrameFormat() == xRet->GetFrameFormat());
    return xRet;
}

void SAL_CALL
SwXText::copyText(
    const uno::Reference< text::XTextCopy >& xSource )
{
    SolarMutexGuard aGuard;

    SwXText* const pSource(dynamic_cast<SwXText*>(xSource.get()));
    if (!pSource)
        throw uno::RuntimeException();

    rtl::Reference< SwXTextCursor > const xCursor = pSource->createXTextCursor();
    xCursor->gotoEnd( true );

    SwNodeIndex rNdIndex( *GetStartNode( ), 1 );
    SwPosition rPos( rNdIndex );
    // tdf#112202 need SwXText because cursor cannot select table at the start
    SwTextNode * pFirstNode;
    {
        SwPaM temp(*pSource->GetStartNode(), *pSource->GetStartNode()->EndOfSectionNode(), SwNodeOffset(+1), SwNodeOffset(-1));
        pFirstNode = temp.GetMark()->GetNode().GetTextNode();
        if (pFirstNode)
        {
            temp.GetMark()->AssignStartIndex(*pFirstNode);
        }
        if (SwTextNode *const pNode = temp.GetPoint()->GetNode().GetTextNode())
        {
            temp.GetPoint()->AssignEndIndex(*pNode);
        }
        // Explicitly request copy text mode, so
        // sw::DocumentContentOperationsManager::CopyFlyInFlyImpl() will copy shapes anchored to
        // us, even if we have only a single paragraph.
        m_pImpl->m_pDoc->getIDocumentContentOperations().CopyRange(temp, rPos, SwCopyFlags::CheckPosInFly);
    }
}

SwXBodyText::SwXBodyText(SwDoc *const pDoc)
    : SwXText(pDoc, CursorType::Body)
{
}

SwXBodyText::~SwXBodyText()
{
}

OUString SAL_CALL
SwXBodyText::getImplementationName()
{
    return u"SwXBodyText"_ustr;
}

sal_Bool SAL_CALL SwXBodyText::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXBodyText::getSupportedServiceNames()
{
    return { u"com.sun.star.text.Text"_ustr };
}

uno::Sequence< uno::Type > SAL_CALL
SwXBodyText::getTypes()
{
    const uno::Sequence< uno::Type > aTypes = SwXBodyText_Base::getTypes();
    const uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();
    return ::comphelper::concatSequences(aTypes, aTextTypes);
}

uno::Sequence< sal_Int8 > SAL_CALL
SwXBodyText::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Any SAL_CALL
SwXBodyText::queryInterface(const uno::Type& rType)
{
    const uno::Any ret = SwXText::queryInterface(rType);
    return (ret.getValueType() == cppu::UnoType<void>::get())
        ?   SwXBodyText_Base::queryInterface(rType)
        :   ret;
}

rtl::Reference<SwXTextCursor> SwXBodyText::CreateTextCursor(const bool bIgnoreTables)
{
    if(!IsValid())
    {
        return nullptr;
    }

    // the cursor has to skip tables contained in this text
    SwPaM aPam(GetDoc()->GetNodes().GetEndOfContent());
    aPam.Move( fnMoveBackward, GoInDoc );
    if (!bIgnoreTables)
    {
        SwTableNode * pTableNode = aPam.GetPointNode().FindTableNode();
        while (pTableNode)
        {
            aPam.GetPoint()->Assign( *pTableNode->EndOfSectionNode() );
            SwContentNode* pCont = SwNodes::GoNext(aPam.GetPoint());
            pTableNode = pCont->FindTableNode();
        }
    }
    return new SwXTextCursor(*GetDoc(), this, CursorType::Body, *aPam.GetPoint());
}

rtl::Reference< SwXTextCursor >
SwXBodyText::createXTextCursor()
{
    return CreateTextCursor();
}

rtl::Reference< SwXTextCursor >
SwXBodyText::createXTextCursorByRange(
    const uno::Reference< text::XTextRange > & xTextPosition)
{
    if(!IsValid())
        throw uno::RuntimeException(cInvalidObject);

    rtl::Reference< SwXTextCursor > aRef;
    SwUnoInternalPaM aPam(*GetDoc());
    if (::sw::XTextRangeToSwPaM(aPam, xTextPosition))
    {
        if ( !aPam.GetPointNode().GetTextNode() )
            throw uno::RuntimeException(u"Invalid text range"_ustr );

        SwNode& rNode = GetDoc()->GetNodes().GetEndOfContent();

        SwStartNode* p1 = aPam.GetPointNode().StartOfSectionNode();
        //document starts with a section?
        while(p1->IsSectionNode())
        {
            p1 = p1->StartOfSectionNode();
        }
        SwStartNode *const p2 = rNode.StartOfSectionNode();

        if(p1 == p2)
        {
            aRef = new SwXTextCursor(*GetDoc(), this, CursorType::Body,
                        *aPam.GetPoint(), aPam.GetMark());
        }
    }
    if(!aRef.is())
    {
        throw uno::RuntimeException( u"End of content node doesn't have the proper start node"_ustr,
               uno::Reference< uno::XInterface >( *this ) );
    }
    return aRef;
}

uno::Reference< container::XEnumeration > SAL_CALL
SwXBodyText::createEnumeration()
{
    return createParagraphEnumeration();
}

rtl::Reference< SwXParagraphEnumeration >
SwXBodyText::createParagraphEnumeration()
{
    SolarMutexGuard aGuard;

    if (!IsValid())
        throw uno::RuntimeException(cInvalidObject);

    SwNode& rNode = GetDoc()->GetNodes().GetEndOfContent();
    SwPosition aPos(rNode);
    auto pUnoCursor(GetDoc()->CreateUnoCursor(aPos));
    pUnoCursor->Move(fnMoveBackward, GoInDoc);
    return SwXParagraphEnumeration::Create(this, pUnoCursor, CursorType::Body);
}

uno::Type SAL_CALL
SwXBodyText::getElementType()
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL
SwXBodyText::hasElements()
{
    SolarMutexGuard aGuard;

    if (!IsValid())
        throw uno::RuntimeException(cInvalidObject);

    return true;
}

class SwXHeadFootText::Impl
    : public SvtListener
{
    public:
        SwFrameFormat* m_pHeadFootFormat;
        bool m_bIsHeader;

        Impl(SwFrameFormat& rHeadFootFormat, const bool bIsHeader)
            : m_pHeadFootFormat(&rHeadFootFormat)
            , m_bIsHeader(bIsHeader)
        {
            StartListening(m_pHeadFootFormat->GetNotifier());
        }

        SwFrameFormat* GetHeadFootFormat() const {
            return m_pHeadFootFormat;
        }

        SwFrameFormat& GetHeadFootFormatOrThrow() {
            if (!m_pHeadFootFormat) {
                throw uno::RuntimeException(u"SwXHeadFootText: disposed or invalid"_ustr, nullptr);
            }
            return *m_pHeadFootFormat;
        }
    protected:
        virtual void Notify(const SfxHint& rHint) override
        {
            if(rHint.GetId() == SfxHintId::Dying)
                m_pHeadFootFormat = nullptr;
        }
};

rtl::Reference<SwXHeadFootText> SwXHeadFootText::CreateXHeadFootText(
        SwFrameFormat& rHeadFootFormat,
        const bool bIsHeader)
{
    // re-use existing SwXHeadFootText
    // #i105557#: do not iterate over the registered clients: race condition
    rtl::Reference<SwXHeadFootText> xText = dynamic_cast<SwXHeadFootText*>(rHeadFootFormat.GetXObject().get().get());
    if(!xText.is())
    {
        xText = new SwXHeadFootText(rHeadFootFormat, bIsHeader);
        rHeadFootFormat.SetXObject(static_cast<cppu::OWeakObject*>(xText.get()));
    }
    return xText;
}

SwXHeadFootText::SwXHeadFootText(SwFrameFormat& rHeadFootFormat, const bool bIsHeader)
    : SwXText(
            rHeadFootFormat.GetDoc(),
            bIsHeader ? CursorType::Header : CursorType::Footer)
    , m_pImpl(new SwXHeadFootText::Impl(rHeadFootFormat, bIsHeader))
{
}

SwXHeadFootText::~SwXHeadFootText()
{ }

OUString SAL_CALL
SwXHeadFootText::getImplementationName()
{
  return {u"SwXHeadFootText"_ustr};
}

sal_Bool SAL_CALL SwXHeadFootText::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL
SwXHeadFootText::getSupportedServiceNames()
{
    return {u"com.sun.star.text.Text"_ustr};
}

const SwStartNode* SwXHeadFootText::GetStartNode() const
{
    const SwStartNode* pSttNd = nullptr;
    SwFrameFormat* const pHeadFootFormat = m_pImpl->GetHeadFootFormat();
    if(pHeadFootFormat)
    {
        const SwFormatContent& rFlyContent = pHeadFootFormat->GetContent();
        if(rFlyContent.GetContentIdx())
        {
            pSttNd = rFlyContent.GetContentIdx()->GetNode().GetStartNode();
        }
    }
    return pSttNd;
}

uno::Sequence<uno::Type> SAL_CALL SwXHeadFootText::getTypes()
{
    return ::comphelper::concatSequences(
        SwXHeadFootText_Base::getTypes(),
        SwXText::getTypes());
}

uno::Sequence<sal_Int8> SAL_CALL SwXHeadFootText::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Any SAL_CALL SwXHeadFootText::queryInterface(const uno::Type& rType)
{
    const uno::Any ret = SwXHeadFootText_Base::queryInterface(rType);
    return (ret.getValueType() == cppu::UnoType<void>::get())
        ? SwXText::queryInterface(rType)
        : ret;
}

rtl::Reference<SwXTextCursor> SwXHeadFootText::CreateTextCursor(const bool bIgnoreTables)
{
    SwFrameFormat & rHeadFootFormat( m_pImpl->GetHeadFootFormatOrThrow() );

    const SwFormatContent& rFlyContent = rHeadFootFormat.GetContent();
    const SwNode& rNode = rFlyContent.GetContentIdx()->GetNode();
    SwPosition aPos(rNode);
    rtl::Reference<SwXTextCursor> pXCursor = new SwXTextCursor(*GetDoc(), this,
            (m_pImpl->m_bIsHeader) ? CursorType::Header : CursorType::Footer, aPos);
    auto& rUnoCursor(pXCursor->GetCursor());
    rUnoCursor.Move(fnMoveForward, GoInNode);

    // save current start node to be able to check if there is content
    // after the table - otherwise the cursor would be in the body text!
    SwStartNode const*const pOwnStartNode = rNode.FindSttNodeByType(
            (m_pImpl->m_bIsHeader) ? SwHeaderStartNode : SwFooterStartNode);

    if (!bIgnoreTables)
    {
        // is there a table here?
        SwTableNode* pTableNode = rUnoCursor.GetPointNode().FindTableNode();
        while (pTableNode)
        {
            rUnoCursor.GetPoint()->Assign(*pTableNode->EndOfSectionNode());
            SwContentNode* pCont = SwNodes::GoNext(rUnoCursor.GetPoint());
            pTableNode = pCont->FindTableNode();
        }
    }
    SwStartNode const*const pNewStartNode = rUnoCursor.GetPointNode().FindSttNodeByType(
            (m_pImpl->m_bIsHeader) ? SwHeaderStartNode : SwFooterStartNode);
    if (!pNewStartNode || (pNewStartNode != pOwnStartNode))
    {
        throw uno::RuntimeException(u"no text available"_ustr);
    }
    return pXCursor;
}

rtl::Reference< SwXTextCursor >
SwXHeadFootText::createXTextCursor()
{
    return CreateTextCursor(false);
}

rtl::Reference<SwXTextCursor> SwXHeadFootText::createXTextCursorByRange(
    const uno::Reference<text::XTextRange>& xTextPosition)
{
    SwFrameFormat& rHeadFootFormat( m_pImpl->GetHeadFootFormatOrThrow() );

    SwUnoInternalPaM aPam(*GetDoc());
    if (!sw::XTextRangeToSwPaM(aPam, xTextPosition))
    {
        throw uno::RuntimeException(cInvalidObject);
    }

    SwNode& rNode = rHeadFootFormat.GetContent().GetContentIdx()->GetNode();
    SwPosition aPos(rNode);
    SwPaM aHFPam(aPos);
    aHFPam.Move(fnMoveForward, GoInNode);
    SwStartNode* const pOwnStartNode = aHFPam.GetPointNode().FindSttNodeByType(
            (m_pImpl->m_bIsHeader) ? SwHeaderStartNode : SwFooterStartNode);
    SwStartNode* const p1 = aPam.GetPointNode().FindSttNodeByType(
            (m_pImpl->m_bIsHeader) ? SwHeaderStartNode : SwFooterStartNode);
    if (p1 == pOwnStartNode)
    {
        return new SwXTextCursor(
                    *GetDoc(),
                    this,
                    (m_pImpl->m_bIsHeader) ? CursorType::Header : CursorType::Footer,
                    *aPam.GetPoint(), aPam.GetMark());
    }
    return nullptr;
}

uno::Reference<container::XEnumeration> SAL_CALL SwXHeadFootText::createEnumeration()
{
    SolarMutexGuard aGuard;
    SwFrameFormat& rHeadFootFormat(m_pImpl->GetHeadFootFormatOrThrow());

    const SwFormatContent& rFlyContent = rHeadFootFormat.GetContent();
    const SwNode& rNode = rFlyContent.GetContentIdx()->GetNode();
    SwPosition aPos(rNode);
    auto pUnoCursor(GetDoc()->CreateUnoCursor(aPos));
    pUnoCursor->Move(fnMoveForward, GoInNode);
    return SwXParagraphEnumeration::Create(
            this,
            pUnoCursor,
            (m_pImpl->m_bIsHeader)
                ? CursorType::Header
                : CursorType::Footer);
}

uno::Type SAL_CALL SwXHeadFootText::getElementType()
    { return cppu::UnoType<text::XTextRange>::get(); }

sal_Bool SAL_CALL SwXHeadFootText::hasElements()
    { return true; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
