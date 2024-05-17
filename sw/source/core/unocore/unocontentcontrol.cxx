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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <comphelper/interfacecontainer4.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <formatcontentcontrol.hxx>
#include <ndtxt.hxx>
#include <textcontentcontrol.hxx>
#include <unotext.hxx>
#include <unotextcursor.hxx>
#include <unotextrange.hxx>
#include <doc.hxx>
#include <unoport.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <utility>

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
    virtual rtl::Reference<SwXTextCursor> createXTextCursor() override;
    virtual rtl::Reference<SwXTextCursor> createXTextCursorByRange(
        const ::css::uno::Reference<::css::text::XTextRange>& aTextPosition) override;
};
}

SwXContentControlText::SwXContentControlText(SwDoc& rDoc, SwXContentControl& rContentControl)
    : SwXText(&rDoc, CursorType::ContentControl)
    , m_rContentControl(rContentControl)
{
}

const SwStartNode* SwXContentControlText::GetStartNode() const
{
    SwXText* pParent = m_rContentControl.GetParentText().get();
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

rtl::Reference<SwXTextCursor> SwXContentControlText::createXTextCursor()
{
    rtl::Reference<SwXTextCursor> xRet;
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
            xRet = new SwXTextCursor(*GetDoc(), &m_rContentControl, CursorType::ContentControl,
                                     aPos);
        }
    }
    return xRet;
}

uno::Sequence<sal_Int8> SAL_CALL SwXContentControlText::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XText

rtl::Reference<SwXTextCursor> SwXContentControlText::createXTextCursorByRange(
    const uno::Reference<text::XTextRange>& xTextPosition)
{
    const rtl::Reference<SwXTextCursor> xCursor(createXTextCursor());
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
    unotools::WeakReference<SwXContentControl> m_wThis;
    // Just for OInterfaceContainerHelper4.
    std::mutex m_Mutex;
    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_EventListeners;
    std::unique_ptr<const TextRangeList_t> m_pTextPortions;
    // 3 possible states: not attached, attached, disposed
    bool m_bIsDisposed;
    bool m_bIsDescriptor;
    css::uno::Reference<SwXText> m_xParentText;
    rtl::Reference<SwXContentControlText> m_xText;
    SwContentControl* m_pContentControl;
    bool m_bShowingPlaceHolder;
    bool m_bCheckbox;
    bool m_bChecked;
    OUString m_aCheckedState;
    OUString m_aUncheckedState;
    std::vector<SwContentControlListItem> m_aListItems;
    bool m_bPicture;
    bool m_bDate;
    OUString m_aDateFormat;
    OUString m_aDateLanguage;
    OUString m_aCurrentDate;
    bool m_bPlainText;
    bool m_bComboBox;
    bool m_bDropDown;
    OUString m_aPlaceholderDocPart;
    OUString m_aDataBindingPrefixMappings;
    OUString m_aDataBindingXpath;
    OUString m_aDataBindingStoreItemID;
    OUString m_aColor;
    OUString m_aAppearance;
    OUString m_aAlias;
    OUString m_aTag;
    sal_Int32 m_nId;
    sal_uInt32 m_nTabIndex;
    OUString m_aLock;
    OUString m_aMultiLine;

    Impl(SwXContentControl& rThis, SwDoc& rDoc, SwContentControl* pContentControl,
         css::uno::Reference<SwXText> xParentText, std::unique_ptr<const TextRangeList_t> pPortions)
        : m_pTextPortions(std::move(pPortions))
        , m_bIsDisposed(false)
        , m_bIsDescriptor(pContentControl == nullptr)
        , m_xParentText(std::move(xParentText))
        , m_xText(new SwXContentControlText(rDoc, rThis))
        , m_pContentControl(pContentControl)
        , m_bShowingPlaceHolder(false)
        , m_bCheckbox(false)
        , m_bChecked(false)
        , m_bPicture(false)
        , m_bDate(false)
        , m_bPlainText(false)
        , m_bComboBox(false)
        , m_bDropDown(false)
        , m_nId(0)
        , m_nTabIndex(0)
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

const css::uno::Reference<SwXText>& SwXContentControl::GetParentText() const
{
    return m_pImpl->m_xParentText;
}

SwXContentControl::SwXContentControl(SwDoc* pDoc, SwContentControl* pContentControl,
                                     const css::uno::Reference<SwXText>& xParentText,
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

rtl::Reference<SwXContentControl> SwXContentControl::CreateXContentControl(SwDoc& rDoc)
{
    rtl::Reference<SwXContentControl> xContentControl(new SwXContentControl(&rDoc));
    xContentControl->m_pImpl->m_wThis = xContentControl.get();
    return xContentControl;
}

rtl::Reference<SwXContentControl>
SwXContentControl::CreateXContentControl(SwContentControl& rContentControl,
                                         const css::uno::Reference<SwXText>& xParent,
                                         std::unique_ptr<const TextRangeList_t>&& pPortions)
{
    // re-use existing SwXContentControl
    rtl::Reference<SwXContentControl> xContentControl(rContentControl.GetXContentControl());
    if (xContentControl.is())
    {
        if (pPortions)
        {
            // The content control must always be created with the complete content.  If
            // SwXTextPortionEnumeration is created for a selection, it must be checked that the
            // content control is contained in the selection.
            xContentControl->m_pImpl->m_pTextPortions = std::move(pPortions);
            if (xContentControl->m_pImpl->m_xParentText.get() != xParent.get())
            {
                SAL_WARN("sw.uno", "SwXContentControl with different parent");
                xContentControl->m_pImpl->m_xParentText = xParent;
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
    css::uno::Reference<SwXText> xParentText(xParent);
    if (!xParentText.is())
    {
        SwTextContentControl* pTextAttr = rContentControl.GetTextAttr();
        if (!pTextAttr)
        {
            SAL_WARN("sw.uno", "CreateXContentControl: no text attr");
            return nullptr;
        }
        SwPosition aPos(*pTextNode, pTextAttr->GetStart());
        xParentText = sw::CreateParentXText(pTextNode->GetDoc(), aPos);
    }
    if (!xParentText.is())
    {
        return nullptr;
    }
    xContentControl = new SwXContentControl(&pTextNode->GetDoc(), &rContentControl, xParentText,
                                            std::move(pPortions));
    rContentControl.SetXContentControl(xContentControl);
    xContentControl->m_pImpl->m_wThis = xContentControl.get();
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
                // rEnd points at the last position within the content control.
                rEnd = *pTextAttr->End() - 1;
                return true;
            }
        }
    }
    return false;
}

// XServiceInfo
OUString SAL_CALL SwXContentControl::getImplementationName() { return u"SwXContentControl"_ustr; }

sal_Bool SAL_CALL SwXContentControl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL SwXContentControl::getSupportedServiceNames()
{
    return { u"com.sun.star.text.TextContent"_ustr, u"com.sun.star.text.ContentControl"_ustr };
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
        lang::EventObject aEvent(getXWeak());
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
        throw uno::RuntimeException(u"SwXContentControl::AttachImpl(): already attached"_ustr,
                                    getXWeak());
    }

    SwXTextRange* pRange = dynamic_cast<SwXTextRange*>(xTextRange.get());
    OTextCursorHelper* pCursor
        = pRange ? nullptr : dynamic_cast<OTextCursorHelper*>(xTextRange.get());
    if (!pRange && !pCursor)
    {
        throw lang::IllegalArgumentException(
            u"SwXContentControl::AttachImpl(): argument not supported type"_ustr, getXWeak(), 0);
    }

    SwDoc* pDoc = pRange ? &pRange->GetDoc() : pCursor->GetDoc();
    if (!pDoc)
    {
        throw lang::IllegalArgumentException(
            u"SwXContentControl::AttachImpl(): argument has no SwDoc"_ustr, getXWeak(), 0);
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

    pContentControl->SetShowingPlaceHolder(m_pImpl->m_bShowingPlaceHolder);
    pContentControl->SetCheckbox(m_pImpl->m_bCheckbox);
    pContentControl->SetChecked(m_pImpl->m_bChecked);
    pContentControl->SetCheckedState(m_pImpl->m_aCheckedState);
    pContentControl->SetUncheckedState(m_pImpl->m_aUncheckedState);
    pContentControl->SetListItems(m_pImpl->m_aListItems);
    pContentControl->SetPicture(m_pImpl->m_bPicture);
    pContentControl->SetDate(m_pImpl->m_bDate);
    pContentControl->SetDateFormat(m_pImpl->m_aDateFormat);
    pContentControl->SetDateLanguage(m_pImpl->m_aDateLanguage);
    pContentControl->SetCurrentDate(m_pImpl->m_aCurrentDate);
    pContentControl->SetPlainText(m_pImpl->m_bPlainText);
    pContentControl->SetComboBox(m_pImpl->m_bComboBox);
    pContentControl->SetDropDown(m_pImpl->m_bDropDown);
    pContentControl->SetPlaceholderDocPart(m_pImpl->m_aPlaceholderDocPart);
    pContentControl->SetDataBindingPrefixMappings(m_pImpl->m_aDataBindingPrefixMappings);
    pContentControl->SetDataBindingXpath(m_pImpl->m_aDataBindingXpath);
    pContentControl->SetDataBindingStoreItemID(m_pImpl->m_aDataBindingStoreItemID);
    pContentControl->SetColor(m_pImpl->m_aColor);
    pContentControl->SetAppearance(m_pImpl->m_aAppearance);
    pContentControl->SetAlias(m_pImpl->m_aAlias);
    pContentControl->SetTag(m_pImpl->m_aTag);
    pContentControl->SetId(m_pImpl->m_nId);
    pContentControl->SetTabIndex(m_pImpl->m_nTabIndex);
    pContentControl->SetLock(m_pImpl->m_aLock);
    pContentControl->SetMultiLine(m_pImpl->m_aMultiLine);

    SwFormatContentControl aContentControl(pContentControl, nWhich);
    bool bSuccess
        = pDoc->getIDocumentContentOperations().InsertPoolItem(aPam, aContentControl, nInsertFlags);
    SwTextAttr* pTextAttr = pContentControl->GetTextAttr();
    if (!bSuccess)
    {
        throw lang::IllegalArgumentException(
            u"SwXContentControl::AttachImpl(): cannot create content control: invalid range"_ustr,
            getXWeak(), 1);
    }
    if (!pTextAttr)
    {
        SAL_WARN("sw.core", "content control inserted, but has no text attribute?");
        throw uno::RuntimeException(
            u"SwXContentControl::AttachImpl(): cannot create content control"_ustr, getXWeak());
    }

    m_pImpl->EndListeningAll();
    m_pImpl->m_pContentControl = pContentControl.get();
    m_pImpl->StartListening(pContentControl->GetNotifier());
    pContentControl->SetXContentControl(this);

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
        throw uno::RuntimeException(u"SwXContentControl::getAnchor(): not inserted"_ustr,
                                    getXWeak());
    }

    SwTextNode* pTextNode;
    sal_Int32 nContentControlStart;
    sal_Int32 nContentControlEnd;
    bool bSuccess = SetContentRange(pTextNode, nContentControlStart, nContentControlEnd);
    if (!bSuccess)
    {
        SAL_WARN("sw.core", "no pam");
        throw lang::DisposedException(u"SwXContentControl::getAnchor(): not attached"_ustr,
                                      getXWeak());
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

// XPropertySet
uno::Reference<beans::XPropertySetInfo> SAL_CALL SwXContentControl::getPropertySetInfo()
{
    SolarMutexGuard aGuard;

    static uno::Reference<beans::XPropertySetInfo> xRet
        = aSwMapProvider.GetPropertySet(PROPERTY_MAP_CONTENTCONTROL)->getPropertySetInfo();
    return xRet;
}

void SAL_CALL SwXContentControl::setPropertyValue(const OUString& rPropertyName,
                                                  const css::uno::Any& rValue)
{
    SolarMutexGuard aGuard;

    if (rPropertyName == UNO_NAME_SHOWING_PLACE_HOLDER)
    {
        bool bValue;
        if (rValue >>= bValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_bShowingPlaceHolder = bValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetShowingPlaceHolder(bValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_CHECKBOX)
    {
        bool bValue;
        if (rValue >>= bValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_bCheckbox = bValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetCheckbox(bValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_CHECKED)
    {
        bool bValue;
        if (rValue >>= bValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_bChecked = bValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetChecked(bValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_CHECKED_STATE)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aCheckedState = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetCheckedState(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_UNCHECKED_STATE)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aUncheckedState = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetUncheckedState(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_LIST_ITEMS)
    {
        std::vector<SwContentControlListItem> aItems
            = SwContentControlListItem::ItemsFromAny(rValue);
        if (m_pImpl->m_bIsDescriptor)
        {
            m_pImpl->m_aListItems = aItems;

            if (!m_pImpl->m_bComboBox && !m_pImpl->m_bDropDown)
            {
                m_pImpl->m_bDropDown = true;
            }
        }
        else
        {
            m_pImpl->m_pContentControl->SetListItems(aItems);

            if (!m_pImpl->m_pContentControl->GetComboBox()
                && !m_pImpl->m_pContentControl->GetDropDown())
            {
                m_pImpl->m_pContentControl->SetDropDown(true);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_PICTURE)
    {
        bool bValue;
        if (rValue >>= bValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_bPicture = bValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetPicture(bValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_DATE)
    {
        bool bValue;
        if (rValue >>= bValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_bDate = bValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetDate(bValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_DATE_FORMAT)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aDateFormat = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetDateFormat(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_DATE_LANGUAGE)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aDateLanguage = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetDateLanguage(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_CURRENT_DATE)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aCurrentDate = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetCurrentDate(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_PLAIN_TEXT)
    {
        bool bValue;
        if (rValue >>= bValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_bPlainText = bValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetPlainText(bValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_COMBO_BOX)
    {
        bool bValue;
        if (rValue >>= bValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_bComboBox = bValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetComboBox(bValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_DROP_DOWN)
    {
        bool bValue;
        if (rValue >>= bValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_bDropDown = bValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetDropDown(bValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_PLACEHOLDER_DOC_PART)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aPlaceholderDocPart = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetPlaceholderDocPart(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_DATA_BINDING_PREFIX_MAPPINGS)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aDataBindingPrefixMappings = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetDataBindingPrefixMappings(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_DATA_BINDING_XPATH)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aDataBindingXpath = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetDataBindingXpath(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_DATA_BINDING_STORE_ITEM_ID)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aDataBindingStoreItemID = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetDataBindingStoreItemID(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_COLOR)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aColor = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetColor(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_APPEARANCE)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aAppearance = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetAppearance(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_ALIAS)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aAlias = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetAlias(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_TAG)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aTag = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetTag(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_ID)
    {
        sal_Int32 nValue = 0;
        if (rValue >>= nValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_nId = nValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetId(nValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_TAB_INDEX)
    {
        sal_uInt32 nValue = 0;
        if (rValue >>= nValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_nTabIndex = nValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetTabIndex(nValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_LOCK)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aLock = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetLock(aValue);
            }
        }
    }
    else if (rPropertyName == UNO_NAME_MULTILINE)
    {
        OUString aValue;
        if (rValue >>= aValue)
        {
            if (m_pImpl->m_bIsDescriptor)
            {
                m_pImpl->m_aMultiLine = aValue;
            }
            else
            {
                m_pImpl->m_pContentControl->SetMultiLine(aValue);
            }
        }
    }
    else
    {
        throw beans::UnknownPropertyException();
    }
}

uno::Any SAL_CALL SwXContentControl::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    uno::Any aRet;
    if (rPropertyName == UNO_NAME_SHOWING_PLACE_HOLDER)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_bShowingPlaceHolder;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetShowingPlaceHolder();
        }
    }
    else if (rPropertyName == UNO_NAME_CHECKBOX)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_bCheckbox;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetCheckbox();
        }
    }
    else if (rPropertyName == UNO_NAME_CHECKED)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_bChecked;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetChecked();
        }
    }
    else if (rPropertyName == UNO_NAME_CHECKED_STATE)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aCheckedState;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetCheckedState();
        }
    }
    else if (rPropertyName == UNO_NAME_UNCHECKED_STATE)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aUncheckedState;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetUncheckedState();
        }
    }
    else if (rPropertyName == UNO_NAME_LIST_ITEMS)
    {
        std::vector<SwContentControlListItem> aItems;
        if (m_pImpl->m_bIsDescriptor)
        {
            aItems = m_pImpl->m_aListItems;
        }
        else
        {
            aItems = m_pImpl->m_pContentControl->GetListItems();
        }
        SwContentControlListItem::ItemsToAny(aItems, aRet);
    }
    else if (rPropertyName == UNO_NAME_PICTURE)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_bPicture;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetPicture();
        }
    }
    else if (rPropertyName == UNO_NAME_DATE)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_bDate;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetDate();
        }
    }
    else if (rPropertyName == UNO_NAME_DATE_FORMAT)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aDateFormat;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetDateFormat();
        }
    }
    else if (rPropertyName == UNO_NAME_DATE_LANGUAGE)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aDateLanguage;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetDateLanguage();
        }
    }
    else if (rPropertyName == UNO_NAME_CURRENT_DATE)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aCurrentDate;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetCurrentDate();
        }
    }
    else if (rPropertyName == UNO_NAME_PLAIN_TEXT)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_bPlainText;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetPlainText();
        }
    }
    else if (rPropertyName == UNO_NAME_COMBO_BOX)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_bComboBox;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetComboBox();
        }
    }
    else if (rPropertyName == UNO_NAME_DROP_DOWN)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_bDropDown;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetDropDown();
        }
    }
    else if (rPropertyName == UNO_NAME_PLACEHOLDER_DOC_PART)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aPlaceholderDocPart;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetCurrentDate();
        }
    }
    else if (rPropertyName == UNO_NAME_DATA_BINDING_PREFIX_MAPPINGS)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aDataBindingPrefixMappings;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetDataBindingPrefixMappings();
        }
    }
    else if (rPropertyName == UNO_NAME_DATA_BINDING_XPATH)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aDataBindingXpath;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetDataBindingXpath();
        }
    }
    else if (rPropertyName == UNO_NAME_DATA_BINDING_STORE_ITEM_ID)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aDataBindingStoreItemID;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetDataBindingStoreItemID();
        }
    }
    else if (rPropertyName == UNO_NAME_COLOR)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aColor;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetColor();
        }
    }
    else if (rPropertyName == UNO_NAME_APPEARANCE)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aAppearance;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetAppearance();
        }
    }
    else if (rPropertyName == UNO_NAME_ALIAS)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aAlias;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetAlias();
        }
    }
    else if (rPropertyName == UNO_NAME_TAG)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aTag;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetTag();
        }
    }
    else if (rPropertyName == UNO_NAME_DATE_STRING)
    {
        if (!m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_pContentControl->GetDateString();
        }
    }
    else if (rPropertyName == UNO_NAME_ID)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_nId;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetId();
        }
    }
    else if (rPropertyName == UNO_NAME_TAB_INDEX)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_nTabIndex;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetTabIndex();
        }
    }
    else if (rPropertyName == UNO_NAME_LOCK)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aLock;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetLock();
        }
    }
    else if (rPropertyName == UNO_NAME_MULTILINE)
    {
        if (m_pImpl->m_bIsDescriptor)
        {
            aRet <<= m_pImpl->m_aMultiLine;
        }
        else
        {
            aRet <<= m_pImpl->m_pContentControl->GetMultiLine();
        }
    }
    else
    {
        throw beans::UnknownPropertyException();
    }

    return aRet;
}

void SAL_CALL SwXContentControl::addPropertyChangeListener(
    const OUString& /*rPropertyName*/,
    const uno::Reference<beans::XPropertyChangeListener>& /*xListener*/)
{
    SAL_WARN("sw.uno", "SwXContentControl::addPropertyChangeListener: not implemented");
}

void SAL_CALL SwXContentControl::removePropertyChangeListener(
    const OUString& /*rPropertyName*/,
    const uno::Reference<beans::XPropertyChangeListener>& /*xListener*/)
{
    SAL_WARN("sw.uno", "SwXContentControl::removePropertyChangeListener: not implemented");
}

void SAL_CALL SwXContentControl::addVetoableChangeListener(
    const OUString& /*rPropertyName*/,
    const uno::Reference<beans::XVetoableChangeListener>& /*xListener*/)
{
    SAL_WARN("sw.uno", "SwXContentControl::addVetoableChangeListener: not implemented");
}

void SAL_CALL SwXContentControl::removeVetoableChangeListener(
    const OUString& /*rPropertyName*/,
    const uno::Reference<beans::XVetoableChangeListener>& /*xListener*/)
{
    SAL_WARN("sw.uno", "SwXContentControl::removeVetoableChangeListener: not implemented");
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
        throw uno::RuntimeException(u"createEnumeration(): not inserted"_ustr, getXWeak());
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

SwXContentControls::SwXContentControls(SwDoc* pDoc)
    : SwUnoCollection(pDoc)
{
}

SwXContentControls::~SwXContentControls() {}

sal_Int32 SwXContentControls::getCount()
{
    SolarMutexGuard aGuard;

    return GetDoc().GetContentControlManager().GetCount();
}

uno::Any SwXContentControls::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;

    SwContentControlManager& rManager = GetDoc().GetContentControlManager();
    if (nIndex < 0 || o3tl::make_unsigned(nIndex) >= rManager.GetCount())
    {
        throw lang::IndexOutOfBoundsException();
    }

    SwTextContentControl* pTextContentControl = rManager.Get(nIndex);
    const SwFormatContentControl& rFormatContentControl = pTextContentControl->GetContentControl();
    rtl::Reference<SwXContentControl> xContentControl
        = SwXContentControl::CreateXContentControl(*rFormatContentControl.GetContentControl());
    uno::Any aRet;
    aRet <<= uno::Reference<text::XTextContent>(xContentControl);
    return aRet;
}

uno::Type SwXContentControls::getElementType() { return cppu::UnoType<text::XTextContent>::get(); }

sal_Bool SwXContentControls::hasElements()
{
    SolarMutexGuard aGuard;

    return !GetDoc().GetContentControlManager().IsEmpty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
