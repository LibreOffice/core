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

#include <unocontentcontrol.hxx>

#include <mutex>

#include <com/sun/star/text/XWordCursor.hpp>

#include <comphelper/interfacecontainer4.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <formatcontentcontrol.hxx>
#include <ndtxt.hxx>
#include <textcontentcontrol.hxx>
#include <unotext.hxx>
#include <unotextcursor.hxx>
#include <unotextrange.hxx>
#include <doc.hxx>
#include <unoport.hxx>

using namespace com::sun::star;

namespace
{
/// UNO API wrapper around the text inside an SwXContentControl.
class SwXContentControlText : public cppu::OWeakObject, public SwXText
{
private:
    SwXContentControl& m_rContentControl;

    void PrepareForAttach(uno::Reference<text::XTextRange>& xRange, const SwPaM& rPam) override;

protected:
    const SwStartNode* GetStartNode() const override;
    uno::Reference<text::XTextCursor> CreateCursor() override;

public:
    SwXContentControlText(SwDoc& rDoc, SwXContentControl& rContentControl);

    /// SwXText::Invalidate() is protected.
    using SwXText::Invalidate;

    // XInterface
    void SAL_CALL acquire() noexcept override { cppu::OWeakObject::acquire(); }
    void SAL_CALL release() noexcept override { cppu::OWeakObject::release(); }

    // XTypeProvider
    uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override;

    // XText
    uno::Reference<text::XTextCursor> SAL_CALL createTextCursor() override;
    uno::Reference<text::XTextCursor> SAL_CALL
    createTextCursorByRange(const uno::Reference<text::XTextRange>& xTextPosition) override;
};
}

SwXContentControlText::SwXContentControlText(SwDoc& rDoc, SwXContentControl& rContentControl)
    : SwXText(&rDoc, CursorType::ContentControl)
    , m_rContentControl(rContentControl)
{
}

const SwStartNode* SwXContentControlText::GetStartNode() const
{
    auto pParent = dynamic_cast<SwXText*>(m_rContentControl.GetParentText().get());
    return pParent ? pParent->GetStartNode() : nullptr;
}

void SwXContentControlText::PrepareForAttach(uno::Reference<text::XTextRange>& xRange,
                                             const SwPaM& rPam)
{
    // Create a new cursor to prevent modifying SwXTextRange.
    xRange = static_cast<text::XWordCursor*>(
        new SwXTextCursor(*GetDoc(), &m_rContentControl, CursorType::ContentControl,
                          *rPam.GetPoint(), (rPam.HasMark()) ? rPam.GetMark() : nullptr));
}

uno::Reference<text::XTextCursor> SwXContentControlText::CreateCursor()
{
    uno::Reference<text::XTextCursor> xRet;
    if (IsValid())
    {
        SwTextNode* pTextNode;
        sal_Int32 nContentControlStart;
        sal_Int32 nContentControlEnd;
        bool bSuccess = m_rContentControl.SetContentRange(pTextNode, nContentControlStart,
                                                          nContentControlEnd);
        if (bSuccess)
        {
            SwPosition aPos(*pTextNode, nContentControlStart);
            xRet = static_cast<text::XWordCursor*>(
                new SwXTextCursor(*GetDoc(), &m_rContentControl, CursorType::ContentControl, aPos));
        }
    }
    return xRet;
}

uno::Sequence<sal_Int8> SAL_CALL SwXContentControlText::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XText
uno::Reference<text::XTextCursor> SAL_CALL SwXContentControlText::createTextCursor()
{
    return CreateCursor();
}

uno::Reference<text::XTextCursor> SAL_CALL SwXContentControlText::createTextCursorByRange(
    const uno::Reference<text::XTextRange>& xTextPosition)
{
    const uno::Reference<text::XTextCursor> xCursor(CreateCursor());
    xCursor->gotoRange(xTextPosition, false);
    return xCursor;
}

/**
 * The inner part SwXContentControl, which is deleted with a locked SolarMutex.
 *
 * The content control has a cached list of text portions for its contents.  This list is created by
 * SwXTextPortionEnumeration.  The content control listens at the SwTextNode and throws away the
 * cache when the text node changes.
 */
class SwXContentControl::Impl : public SvtListener
{
public:
    uno::WeakReference<uno::XInterface> m_wThis;
    // Just for OInterfaceContainerHelper4.
    std::mutex m_Mutex;
    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_EventListeners;
    std::unique_ptr<const TextRangeList_t> m_pTextPortions;
    // 3 possible states: not attached, attached, disposed
    bool m_bIsDisposed;
    bool m_bIsDescriptor;
    uno::Reference<text::XText> m_xParentText;
    rtl::Reference<SwXContentControlText> m_xText;
    SwContentControl* m_pContentControl;

    Impl(SwXContentControl& rThis, SwDoc& rDoc, SwContentControl* pContentControl,
         const uno::Reference<text::XText>& xParentText,
         std::unique_ptr<const TextRangeList_t> pPortions)
        : m_pTextPortions(std::move(pPortions))
        , m_bIsDisposed(false)
        , m_bIsDescriptor(pContentControl == nullptr)
        , m_xParentText(xParentText)
        , m_xText(new SwXContentControlText(rDoc, rThis))
        , m_pContentControl(pContentControl)
    {
        if (m_pContentControl)
        {
            StartListening(m_pContentControl->GetNotifier());
        }
    }

    const SwContentControl* GetContentControl() const;

protected:
    void Notify(const SfxHint& rHint) override;
};

const SwContentControl* SwXContentControl::Impl::GetContentControl() const
{
    return m_pContentControl;
}

// sw::BroadcastingModify
void SwXContentControl::Impl::Notify(const SfxHint& rHint)
{
    // throw away cache (SwTextNode changed)
    m_pTextPortions.reset();

    if (rHint.GetId() != SfxHintId::Dying && rHint.GetId() != SfxHintId::Deinitializing)
        return;

    m_bIsDisposed = true;
    m_pContentControl = nullptr;
    m_xText->Invalidate();
    uno::Reference<uno::XInterface> xThis(m_wThis);
    if (!xThis.is())
    {
        // If UNO object is already dead, don't refer to it in an event.
        return;
    }
    lang::EventObject aEvent(xThis);
    std::unique_lock aGuard(m_Mutex);
    m_EventListeners.disposeAndClear(aGuard, aEvent);
}

uno::Reference<text::XText> SwXContentControl::GetParentText() const
{
    return m_pImpl->m_xParentText;
}

SwXContentControl::SwXContentControl(SwDoc* pDoc, SwContentControl* pContentControl,
                                     const uno::Reference<text::XText>& xParentText,
                                     std::unique_ptr<const TextRangeList_t> pPortions)
    : m_pImpl(new SwXContentControl::Impl(*this, *pDoc, pContentControl, xParentText,
                                          std::move(pPortions)))
{
}

SwXContentControl::SwXContentControl(SwDoc* pDoc)
    : m_pImpl(new SwXContentControl::Impl(*this, *pDoc, nullptr, nullptr, nullptr))
{
}

SwXContentControl::~SwXContentControl() {}

uno::Reference<text::XTextContent> SwXContentControl::CreateXContentControl(SwDoc& rDoc)
{
    rtl::Reference<SwXContentControl> xContentControl(new SwXContentControl(&rDoc));
    uno::Reference<text::XTextContent> xTextContent(xContentControl);
    xContentControl->m_pImpl->m_wThis = xTextContent;
    return xContentControl;
}

uno::Reference<text::XTextContent>
SwXContentControl::CreateXContentControl(SwContentControl& rContentControl,
                                         const uno::Reference<text::XText>& xParent,
                                         std::unique_ptr<const TextRangeList_t>&& pPortions)
{
    // re-use existing SwXContentControl
    uno::Reference<text::XTextContent> xContentControl(rContentControl.GetXContentControl());
    if (xContentControl.is())
    {
        if (pPortions)
        {
            // Set the cache in the XContentControl to the given portions.
            auto pXContentControl
                = comphelper::getFromUnoTunnel<SwXContentControl>(xContentControl);
            assert(pXContentControl);
            // The content control must always be created with the complete content.  If
            // SwXTextPortionEnumeration is created for a selection, it must be checked that the
            // content control is contained in the selection.
            pXContentControl->m_pImpl->m_pTextPortions = std::move(pPortions);
            if (pXContentControl->m_pImpl->m_xParentText.get() != xParent.get())
            {
                SAL_WARN("sw.uno", "SwXContentControl with different parent");
                pXContentControl->m_pImpl->m_xParentText.set(xParent);
            }
        }
        return xContentControl;
    }

    // Create new SwXContentControl.
    SwTextNode* pTextNode = rContentControl.GetTextNode();
    if (!pTextNode)
    {
        SAL_WARN("sw.uno", "CreateXContentControl: no text node");
        return nullptr;
    }
    uno::Reference<text::XText> xParentText(xParent);
    if (!xParentText.is())
    {
        SwTextContentControl* pTextAttr = rContentControl.GetTextAttr();
        if (!pTextAttr)
        {
            SAL_WARN("sw.uno", "CreateXContentControl: no text attr");
            return nullptr;
        }
        SwPosition aPos(*pTextNode, pTextAttr->GetStart());
        xParentText.set(sw::CreateParentXText(pTextNode->GetDoc(), aPos));
    }
    if (!xParentText.is())
    {
        return nullptr;
    }
    rtl::Reference<SwXContentControl> pXContentControl = new SwXContentControl(
        &pTextNode->GetDoc(), &rContentControl, xParentText, std::move(pPortions));
    xContentControl.set(pXContentControl);
    rContentControl.SetXContentControl(xContentControl);
    pXContentControl->m_pImpl->m_wThis = xContentControl;
    return xContentControl;
}

bool SwXContentControl::SetContentRange(SwTextNode*& rpNode, sal_Int32& rStart,
                                        sal_Int32& rEnd) const
{
    const SwContentControl* pContentControl = m_pImpl->GetContentControl();
    if (pContentControl)
    {
        const SwTextContentControl* pTextAttr = pContentControl->GetTextAttr();
        if (pTextAttr)
        {
            rpNode = pContentControl->GetTextNode();
            if (rpNode)
            {
                // rStart points at the first position within the content control.
                rStart = pTextAttr->GetStart() + 1;
                rEnd = *pTextAttr->End();
                return true;
            }
        }
    }
    return false;
}

bool SwXContentControl::CheckForOwnMemberContentControl(const SwPaM& rPam, bool bAbsorb)
{
    SwTextNode* pTextNode;
    sal_Int32 nContentControlStart;
    sal_Int32 nContentControlEnd;
    bool bSuccess = SetContentRange(pTextNode, nContentControlStart, nContentControlEnd);
    if (!bSuccess)
    {
        SAL_WARN("sw.core", "SwXContentControl::CheckForOwnMemberContentControl: no pam");
        throw lang::DisposedException();
    }

    const SwPosition* pStartPos(rPam.Start());
    if (&pStartPos->nNode.GetNode() != pTextNode)
    {
        throw lang::IllegalArgumentException(
            "trying to insert into a nesting text content, but start "
            "of text range not in same paragraph as text content",
            nullptr, 0);
    }
    bool bForceExpandHints(false);
    sal_Int32 nStartPos = pStartPos->nContent.GetIndex();
    if ((nStartPos < nContentControlStart) || (nStartPos > nContentControlEnd))
    {
        throw lang::IllegalArgumentException(
            "trying to insert into a nesting text content, but start "
            "of text range not inside text content",
            nullptr, 0);
    }
    else if (nStartPos == nContentControlEnd)
    {
        bForceExpandHints = true;
    }
    if (rPam.HasMark() && bAbsorb)
    {
        const SwPosition* pEndPos = rPam.End();
        if (&pEndPos->nNode.GetNode() != pTextNode)
        {
            throw lang::IllegalArgumentException(
                "trying to insert into a nesting text content, but end "
                "of text range not in same paragraph as text content",
                nullptr, 0);
        }
        sal_Int32 nEndPos = pEndPos->nContent.GetIndex();
        if ((nEndPos < nContentControlStart) || (nEndPos > nContentControlEnd))
        {
            throw lang::IllegalArgumentException(
                "trying to insert into a nesting text content, but end "
                "of text range not inside text content",
                nullptr, 0);
        }
        else if (nEndPos == nContentControlEnd)
        {
            bForceExpandHints = true;
        }
    }
    return bForceExpandHints;
}

const uno::Sequence<sal_Int8>& SwXContentControl::getUnoTunnelId()
{
    static const comphelper::UnoIdInit theSwXContentControlUnoTunnelId;
    return theSwXContentControlUnoTunnelId.getSeq();
}

// XUnoTunnel
sal_Int64 SAL_CALL SwXContentControl::getSomething(const uno::Sequence<sal_Int8>& rId)
{
    return comphelper::getSomethingImpl<SwXContentControl>(rId, this);
}

// XServiceInfo
OUString SAL_CALL SwXContentControl::getImplementationName() { return "SwXContentControl"; }

sal_Bool SAL_CALL SwXContentControl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL SwXContentControl::getSupportedServiceNames()
{
    return { "com.sun.star.text.TextContent", "com.sun.star.text.ContentControl" };
}

// XComponent
void SAL_CALL
SwXContentControl::addEventListener(const uno::Reference<lang::XEventListener>& xListener)
{
    std::unique_lock aGuard(m_pImpl->m_Mutex);
    m_pImpl->m_EventListeners.addInterface(aGuard, xListener);
}

void SAL_CALL
SwXContentControl::removeEventListener(const uno::Reference<lang::XEventListener>& xListener)
{
    std::unique_lock aGuard(m_pImpl->m_Mutex);
    m_pImpl->m_EventListeners.removeInterface(aGuard, xListener);
}

void SAL_CALL SwXContentControl::dispose()
{
    SolarMutexGuard g;

    if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->m_pTextPortions.reset();
        lang::EventObject aEvent(static_cast<::cppu::OWeakObject&>(*this));
        std::unique_lock aGuard(m_pImpl->m_Mutex);
        m_pImpl->m_EventListeners.disposeAndClear(aGuard, aEvent);
        m_pImpl->m_bIsDisposed = true;
        m_pImpl->m_xText->Invalidate();
    }
    else if (!m_pImpl->m_bIsDisposed)
    {
        SwTextNode* pTextNode;
        sal_Int32 nContentControlStart;
        sal_Int32 nContentControlEnd;
        bool bSuccess = SetContentRange(pTextNode, nContentControlStart, nContentControlEnd);
        if (!bSuccess)
        {
            SAL_WARN("sw.core", "SwXContentControl::dispose: no pam");
        }
        else
        {
            // -1 because of CH_TXTATR
            SwPaM aPam(*pTextNode, nContentControlStart - 1, *pTextNode, nContentControlEnd);
            SwDoc& rDoc(pTextNode->GetDoc());
            rDoc.getIDocumentContentOperations().DeleteAndJoin(aPam);

            // removal should call Modify and do the dispose
            assert(m_pImpl->m_bIsDisposed);
        }
    }
}

void SwXContentControl::AttachImpl(const uno::Reference<text::XTextRange>& xTextRange,
                                   sal_uInt16 nWhich)
{
    SolarMutexGuard aGuard;

    if (m_pImpl->m_bIsDisposed)
    {
        throw lang::DisposedException();
    }
    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException("SwXContentControl::AttachImpl(): already attached",
                                    static_cast<::cppu::OWeakObject*>(this));
    }

    uno::Reference<lang::XUnoTunnel> xRangeTunnel(xTextRange, uno::UNO_QUERY);
    if (!xRangeTunnel.is())
    {
        throw lang::IllegalArgumentException(
            "SwXContentControl::AttachImpl(): argument is no XUnoTunnel",
            static_cast<::cppu::OWeakObject*>(this), 0);
    }
    SwXTextRange* pRange = comphelper::getFromUnoTunnel<SwXTextRange>(xRangeTunnel);
    OTextCursorHelper* pCursor
        = pRange ? nullptr : comphelper::getFromUnoTunnel<OTextCursorHelper>(xRangeTunnel);
    if (!pRange && !pCursor)
    {
        throw lang::IllegalArgumentException(
            "SwXContentControl::AttachImpl(): argument not supported type",
            static_cast<::cppu::OWeakObject*>(this), 0);
    }

    SwDoc* pDoc = pRange ? &pRange->GetDoc() : pCursor->GetDoc();
    if (!pDoc)
    {
        throw lang::IllegalArgumentException(
            "SwXContentControl::AttachImpl(): argument has no SwDoc",
            static_cast<::cppu::OWeakObject*>(this), 0);
    }

    SwUnoInternalPaM aPam(*pDoc);
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);

    UnoActionContext aContext(pDoc);

    auto pTextCursor = dynamic_cast<SwXTextCursor*>(pCursor);
    bool bForceExpandHints = pTextCursor && pTextCursor->IsAtEndOfContentControl();
    SetAttrMode nInsertFlags = bForceExpandHints
                                   ? (SetAttrMode::FORCEHINTEXPAND | SetAttrMode::DONTEXPAND)
                                   : SetAttrMode::DONTEXPAND;

    auto pContentControl = std::make_shared<SwContentControl>(nullptr);
    SwFormatContentControl aContentControl(pContentControl, nWhich);
    bool bSuccess
        = pDoc->getIDocumentContentOperations().InsertPoolItem(aPam, aContentControl, nInsertFlags);
    SwTextAttr* pTextAttr = pContentControl->GetTextAttr();
    if (!bSuccess)
    {
        throw lang::IllegalArgumentException(
            "SwXContentControl::AttachImpl(): cannot create meta: range invalid?",
            static_cast<::cppu::OWeakObject*>(this), 1);
    }
    if (!pTextAttr)
    {
        SAL_WARN("sw.core", "content control inserted, but has no text attribute?");
        throw uno::RuntimeException(
            "SwXContentControl::AttachImpl(): cannot create content control",
            static_cast<::cppu::OWeakObject*>(this));
    }

    m_pImpl->EndListeningAll();
    m_pImpl->m_pContentControl = pContentControl.get();
    m_pImpl->StartListening(pContentControl->GetNotifier());
    pContentControl->SetXContentControl(uno::Reference<text::XTextContent>(this));

    m_pImpl->m_xParentText = sw::CreateParentXText(*pDoc, *aPam.GetPoint());

    m_pImpl->m_bIsDescriptor = false;
}

// XTextContent
void SAL_CALL SwXContentControl::attach(const uno::Reference<text::XTextRange>& xTextRange)
{
    return SwXContentControl::AttachImpl(xTextRange, RES_TXTATR_CONTENTCONTROL);
}

uno::Reference<text::XTextRange> SAL_CALL SwXContentControl::getAnchor()
{
    SolarMutexGuard g;

    if (m_pImpl->m_bIsDisposed)
    {
        throw lang::DisposedException();
    }
    if (m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException("SwXContentControl::getAnchor(): not inserted",
                                    static_cast<::cppu::OWeakObject*>(this));
    }

    SwTextNode* pTextNode;
    sal_Int32 nContentControlStart;
    sal_Int32 nContentControlEnd;
    bool bSuccess = SetContentRange(pTextNode, nContentControlStart, nContentControlEnd);
    if (!bSuccess)
    {
        SAL_WARN("sw.core", "no pam");
        throw lang::DisposedException("SwXContentControl::getAnchor(): not attached",
                                      static_cast<::cppu::OWeakObject*>(this));
    }

    SwPosition aStart(*pTextNode, nContentControlStart - 1); // -1 due to CH_TXTATR
    SwPosition aEnd(*pTextNode, nContentControlEnd);
    return SwXTextRange::CreateXTextRange(pTextNode->GetDoc(), aStart, &aEnd);
}

// XTextRange
uno::Reference<text::XText> SAL_CALL SwXContentControl::getText() { return this; }

uno::Reference<text::XTextRange> SAL_CALL SwXContentControl::getStart()
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->getStart();
}

uno::Reference<text::XTextRange> SAL_CALL SwXContentControl::getEnd()
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->getEnd();
}

OUString SAL_CALL SwXContentControl::getString()
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->getString();
}

void SAL_CALL SwXContentControl::setString(const OUString& rString)
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->setString(rString);
}

// XSimpleText
uno::Reference<text::XTextCursor> SAL_CALL SwXContentControl::createTextCursor()
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->createTextCursor();
}

uno::Reference<text::XTextCursor> SAL_CALL
SwXContentControl::createTextCursorByRange(const uno::Reference<text::XTextRange>& xTextPosition)
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->createTextCursorByRange(xTextPosition);
}

void SAL_CALL SwXContentControl::insertString(const uno::Reference<text::XTextRange>& xRange,
                                              const OUString& rString, sal_Bool bAbsorb)
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->insertString(xRange, rString, bAbsorb);
}

void SAL_CALL SwXContentControl::insertControlCharacter(
    const uno::Reference<text::XTextRange>& xRange, sal_Int16 nControlCharacter, sal_Bool bAbsorb)
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->insertControlCharacter(xRange, nControlCharacter, bAbsorb);
}

// XText
void SAL_CALL SwXContentControl::insertTextContent(
    const uno::Reference<text::XTextRange>& xRange,
    const uno::Reference<text::XTextContent>& xContent, sal_Bool bAbsorb)
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->insertTextContent(xRange, xContent, bAbsorb);
}

void SAL_CALL
SwXContentControl::removeTextContent(const uno::Reference<text::XTextContent>& xContent)
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->removeTextContent(xContent);
}

// XElementAccess
uno::Type SAL_CALL SwXContentControl::getElementType()
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL SwXContentControl::hasElements()
{
    SolarMutexGuard g;
    return m_pImpl->m_pContentControl != nullptr;
}

// XEnumerationAccess
uno::Reference<container::XEnumeration> SAL_CALL SwXContentControl::createEnumeration()
{
    SolarMutexGuard g;

    if (m_pImpl->m_bIsDisposed)
    {
        throw lang::DisposedException();
    }
    if (m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException("createEnumeration(): not inserted",
                                    static_cast<::cppu::OWeakObject*>(this));
    }

    SwTextNode* pTextNode;
    sal_Int32 nContentControlStart;
    sal_Int32 nContentControlEnd;
    bool bSuccess = SetContentRange(pTextNode, nContentControlStart, nContentControlEnd);
    if (!bSuccess)
    {
        SAL_WARN("sw.core", "no pam");
        throw lang::DisposedException();
    }

    SwPaM aPam(*pTextNode, nContentControlStart);

    if (!m_pImpl->m_pTextPortions)
    {
        return new SwXTextPortionEnumeration(aPam, GetParentText(), nContentControlStart,
                                             nContentControlEnd);
    }
    else
    {
        return new SwXTextPortionEnumeration(aPam, std::deque(*m_pImpl->m_pTextPortions));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
