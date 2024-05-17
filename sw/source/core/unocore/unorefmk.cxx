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
#include <utility>

#include <comphelper/interfacecontainer4.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <sal/config.h>
#include <svl/listener.hxx>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include <unotextrange.hxx>
#include <unorefmark.hxx>
#include <unotextcursor.hxx>
#include <unomap.hxx>
#include <unocrsrhelper.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <fmtrfmrk.hxx>
#include <txtrfmrk.hxx>
#include <unometa.hxx>
#include <unotext.hxx>
#include <unoport.hxx>
#include <txtatr.hxx>
#include <fmtmeta.hxx>
#include <docsh.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/URIs.hpp>
#include <com/sun/star/rdf/XLiteral.hpp>
#include <com/sun/star/rdf/XRepositorySupplier.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

using namespace ::com::sun::star;

class SwXReferenceMark::Impl
    : public SvtListener
{
public:
    unotools::WeakReference<SwXReferenceMark> m_wThis;
    std::mutex m_Mutex; // just for OInterfaceContainerHelper4
    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_EventListeners;
    bool m_bIsDescriptor;
    SwDoc* m_pDoc;
    const SwFormatRefMark* m_pMarkFormat;
    OUString m_sMarkName;

    Impl(SwDoc* const pDoc, SwFormatRefMark* const pRefMark)
        : m_bIsDescriptor(nullptr == pRefMark)
        , m_pDoc(pDoc)
        , m_pMarkFormat(pRefMark)
    {
        if (pRefMark)
        {
            StartListening(pRefMark->GetNotifier());
            m_sMarkName = pRefMark->GetRefName();
        }
    }

    bool IsValid() const { return m_pMarkFormat; }
    void InsertRefMark( SwPaM & rPam, SwXTextCursor const*const pCursor );
    void Invalidate();
protected:
    virtual void Notify(const SfxHint&) override;

};

void SwXReferenceMark::Impl::Invalidate()
{
    EndListeningAll();
    m_pDoc = nullptr;
    m_pMarkFormat = nullptr;
    uno::Reference<uno::XInterface> const xThis(m_wThis);
    if (!xThis.is())
    {   // fdo#72695: if UNO object is already dead, don't revive it with event
        return;
    }
    lang::EventObject const ev(xThis);
    std::unique_lock aGuard(m_Mutex);
    m_EventListeners.disposeAndClear(aGuard, ev);
}

void SwXReferenceMark::Impl::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
        Invalidate();
}

SwXReferenceMark::SwXReferenceMark(
        SwDoc *const pDoc, SwFormatRefMark *const pRefMark)
    : m_pImpl( new SwXReferenceMark::Impl(pDoc, pRefMark) )
{
}

SwXReferenceMark::~SwXReferenceMark()
{
}

rtl::Reference<SwXReferenceMark>
SwXReferenceMark::CreateXReferenceMark(
        SwDoc & rDoc, SwFormatRefMark *const pMarkFormat)
{
    // i#105557: do not iterate over the registered clients: race condition
    rtl::Reference<SwXReferenceMark> xMark;
    if (pMarkFormat)
    {
        xMark = pMarkFormat->GetXRefMark();
    }
    if (!xMark.is())
    {
        xMark = new SwXReferenceMark(&rDoc, pMarkFormat);
        if (pMarkFormat)
        {
            pMarkFormat->SetXRefMark(xMark);
        }
        // need a permanent Reference to initialize m_wThis
        xMark->m_pImpl->m_wThis = xMark.get();
    }
    return xMark;
}

OUString SAL_CALL SwXReferenceMark::getImplementationName()
{
    return u"SwXReferenceMark"_ustr;
}

sal_Bool SAL_CALL
SwXReferenceMark::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXReferenceMark::getSupportedServiceNames()
{
    return {
        u"com.sun.star.text.TextContent"_ustr,
        u"com.sun.star.text.ReferenceMark"_ustr
    };
}

namespace {

template<typename T> struct NotContainedIn
{
    std::vector<T> const& m_rVector;
    explicit NotContainedIn(std::vector<T> const& rVector)
        : m_rVector(rVector) { }
    bool operator() (T const& rT) {
        return std::find(m_rVector.begin(), m_rVector.end(), rT)
                    == m_rVector.end();
    }
};

}

void SwXReferenceMark::Impl::InsertRefMark(SwPaM& rPam,
        SwXTextCursor const*const pCursor)
{
    //! in some cases when this function is called the pDoc pointer member may have become
    //! invalid/deleted thus we obtain the document pointer from rPaM where it should always
    //! be valid.
    SwDoc& rDoc2 = rPam.GetDoc();

    UnoActionContext aCont(&rDoc2);
    SwFormatRefMark aRefMark(m_sMarkName);
    bool bMark = *rPam.GetPoint() != *rPam.GetMark();

    const bool bForceExpandHints( !bMark && pCursor && pCursor->IsAtEndOfMeta() );
    const SetAttrMode nInsertFlags = bForceExpandHints
        ?   ( SetAttrMode::FORCEHINTEXPAND
            | SetAttrMode::DONTEXPAND)
        : SetAttrMode::DONTEXPAND;

    std::vector<SwTextAttr *> oldMarks;
    if (bMark)
    {
        oldMarks = rPam.GetPointNode().GetTextNode()->GetTextAttrsAt(
            rPam.GetPoint()->GetContentIndex(), RES_TXTATR_REFMARK);
    }

    rDoc2.getIDocumentContentOperations().InsertPoolItem( rPam, aRefMark, nInsertFlags );

    if( bMark && *rPam.GetPoint() > *rPam.GetMark())
    {
        rPam.Exchange();
    }

    // aRefMark was copied into the document pool; now retrieve real format...
    SwTextAttr * pTextAttr(nullptr);
    if (bMark)
    {
        // #i107672#
        // ensure that we do not retrieve a different mark at the same position
        std::vector<SwTextAttr *> const newMarks(
            rPam.GetPointNode().GetTextNode()->GetTextAttrsAt(
                rPam.GetPoint()->GetContentIndex(), RES_TXTATR_REFMARK));
        std::vector<SwTextAttr *>::const_iterator const iter(
            std::find_if(newMarks.begin(), newMarks.end(),
                NotContainedIn<SwTextAttr *>(oldMarks)));
        assert(newMarks.end() != iter);
        if (newMarks.end() != iter)
        {
            pTextAttr = *iter;
        }
    }
    else
    {
        SwTextNode *pTextNd = rPam.GetPointNode().GetTextNode();
        assert(pTextNd);
        pTextAttr = pTextNd ? rPam.GetPointNode().GetTextNode()->GetTextAttrForCharAt(
                rPam.GetPoint()->GetContentIndex() - 1, RES_TXTATR_REFMARK) : nullptr;
    }

    if (!pTextAttr)
    {
        throw uno::RuntimeException(
            u"SwXReferenceMark::InsertRefMark(): cannot insert attribute"_ustr, nullptr);
    }

    m_pMarkFormat = &pTextAttr->GetRefMark();
    EndListeningAll();
    StartListening(const_cast<SwFormatRefMark*>(m_pMarkFormat)->GetNotifier());
}

void SAL_CALL
SwXReferenceMark::attach(const uno::Reference< text::XTextRange > & xTextRange)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }
    SwXTextRange* pRange = dynamic_cast<SwXTextRange*>(xTextRange.get());
    OTextCursorHelper* pCursor = dynamic_cast<OTextCursorHelper*>(xTextRange.get());
    SwDoc *const pDocument =
        pRange ? &pRange->GetDoc() : (pCursor ? pCursor->GetDoc() : nullptr);
    if (!pDocument)
    {
        throw lang::IllegalArgumentException();
    }

    SwUnoInternalPaM aPam(*pDocument);
    // this now needs to return TRUE
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);
    m_pImpl->InsertRefMark(aPam, dynamic_cast<SwXTextCursor*>(pCursor));
    m_pImpl->m_bIsDescriptor = false;
    m_pImpl->m_pDoc = pDocument;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXReferenceMark::getAnchor()
{
    SolarMutexGuard aGuard;

    if (m_pImpl->IsValid())
    {
        SwFormatRefMark const*const pNewMark =
            m_pImpl->m_pDoc->GetRefMark(m_pImpl->m_sMarkName);
        if (pNewMark && SfxPoolItem::areSame(pNewMark, m_pImpl->m_pMarkFormat))
        {
            SwTextRefMark const*const pTextMark =
                m_pImpl->m_pMarkFormat->GetTextRefMark();
            if (pTextMark &&
                (&pTextMark->GetTextNode().GetNodes() ==
                    &m_pImpl->m_pDoc->GetNodes()))
            {
                SwTextNode const& rTextNode = pTextMark->GetTextNode();
                std::optional<SwPaM> pPam;
                if ( pTextMark->End() )
                    pPam.emplace( rTextNode, *pTextMark->End(),
                                   rTextNode, pTextMark->GetStart());
                else
                    pPam.emplace( rTextNode, pTextMark->GetStart());

                return SwXTextRange::CreateXTextRange(
                            *m_pImpl->m_pDoc, *pPam->Start(), pPam->End());
            }
        }
    }
    return nullptr;
}

void SAL_CALL SwXReferenceMark::dispose()
{
    SolarMutexGuard aGuard;
    if (m_pImpl->IsValid())
    {
        SwFormatRefMark const*const pNewMark =
            m_pImpl->m_pDoc->GetRefMark(m_pImpl->m_sMarkName);
        if (pNewMark && SfxPoolItem::areSame(pNewMark, m_pImpl->m_pMarkFormat))
        {
            SwTextRefMark const*const pTextMark =
                m_pImpl->m_pMarkFormat->GetTextRefMark();
            if (pTextMark &&
                (&pTextMark->GetTextNode().GetNodes() ==
                    &m_pImpl->m_pDoc->GetNodes()))
            {
                SwTextNode const& rTextNode = pTextMark->GetTextNode();
                const sal_Int32 nStt = pTextMark->GetStart();
                const sal_Int32 nEnd = pTextMark->End()
                                  ? *pTextMark->End()
                                  : nStt + 1;

                SwPaM aPam( rTextNode, nStt, rTextNode, nEnd );
                m_pImpl->m_pDoc->getIDocumentContentOperations().DeleteAndJoin( aPam );
            }
        }
    }
    else if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->Invalidate();
    }
}

void SAL_CALL SwXReferenceMark::addEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    std::unique_lock aGuard(m_pImpl->m_Mutex);
    m_pImpl->m_EventListeners.addInterface(aGuard, xListener);
}

void SAL_CALL SwXReferenceMark::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    std::unique_lock aGuard(m_pImpl->m_Mutex);
    m_pImpl->m_EventListeners.removeInterface(aGuard, xListener);
}

OUString SAL_CALL SwXReferenceMark::getName()
{
    SolarMutexGuard aGuard;
    if (!m_pImpl->IsValid() ||
        !m_pImpl->m_pDoc->GetRefMark(m_pImpl->m_sMarkName))
    {
        throw uno::RuntimeException();
    }
    return m_pImpl->m_sMarkName;
}

void SAL_CALL SwXReferenceMark::setName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->m_sMarkName = rName;
    }
    else
    {
        if (!m_pImpl->IsValid()
            || !m_pImpl->m_pDoc->GetRefMark(m_pImpl->m_sMarkName)
            || m_pImpl->m_pDoc->GetRefMark(rName))
        {
            throw uno::RuntimeException();
        }
        SwFormatRefMark const*const pCurMark =
            m_pImpl->m_pDoc->GetRefMark(m_pImpl->m_sMarkName);
        if ((rName != m_pImpl->m_sMarkName)
            && pCurMark && SfxPoolItem::areSame(pCurMark, m_pImpl->m_pMarkFormat))
        {
            const UnoActionContext aCont(m_pImpl->m_pDoc);
            SwTextRefMark const*const pTextMark =
                m_pImpl->m_pMarkFormat->GetTextRefMark();
            if (pTextMark &&
                (&pTextMark->GetTextNode().GetNodes() ==
                     &m_pImpl->m_pDoc->GetNodes()))
            {
                SwTextNode const& rTextNode = pTextMark->GetTextNode();
                const sal_Int32 nStt = pTextMark->GetStart();
                const sal_Int32 nEnd = pTextMark->End()
                                        ? *pTextMark->End()
                                        : nStt + 1;

                SwPaM aPam( rTextNode, nStt, rTextNode, nEnd );
                // deletes the m_pImpl->m_pDoc member in the SwXReferenceMark!
                m_pImpl->m_pDoc->getIDocumentContentOperations().DeleteAndJoin( aPam );
                // The aPam will keep the correct and functional doc though

                m_pImpl->m_sMarkName = rName;
                //create a new one
                m_pImpl->InsertRefMark( aPam, nullptr );
                m_pImpl->m_pDoc = &aPam.GetDoc();
            }
        }
    }
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXReferenceMark::getPropertySetInfo()
{
    SolarMutexGuard g;

    static uno::Reference< beans::XPropertySetInfo >  xRef =
        aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARAGRAPH_EXTENSIONS)
            ->getPropertySetInfo();
    return xRef;
}

void SAL_CALL SwXReferenceMark::setPropertyValue(
    const OUString& /*rPropertyName*/, const uno::Any& /*rValue*/ )
{
    throw lang::IllegalArgumentException();
}

uno::Any SAL_CALL
SwXReferenceMark::getPropertyValue(const OUString& rPropertyName)
{
    // does not seem to need SolarMutex
    uno::Any aRet;
    if (! ::sw::GetDefaultTextContentValue(aRet, rPropertyName))
    {
        throw beans::UnknownPropertyException(rPropertyName);
    }
    return aRet;
}

void SAL_CALL SwXReferenceMark::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXReferenceMark::addPropertyChangeListener(): not implemented");
}

void SAL_CALL SwXReferenceMark::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXReferenceMark::removePropertyChangeListener(): not implemented");
}

void SAL_CALL SwXReferenceMark::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXReferenceMark::addVetoableChangeListener(): not implemented");
}

void SAL_CALL SwXReferenceMark::removeVetoableChangeListener(
    const OUString& /*rPropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXReferenceMark::removeVetoableChangeListener(): not implemented");
}

namespace {

class SwXMetaText : public cppu::OWeakObject, public SwXText
{
private:
    SwXMeta & m_rMeta;

    virtual void PrepareForAttach(uno::Reference< text::XTextRange > & xRange,
            const SwPaM & rPam) override;

    virtual bool CheckForOwnMemberMeta(const SwPaM & rPam, const bool bAbsorb) override;

protected:
    virtual const SwStartNode *GetStartNode() const override;

public:
    SwXMetaText(SwDoc & rDoc, SwXMeta & rMeta);

    /// make available for SwXMeta
    using SwXText::Invalidate;

    // XInterface
    virtual void SAL_CALL acquire() noexcept override { cppu::OWeakObject::acquire(); }
    virtual void SAL_CALL release() noexcept override { cppu::OWeakObject::release(); }

    // XTypeProvider
    virtual uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId() override;

    // XText
    virtual rtl::Reference< SwXTextCursor > createXTextCursor() override;
    virtual rtl::Reference< SwXTextCursor > createXTextCursorByRange(
            const ::css::uno::Reference< ::css::text::XTextRange >& aTextPosition ) override;
};

}

SwXMetaText::SwXMetaText(SwDoc & rDoc, SwXMeta & rMeta)
    : SwXText(&rDoc, CursorType::Meta)
    , m_rMeta(rMeta)
{
}

const SwStartNode *SwXMetaText::GetStartNode() const
{
    SwXText const * const pParent = m_rMeta.GetParentText().get();
    return pParent ? pParent->GetStartNode() : nullptr;
}

void SwXMetaText::PrepareForAttach( uno::Reference<text::XTextRange> & xRange,
        const SwPaM & rPam)
{
    // create a new cursor to prevent modifying SwXTextRange
    xRange = static_cast<text::XWordCursor*>(
        new SwXTextCursor(*GetDoc(), &m_rMeta, CursorType::Meta, *rPam.GetPoint(),
                (rPam.HasMark()) ? rPam.GetMark() : nullptr));
}

bool SwXMetaText::CheckForOwnMemberMeta(const SwPaM & rPam, const bool bAbsorb)
{
    return m_rMeta.CheckForOwnMemberMeta(rPam, bAbsorb);
}

rtl::Reference< SwXTextCursor > SwXMetaText::createXTextCursor()
{
    rtl::Reference< SwXTextCursor > xRet;
    if (IsValid())
    {
        SwTextNode * pTextNode;
        sal_Int32 nMetaStart;
        sal_Int32 nMetaEnd;
        const bool bSuccess(
                m_rMeta.SetContentRange(pTextNode, nMetaStart, nMetaEnd) );
        if (bSuccess)
        {
            SwPosition aPos(*pTextNode, nMetaStart);
            xRet = new SwXTextCursor(*GetDoc(), &m_rMeta, CursorType::Meta, aPos);
        }
    }
    return xRet;
}

uno::Sequence<sal_Int8> SAL_CALL
SwXMetaText::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XText

rtl::Reference< SwXTextCursor >
SwXMetaText::createXTextCursorByRange(
        const uno::Reference<text::XTextRange> & xTextPosition)
{
    const rtl::Reference< SwXTextCursor > xCursor( createXTextCursor() );
    xCursor->gotoRange(xTextPosition, false);
    return xCursor;
}

/**
 * the Meta has a cached list of text portions for its contents
 * this list is created by SwXTextPortionEnumeration
 * the Meta listens at the SwTextNode and throws away the cache when it changes
 *
 * This inner part of SwXMeta is deleted with a locked SolarMutex.
 */
class SwXMeta::Impl : public SvtListener
{
public:
    unotools::WeakReference<SwXMeta> m_wThis;
    std::mutex m_Mutex; // just for OInterfaceContainerHelper4
    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_EventListeners;
    std::unique_ptr<const TextRangeList_t> m_pTextPortions;
    // 3 possible states: not attached, attached, disposed
    bool m_bIsDisposed;
    bool m_bIsDescriptor;
    css::uno::Reference<SwXText> m_xParentText;
    rtl::Reference<SwXMetaText> m_xText;
    sw::Meta* m_pMeta;

    Impl(SwXMeta& rThis, SwDoc& rDoc,
            ::sw::Meta* const pMeta,
            css::uno::Reference<SwXText> xParentText,
            std::unique_ptr<TextRangeList_t const> pPortions)
        : m_pTextPortions(std::move(pPortions))
        , m_bIsDisposed(false)
        , m_bIsDescriptor(nullptr == pMeta)
        , m_xParentText(std::move(xParentText))
        , m_xText(new SwXMetaText(rDoc, rThis))
        , m_pMeta(pMeta)
    {
        !m_bIsDescriptor && StartListening(m_pMeta->GetNotifier());
    }

    inline const ::sw::Meta* GetMeta() const;
    // only for SwXMetaField!
    inline const ::sw::MetaField* GetMetaField() const;
protected:
    virtual void Notify(const SfxHint& rHint) override;

};

inline const ::sw::Meta* SwXMeta::Impl::GetMeta() const
{
    return m_pMeta;
}

// sw::BroadcastingModify
void SwXMeta::Impl::Notify(const SfxHint& rHint)
{
    m_pTextPortions.reset(); // throw away cache (SwTextNode changed)
    if(rHint.GetId() != SfxHintId::Dying && rHint.GetId() != SfxHintId::Deinitializing)
        return;

    m_bIsDisposed = true;
    m_pMeta = nullptr;
    m_xText->Invalidate();
    uno::Reference<uno::XInterface> const xThis(m_wThis);
    if (!xThis.is())
    {   // fdo#72695: if UNO object is already dead, don't revive it with event
        return;
    }
    lang::EventObject const ev(xThis);
    std::unique_lock aGuard(m_Mutex);
    m_EventListeners.disposeAndClear(aGuard, ev);
}

css::uno::Reference<SwXText> const & SwXMeta::GetParentText() const
{
    return m_pImpl->m_xParentText;
}

SwXMeta::SwXMeta(SwDoc *const pDoc, ::sw::Meta *const pMeta,
        css::uno::Reference<SwXText> const& xParentText,
        std::unique_ptr<TextRangeList_t const> pPortions)
    : m_pImpl( new SwXMeta::Impl(*this, *pDoc, pMeta, xParentText, std::move(pPortions)) )
{
}

SwXMeta::SwXMeta(SwDoc *const pDoc)
    : m_pImpl( new SwXMeta::Impl(*this, *pDoc, nullptr, nullptr, nullptr) )
{
}

SwXMeta::~SwXMeta()
{
}

rtl::Reference<SwXMeta>
SwXMeta::CreateXMeta(SwDoc & rDoc, bool const isField)
{
    // this is why the constructor is private: need to acquire pXMeta here
    rtl::Reference<SwXMeta> xMeta(isField
            ? new SwXMetaField(& rDoc) : new SwXMeta(& rDoc));
    // need a permanent Reference to initialize m_wThis
    xMeta->m_pImpl->m_wThis = xMeta.get();
    return xMeta;
}

rtl::Reference<SwXMeta>
SwXMeta::CreateXMeta(::sw::Meta & rMeta,
            css::uno::Reference<SwXText> i_xParent,
            std::unique_ptr<TextRangeList_t const> && pPortions)
{
    // re-use existing SwXMeta
    // #i105557#: do not iterate over the registered clients: race condition
    rtl::Reference<SwXMeta> xMeta(rMeta.GetXMeta());
    if (xMeta.is())
    {
        if (pPortions) // set cache in the XMeta to the given portions
        {
            // NB: the meta must always be created with the complete content
            // if SwXTextPortionEnumeration is created for a selection,
            // it must be checked that the Meta is contained in the selection!
            xMeta->m_pImpl->m_pTextPortions = std::move(pPortions);
            // ??? is this necessary?
            if (xMeta->m_pImpl->m_xParentText.get() != i_xParent.get())
            {
                SAL_WARN("sw.uno", "SwXMeta with different parent?");
                xMeta->m_pImpl->m_xParentText = i_xParent;
            }
        }
        return xMeta;
    }

    // create new SwXMeta
    SwTextNode * const pTextNode( rMeta.GetTextNode() );
    SAL_WARN_IF(!pTextNode, "sw.uno", "CreateXMeta: no text node?");
    if (!pTextNode) { return nullptr; }
    css::uno::Reference<SwXText> xParentText(i_xParent);
    if (!xParentText.is())
    {
        SwTextMeta * const pTextAttr( rMeta.GetTextAttr() );
        SAL_WARN_IF(!pTextAttr, "sw.uno", "CreateXMeta: no text attr?");
        if (!pTextAttr) { return nullptr; }
        const SwPosition aPos(*pTextNode, pTextAttr->GetStart());
        xParentText = ::sw::CreateParentXText(pTextNode->GetDoc(), aPos);
    }
    if (!xParentText.is()) { return nullptr; }
    // this is why the constructor is private: need to acquire pXMeta here
    xMeta = (RES_TXTATR_META == rMeta.GetFormatMeta()->Which())
        ? new SwXMeta     (&pTextNode->GetDoc(), &rMeta, xParentText,
                            std::move(pPortions))
        : new SwXMetaField(&pTextNode->GetDoc(), &rMeta, xParentText,
                            std::move(pPortions));
    // in order to initialize the weak pointer cache in the core object
    rMeta.SetXMeta(xMeta);
    // need a permanent Reference to initialize m_wThis
    xMeta->m_pImpl->m_wThis = xMeta.get();
    return xMeta;
}

bool SwXMeta::SetContentRange(
        SwTextNode *& rpNode, sal_Int32 & rStart, sal_Int32 & rEnd ) const
{
    ::sw::Meta const * const pMeta( m_pImpl->GetMeta() );
    if (pMeta)
    {
        SwTextMeta const * const pTextAttr( pMeta->GetTextAttr() );
        if (pTextAttr)
        {
            rpNode = pMeta->GetTextNode();
            if (rpNode)
            {
                // rStart points at the first position _within_ the meta!
                rStart = pTextAttr->GetStart() + 1;
                rEnd = *pTextAttr->End();
                return true;
            }
        }
    }
    return false;
}

bool SwXMeta::CheckForOwnMemberMeta(const SwPaM & rPam, const bool bAbsorb)
{
    SwTextNode * pTextNode;
    sal_Int32 nMetaStart;
    sal_Int32 nMetaEnd;
    const bool bSuccess( SetContentRange(pTextNode, nMetaStart, nMetaEnd) );
    OSL_ENSURE(bSuccess, "no pam?");
    if (!bSuccess)
        throw lang::DisposedException();

    SwPosition const * const pStartPos( rPam.Start() );
    if (&pStartPos->GetNode() != pTextNode)
    {
        throw lang::IllegalArgumentException(
            u"trying to insert into a nesting text content, but start "
                "of text range not in same paragraph as text content"_ustr,
                nullptr, 0);
    }
    bool bForceExpandHints(false);
    const sal_Int32 nStartPos(pStartPos->GetContentIndex());
    // not <= but < because nMetaStart is behind dummy char!
    // not >= but > because == means insert at end!
    if ((nStartPos < nMetaStart) || (nStartPos > nMetaEnd))
    {
        throw lang::IllegalArgumentException(
            u"trying to insert into a nesting text content, but start "
                "of text range not inside text content"_ustr,
                nullptr, 0);
    }
    else if (nStartPos == nMetaEnd)
    {
        bForceExpandHints = true;
    }
    if (rPam.HasMark() && bAbsorb)
    {
        SwPosition const * const pEndPos( rPam.End() );
        if (&pEndPos->GetNode() != pTextNode)
        {
            throw lang::IllegalArgumentException(
                u"trying to insert into a nesting text content, but end "
                    "of text range not in same paragraph as text content"_ustr,
                    nullptr, 0);
        }
        const sal_Int32 nEndPos(pEndPos->GetContentIndex());
        // not <= but < because nMetaStart is behind dummy char!
        // not >= but > because == means insert at end!
        if ((nEndPos < nMetaStart) || (nEndPos > nMetaEnd))
        {
            throw lang::IllegalArgumentException(
                u"trying to insert into a nesting text content, but end "
                    "of text range not inside text content"_ustr,
                    nullptr, 0);
        }
        else if (nEndPos == nMetaEnd)
        {
            bForceExpandHints = true;
        }
    }
    return bForceExpandHints;
}

// XServiceInfo
OUString SAL_CALL
SwXMeta::getImplementationName()
{
    return u"SwXMeta"_ustr;
}

sal_Bool SAL_CALL
SwXMeta::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXMeta::getSupportedServiceNames()
{
    return {
        u"com.sun.star.text.TextContent"_ustr,
        u"com.sun.star.text.InContentMetadata"_ustr
    };
}

// XComponent
void SAL_CALL
SwXMeta::addEventListener(
        uno::Reference< lang::XEventListener> const & xListener )
{
    // no need to lock here as m_pImpl is const and container threadsafe
    std::unique_lock aGuard(m_pImpl->m_Mutex);
    m_pImpl->m_EventListeners.addInterface(aGuard, xListener);
}

void SAL_CALL
SwXMeta::removeEventListener(
        uno::Reference< lang::XEventListener> const & xListener )
{
    // no need to lock here as m_pImpl is const and container threadsafe
    std::unique_lock aGuard(m_pImpl->m_Mutex);
    m_pImpl->m_EventListeners.removeInterface(aGuard, xListener);
}

void SAL_CALL
SwXMeta::dispose()
{
    SolarMutexGuard g;

    if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->m_pTextPortions.reset();
        lang::EventObject const ev(getXWeak());
        std::unique_lock aGuard(m_pImpl->m_Mutex);
        m_pImpl->m_EventListeners.disposeAndClear(aGuard, ev);
        m_pImpl->m_bIsDisposed = true;
        m_pImpl->m_xText->Invalidate();
    }
    else if (!m_pImpl->m_bIsDisposed)
    {
        SwTextNode * pTextNode;
        sal_Int32 nMetaStart;
        sal_Int32 nMetaEnd;
        const bool bSuccess(SetContentRange(pTextNode, nMetaStart, nMetaEnd));
        OSL_ENSURE(bSuccess, "no pam?");
        if (bSuccess)
        {
            // -1 because of CH_TXTATR
            SwPaM aPam( *pTextNode, nMetaStart - 1, *pTextNode, nMetaEnd );
            SwDoc& rDoc( pTextNode->GetDoc() );
            rDoc.getIDocumentContentOperations().DeleteAndJoin( aPam );

            // removal should call Modify and do the dispose
            assert(m_pImpl->m_bIsDisposed);
        }
    }
}

void
SwXMeta::AttachImpl(const uno::Reference< text::XTextRange > & i_xTextRange,
        const sal_uInt16 i_nWhich)
{
    SolarMutexGuard g;

    if (m_pImpl->m_bIsDisposed)
    {
        throw lang::DisposedException();
    }
    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException(
            u"SwXMeta::attach(): already attached"_ustr,
            getXWeak());
    }

    SwXTextRange *const pRange(dynamic_cast<SwXTextRange*>(i_xTextRange.get()));
    OTextCursorHelper *const pCursor(dynamic_cast<OTextCursorHelper*>(i_xTextRange.get()));
    if (!pRange && !pCursor)
    {
        throw lang::IllegalArgumentException(
            u"SwXMeta::attach(): argument not supported type"_ustr,
            getXWeak(), 0);
    }

    SwDoc * const pDoc(
            pRange ? &pRange->GetDoc() : pCursor->GetDoc());
    if (!pDoc)
    {
        throw lang::IllegalArgumentException(
            u"SwXMeta::attach(): argument has no SwDoc"_ustr,
            getXWeak(), 0);
    }

    SwUnoInternalPaM aPam(*pDoc);
    ::sw::XTextRangeToSwPaM(aPam, i_xTextRange);

    UnoActionContext aContext(pDoc);

    SwXTextCursor const*const pTextCursor(
            dynamic_cast<SwXTextCursor*>(pCursor));
    const bool bForceExpandHints(pTextCursor && pTextCursor->IsAtEndOfMeta());
    const SetAttrMode nInsertFlags( bForceExpandHints
        ?   ( SetAttrMode::FORCEHINTEXPAND
            | SetAttrMode::DONTEXPAND)
        : SetAttrMode::DONTEXPAND );

    const std::shared_ptr< ::sw::Meta> pMeta( (RES_TXTATR_META == i_nWhich)
        ? std::make_shared< ::sw::Meta>( nullptr )
        : std::shared_ptr< ::sw::Meta>(
            pDoc->GetMetaFieldManager().makeMetaField()) );
    SwFormatMeta meta(pMeta, i_nWhich); // this is cloned by Insert!
    const bool bSuccess( pDoc->getIDocumentContentOperations().InsertPoolItem( aPam, meta, nInsertFlags ) );
    SwTextAttr * const pTextAttr( pMeta->GetTextAttr() );
    if (!bSuccess)
    {
        throw lang::IllegalArgumentException(
            u"SwXMeta::attach(): cannot create meta: range invalid?"_ustr,
            getXWeak(), 1);
    }
    if (!pTextAttr)
    {
        OSL_FAIL("meta inserted, but has no text attribute?");
        throw uno::RuntimeException(
            u"SwXMeta::attach(): cannot create meta"_ustr,
            getXWeak());
    }

    m_pImpl->EndListeningAll();
    m_pImpl->m_pMeta = pMeta.get();
    m_pImpl->StartListening(pMeta->GetNotifier());
    pMeta->SetXMeta(this);

    m_pImpl->m_xParentText = ::sw::CreateParentXText(*pDoc, *aPam.GetPoint());

    m_pImpl->m_bIsDescriptor = false;
}

// XTextContent
void SAL_CALL
SwXMeta::attach(const uno::Reference< text::XTextRange > & i_xTextRange)
{
    return SwXMeta::AttachImpl(i_xTextRange, RES_TXTATR_META);
}

uno::Reference< text::XTextRange > SAL_CALL
SwXMeta::getAnchor()
{
    SolarMutexGuard g;

    if (m_pImpl->m_bIsDisposed)
    {
        throw lang::DisposedException();
    }
    if (m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException(
                u"SwXMeta::getAnchor(): not inserted"_ustr,
                getXWeak());
    }

    SwTextNode * pTextNode;
    sal_Int32 nMetaStart;
    sal_Int32 nMetaEnd;
    const bool bSuccess(SetContentRange(pTextNode, nMetaStart, nMetaEnd));
    OSL_ENSURE(bSuccess, "no pam?");
    if (!bSuccess)
    {
        throw lang::DisposedException(
                u"SwXMeta::getAnchor(): not attached"_ustr,
                getXWeak());
    }

    const SwPosition start(*pTextNode, nMetaStart - 1); // -1 due to CH_TXTATR
    const SwPosition end(*pTextNode, nMetaEnd);
    return SwXTextRange::CreateXTextRange(pTextNode->GetDoc(), start, &end);
}

// XTextRange
uno::Reference< text::XText > SAL_CALL
SwXMeta::getText()
{
    return this;
}

uno::Reference< text::XTextRange > SAL_CALL
SwXMeta::getStart()
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->getStart();
}

uno::Reference< text::XTextRange > SAL_CALL
SwXMeta::getEnd()
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->getEnd();
}

OUString SAL_CALL
SwXMeta::getString()
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->getString();
}

void SAL_CALL
SwXMeta::setString(const OUString& rString)
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->setString(rString);
}

// XSimpleText
uno::Reference< text::XTextCursor > SAL_CALL
SwXMeta::createTextCursor()
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->createTextCursor();
}

uno::Reference< text::XTextCursor > SAL_CALL
SwXMeta::createTextCursorByRange(
        const uno::Reference<text::XTextRange> & xTextPosition)
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->createTextCursorByRange(xTextPosition);
}

void SAL_CALL
SwXMeta::insertString(const uno::Reference<text::XTextRange> & xRange,
        const OUString& rString, sal_Bool bAbsorb)
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->insertString(xRange, rString, bAbsorb);
}

void SAL_CALL
SwXMeta::insertControlCharacter(const uno::Reference<text::XTextRange> & xRange,
        sal_Int16 nControlCharacter, sal_Bool bAbsorb)
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->insertControlCharacter(xRange, nControlCharacter,
                bAbsorb);
}

// XText
void SAL_CALL
SwXMeta::insertTextContent( const uno::Reference<text::XTextRange> & xRange,
        const uno::Reference<text::XTextContent> & xContent, sal_Bool bAbsorb)
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->insertTextContent(xRange, xContent, bAbsorb);
}

void SAL_CALL
SwXMeta::removeTextContent(
        const uno::Reference< text::XTextContent > & xContent)
{
    SolarMutexGuard g;
    return m_pImpl->m_xText->removeTextContent(xContent);
}

// XChild
uno::Reference< uno::XInterface > SAL_CALL
SwXMeta::getParent()
{
    SolarMutexGuard g;
    SwTextNode * pTextNode;
    sal_Int32 nMetaStart;
    sal_Int32 nMetaEnd;
    bool const bSuccess( SetContentRange(pTextNode, nMetaStart, nMetaEnd) );
    OSL_ENSURE(bSuccess, "no pam?");
    if (!bSuccess) { throw lang::DisposedException(); }
    // in order to prevent getting this meta, subtract 1 from nMetaStart;
    // so we get the index of the dummy character, and we exclude it
    // by calling GetTextAttrAt(_, _, PARENT) in GetNestedTextContent
    uno::Reference<text::XTextContent> const xRet(
        SwUnoCursorHelper::GetNestedTextContent(*pTextNode, nMetaStart - 1,
            true) );
    return xRet;
}

void SAL_CALL
SwXMeta::setParent(uno::Reference< uno::XInterface > const& /*xParent*/)
{
    throw lang::NoSupportException(u"setting parent not supported"_ustr, *this);
}

// XElementAccess
uno::Type SAL_CALL
SwXMeta::getElementType()
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL SwXMeta::hasElements()
{
    SolarMutexGuard g;
    return m_pImpl->m_pMeta != nullptr;
}

// XEnumerationAccess
uno::Reference< container::XEnumeration > SAL_CALL
SwXMeta::createEnumeration()
{
    SolarMutexGuard g;

    if (m_pImpl->m_bIsDisposed)
    {
        throw lang::DisposedException();
    }
    if (m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException(
                u"createEnumeration(): not inserted"_ustr,
                getXWeak());
    }

    SwTextNode * pTextNode;
    sal_Int32 nMetaStart;
    sal_Int32 nMetaEnd;
    const bool bSuccess(SetContentRange(pTextNode, nMetaStart, nMetaEnd));
    OSL_ENSURE(bSuccess, "no pam?");
    if (!bSuccess)
        throw lang::DisposedException();

    SwPaM aPam(*pTextNode, nMetaStart);

    if (!m_pImpl->m_pTextPortions)
    {
        return new SwXTextPortionEnumeration(
                    aPam, GetParentText(), nMetaStart, nMetaEnd);
    }
    else // cached!
    {
        return new SwXTextPortionEnumeration(aPam, std::deque(*m_pImpl->m_pTextPortions));
    }
}

// MetadatableMixin
::sfx2::Metadatable* SwXMeta::GetCoreObject()
{
    return const_cast< ::sw::Meta * >(m_pImpl->GetMeta());
}

uno::Reference<frame::XModel> SwXMeta::GetModel()
{
    ::sw::Meta const * const pMeta( m_pImpl->GetMeta() );
    if (pMeta)
    {
        SwTextNode const * const pTextNode( pMeta->GetTextNode() );
        if (pTextNode)
        {
            SwDocShell const * const pShell(pTextNode->GetDoc().GetDocShell());
            return pShell ? pShell->GetModel() : nullptr;
        }
    }
    return nullptr;
}

inline const ::sw::MetaField* SwXMeta::Impl::GetMetaField() const
{
    return dynamic_cast<sw::MetaField*>(m_pMeta);
}

SwXMetaField::SwXMetaField(SwDoc *const pDoc, ::sw::Meta *const pMeta,
        css::uno::Reference<SwXText> const& xParentText,
        std::unique_ptr<TextRangeList_t const> pPortions)
    : SwXMetaField_Base(pDoc, pMeta, xParentText, std::move(pPortions))
{
    assert(dynamic_cast< ::sw::MetaField* >(pMeta) && "SwXMetaField created for wrong hint!");
}

SwXMetaField::SwXMetaField(SwDoc *const pDoc)
    :  SwXMetaField_Base(pDoc)
{
}

SwXMetaField::~SwXMetaField()
{
}

// XServiceInfo
OUString SAL_CALL
SwXMetaField::getImplementationName()
{
    return u"SwXMetaField"_ustr;
}

sal_Bool SAL_CALL
SwXMetaField::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXMetaField::getSupportedServiceNames()
{
    return {
        u"com.sun.star.text.TextContent"_ustr,
        u"com.sun.star.text.TextField"_ustr,
        u"com.sun.star.text.textfield.MetadataField"_ustr
    };
}

// XComponent
void SAL_CALL
SwXMetaField::addEventListener(
        uno::Reference< lang::XEventListener> const & xListener )
{
    return SwXMeta::addEventListener(xListener);
}

void SAL_CALL
SwXMetaField::removeEventListener(
        uno::Reference< lang::XEventListener> const & xListener )
{
    return SwXMeta::removeEventListener(xListener);
}

void SAL_CALL
SwXMetaField::dispose()
{
    return SwXMeta::dispose();
}

// XTextContent
void SAL_CALL
SwXMetaField::attach(const uno::Reference< text::XTextRange > & i_xTextRange)
{
    return SwXMeta::AttachImpl(i_xTextRange, RES_TXTATR_METAFIELD);
}

uno::Reference< text::XTextRange > SAL_CALL
SwXMetaField::getAnchor()
{
    return SwXMeta::getAnchor();
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXMetaField::getPropertySetInfo()
{
    SolarMutexGuard g;

    static uno::Reference< beans::XPropertySetInfo > xRef(
        aSwMapProvider.GetPropertySet(PROPERTY_MAP_METAFIELD)
            ->getPropertySetInfo() );
    return xRef;
}

void SAL_CALL
SwXMetaField::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
{
    SolarMutexGuard g;

    ::sw::MetaField * const pMeta(
            const_cast< ::sw::MetaField * >(m_pImpl->GetMetaField()) );
    if (!pMeta)
        throw lang::DisposedException();

    if ( rPropertyName == "NumberFormat" )
    {
        sal_Int32 nNumberFormat(0);
        if (rValue >>= nNumberFormat)
        {
            pMeta->SetNumberFormat(static_cast<sal_uInt32>(nNumberFormat));
        }
    }
    else if ( rPropertyName == "IsFixedLanguage" )
    {
        bool b(false);
        if (rValue >>= b)
        {
            pMeta->SetIsFixedLanguage(b);
        }
    }
    else
    {
        throw beans::UnknownPropertyException(rPropertyName);
    }
}

uno::Any SAL_CALL
SwXMetaField::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard g;

    ::sw::MetaField const * const pMeta( m_pImpl->GetMetaField() );
    if (!pMeta)
        throw lang::DisposedException();

    uno::Any any;

    if ( rPropertyName == "NumberFormat" )
    {
        const OUString text( getPresentation(false) );
        any <<= static_cast<sal_Int32>(pMeta->GetNumberFormat(text));
    }
    else if ( rPropertyName == "IsFixedLanguage" )
    {
        any <<= pMeta->IsFixedLanguage();
    }
    else
    {
        throw beans::UnknownPropertyException(rPropertyName);
    }

    return any;
}

void SAL_CALL
SwXMetaField::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXMetaField::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXMetaField::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXMetaField::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXMetaField::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXMetaField::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXMetaField::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXMetaField::removeVetoableChangeListener(): not implemented");
}

static uno::Reference<rdf::XURI> const&
lcl_getURI(const sal_Int16 eKnown)
{
    static uno::Reference< uno::XComponentContext > xContext(
        ::comphelper::getProcessComponentContext());
    static uno::Reference< rdf::XURI > xOdfPrefix(
        rdf::URI::createKnown(xContext, rdf::URIs::ODF_PREFIX),
        uno::UNO_SET_THROW);
    static uno::Reference< rdf::XURI > xOdfSuffix(
        rdf::URI::createKnown(xContext, rdf::URIs::ODF_SUFFIX),
        uno::UNO_SET_THROW);
    static uno::Reference< rdf::XURI > xOdfShading(
        rdf::URI::createKnown(xContext, rdf::URIs::LO_EXT_SHADING),
        uno::UNO_SET_THROW);
    switch (eKnown)
    {
        case rdf::URIs::ODF_PREFIX:
            return xOdfPrefix;
        case rdf::URIs::ODF_SUFFIX:
            return xOdfSuffix;
        default:
            return xOdfShading;
    }
}

static OUString
lcl_getPrefixOrSuffix(
    uno::Reference<rdf::XRepository> const & xRepository,
    uno::Reference<rdf::XResource> const & xMetaField,
    uno::Reference<rdf::XURI> const & xPredicate)
{
    const uno::Reference<container::XEnumeration> xEnum(
        xRepository->getStatements(xMetaField, xPredicate, nullptr),
        uno::UNO_SET_THROW);
    while (xEnum->hasMoreElements()) {
        rdf::Statement stmt;
        if (!(xEnum->nextElement() >>= stmt)) {
            throw uno::RuntimeException();
        }
        const uno::Reference<rdf::XLiteral> xObject(stmt.Object,
            uno::UNO_QUERY);
        if (!xObject.is()) continue;
        if (xEnum->hasMoreElements()) {
            SAL_INFO("sw.uno", "ignoring other odf:Prefix/odf:Suffix statements");
        }
        return xObject->getValue();
    }
    return OUString();
}

void
getPrefixAndSuffix(
        const uno::Reference<frame::XModel>& xModel,
        const uno::Reference<rdf::XMetadatable>& xMetaField,
        OUString *const o_pPrefix, OUString *const o_pSuffix, OUString *const o_pShadingColor)
{
    try {
        const uno::Reference<rdf::XRepositorySupplier> xRS(
                xModel, uno::UNO_QUERY_THROW);
        const uno::Reference<rdf::XRepository> xRepo(
                xRS->getRDFRepository(), uno::UNO_SET_THROW);
        const uno::Reference<rdf::XResource> xMeta(
                xMetaField, uno::UNO_QUERY_THROW);
        if (o_pPrefix)
        {
            *o_pPrefix = lcl_getPrefixOrSuffix(xRepo, xMeta, lcl_getURI(rdf::URIs::ODF_PREFIX));
        }
        if (o_pSuffix)
        {
            *o_pSuffix = lcl_getPrefixOrSuffix(xRepo, xMeta, lcl_getURI(rdf::URIs::ODF_SUFFIX));
        }
        if (o_pShadingColor)
        {
            *o_pShadingColor = lcl_getPrefixOrSuffix(xRepo, xMeta, lcl_getURI(rdf::URIs::LO_EXT_SHADING));
        }
    } catch (uno::RuntimeException &) {
        throw;
    } catch (const uno::Exception &) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(u"getPrefixAndSuffix: exception"_ustr, nullptr, anyEx);
    }
}

// XTextField
OUString SAL_CALL
SwXMetaField::getPresentation(sal_Bool bShowCommand)
{
    SolarMutexGuard g;

    if (bShowCommand)
    {
//FIXME ?
        return OUString();
    }
    else
    {
        // getString should check if this is invalid
        const OUString content( getString() );
        OUString prefix;
        OUString suffix;
        getPrefixAndSuffix(GetModel(), this, &prefix, &suffix, nullptr);
        return prefix + content + suffix;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
