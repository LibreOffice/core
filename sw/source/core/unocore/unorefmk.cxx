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

#include <utility>

#include <osl/mutex.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>

#include <unomid.h>
#include <unotextrange.hxx>
#include <unorefmark.hxx>
#include <unotextcursor.hxx>
#include <unomap.hxx>
#include <unocrsr.hxx>
#include <unocrsrhelper.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <fmtrfmrk.hxx>
#include <txtrfmrk.hxx>
#include <hints.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/URIs.hpp>
#include <com/sun/star/rdf/XLiteral.hpp>
#include <com/sun/star/rdf/XRepositorySupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <unometa.hxx>
#include <unotext.hxx>
#include <unoport.hxx>
#include <txtatr.hxx>
#include <fmtmeta.hxx>
#include <docsh.hxx>
#include <cppuhelper/weak.hxx>


using namespace ::com::sun::star;

class SwXReferenceMark::Impl
    : public SwClient
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper2

public:
    uno::WeakReference<uno::XInterface> m_wThis;
    ::comphelper::OInterfaceContainerHelper2 m_EventListeners;
    bool                        m_bIsDescriptor;
    SwDoc *                     m_pDoc;
    const SwFormatRefMark *        m_pMarkFormat;
    OUString             m_sMarkName;

    Impl(   SwDoc *const pDoc, SwFormatRefMark *const pRefMark)
        : SwClient(pRefMark)
        , m_EventListeners(m_Mutex)
        , m_bIsDescriptor(nullptr == pRefMark)
        , m_pDoc(pDoc)
        , m_pMarkFormat(pRefMark)
    {
        if (pRefMark)
        {
            m_sMarkName = pRefMark->GetRefName();
        }
    }

    bool    IsValid() const { return nullptr != GetRegisteredIn(); }
    void    InsertRefMark( SwPaM & rPam, SwXTextCursor const*const pCursor );
    void    Invalidate();
protected:
    // SwClient
    virtual void    Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

};

void SwXReferenceMark::Impl::Invalidate()
{
    if (IsValid())
    {
        GetRegisteredIn()->Remove(this);
    }
    m_pDoc = nullptr;
    m_pMarkFormat = nullptr;
    uno::Reference<uno::XInterface> const xThis(m_wThis);
    if (!xThis.is())
    {   // fdo#72695: if UNO object is already dead, don't revive it with event
        return;
    }
    lang::EventObject const ev(xThis);
    m_EventListeners.disposeAndClear(ev);
}

void SwXReferenceMark::Impl::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);

    if (!GetRegisteredIn()) // removed => dispose
    {
        Invalidate();
    }
}

SwXReferenceMark::SwXReferenceMark(
        SwDoc *const pDoc, SwFormatRefMark *const pRefMark)
    : m_pImpl( new SwXReferenceMark::Impl(pDoc, pRefMark) )
{
}

SwXReferenceMark::~SwXReferenceMark()
{
}

uno::Reference<text::XTextContent>
SwXReferenceMark::CreateXReferenceMark(
        SwDoc & rDoc, SwFormatRefMark *const pMarkFormat)
{
    // i#105557: do not iterate over the registered clients: race condition
    uno::Reference<text::XTextContent> xMark;
    if (pMarkFormat)
    {
        xMark = pMarkFormat->GetXRefMark();
    }
    if (!xMark.is())
    {
        SwXReferenceMark *const pMark(new SwXReferenceMark(&rDoc, pMarkFormat));
        xMark.set(pMark);
        if (pMarkFormat)
        {
            pMarkFormat->SetXRefMark(xMark);
        }
        // need a permanent Reference to initialize m_wThis
        pMark->m_pImpl->m_wThis = xMark;
    }
    return xMark;
}

namespace
{
    class theSwXReferenceMarkUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXReferenceMarkUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXReferenceMark::getUnoTunnelId()
{
    return theSwXReferenceMarkUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL
SwXReferenceMark::getSomething(const uno::Sequence< sal_Int8 >& rId)
{
    return ::sw::UnoTunnelImpl<SwXReferenceMark>(rId, this);
}

OUString SAL_CALL SwXReferenceMark::getImplementationName()
{
    return OUString("SwXReferenceMark");
}

static char const*const g_ServicesReferenceMark[] =
{
    "com.sun.star.text.TextContent",
    "com.sun.star.text.ReferenceMark",
};

static const size_t g_nServicesReferenceMark(SAL_N_ELEMENTS(g_ServicesReferenceMark));

sal_Bool SAL_CALL
SwXReferenceMark::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXReferenceMark::getSupportedServiceNames()
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesReferenceMark, g_ServicesReferenceMark);
}

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

void SwXReferenceMark::Impl::InsertRefMark(SwPaM& rPam,
        SwXTextCursor const*const pCursor)
{
    //! in some cases when this function is called the pDoc pointer member may have become
    //! invalid/deleted thus we obtain the document pointer from rPaM where it should always
    //! be valid.
    SwDoc *pDoc2 = rPam.GetDoc();

    UnoActionContext aCont(pDoc2);
    SwFormatRefMark aRefMark(m_sMarkName);
    bool bMark = *rPam.GetPoint() != *rPam.GetMark();

    const bool bForceExpandHints( !bMark && pCursor && pCursor->IsAtEndOfMeta() );
    const SetAttrMode nInsertFlags = (bForceExpandHints)
        ?   ( SetAttrMode::FORCEHINTEXPAND
            | SetAttrMode::DONTEXPAND)
        : SetAttrMode::DONTEXPAND;

    std::vector<SwTextAttr *> oldMarks;
    if (bMark)
    {
        oldMarks = rPam.GetNode().GetTextNode()->GetTextAttrsAt(
            rPam.GetPoint()->nContent.GetIndex(), RES_TXTATR_REFMARK);
    }

    pDoc2->getIDocumentContentOperations().InsertPoolItem( rPam, aRefMark, nInsertFlags );

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
            rPam.GetNode().GetTextNode()->GetTextAttrsAt(
                rPam.GetPoint()->nContent.GetIndex(), RES_TXTATR_REFMARK));
        std::vector<SwTextAttr *>::const_iterator const iter(
            std::find_if(newMarks.begin(), newMarks.end(),
                NotContainedIn<SwTextAttr *>(oldMarks)));
        OSL_ASSERT(newMarks.end() != iter);
        if (newMarks.end() != iter)
        {
            pTextAttr = *iter;
        }
    }
    else
    {
        SwTextNode *pTextNd = rPam.GetNode().GetTextNode();
        OSL_ASSERT(pTextNd);
        pTextAttr = pTextNd ? rPam.GetNode().GetTextNode()->GetTextAttrForCharAt(
                rPam.GetPoint()->nContent.GetIndex() - 1, RES_TXTATR_REFMARK) : nullptr;
    }

    if (!pTextAttr)
    {
        throw uno::RuntimeException(
            "SwXReferenceMark::InsertRefMark(): cannot insert attribute", nullptr);
    }

    m_pMarkFormat = &pTextAttr->GetRefMark();

    const_cast<SwFormatRefMark*>(m_pMarkFormat)->Add(this);
}

void SAL_CALL
SwXReferenceMark::attach(const uno::Reference< text::XTextRange > & xTextRange)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }
    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = nullptr;
    OTextCursorHelper* pCursor = nullptr;
    if(xRangeTunnel.is())
    {
        pRange = ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
        pCursor =
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);
    }
    SwDoc *const pDocument =
        (pRange) ? &pRange->GetDoc() : ((pCursor) ? pCursor->GetDoc() : nullptr);
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
        if (pNewMark && (pNewMark == m_pImpl->m_pMarkFormat))
        {
            SwTextRefMark const*const pTextMark =
                m_pImpl->m_pMarkFormat->GetTextRefMark();
            if (pTextMark &&
                (&pTextMark->GetTextNode().GetNodes() ==
                    &m_pImpl->m_pDoc->GetNodes()))
            {
                SwTextNode const& rTextNode = pTextMark->GetTextNode();
                const std::unique_ptr<SwPaM> pPam( (pTextMark->End())
                    ?   new SwPaM( rTextNode, *pTextMark->End(),
                                   rTextNode, pTextMark->GetStart())
                    :   new SwPaM( rTextNode, pTextMark->GetStart()) );

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
        if (pNewMark && (pNewMark == m_pImpl->m_pMarkFormat))
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
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXReferenceMark::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
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
            && pCurMark && (pCurMark == m_pImpl->m_pMarkFormat))
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
                m_pImpl->m_pDoc = aPam.GetDoc();
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
        throw beans::UnknownPropertyException();
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

class SwXMetaText : public cppu::OWeakObject, public SwXText
{
private:
    SwXMeta & m_rMeta;

    virtual void PrepareForAttach(uno::Reference< text::XTextRange > & xRange,
            const SwPaM & rPam) override;

    virtual bool CheckForOwnMemberMeta(const SwPaM & rPam, const bool bAbsorb) override;

protected:
    virtual const SwStartNode *GetStartNode() const override;
    virtual uno::Reference< text::XTextCursor >
        CreateCursor() override;

public:
    SwXMetaText(SwDoc & rDoc, SwXMeta & rMeta);

    /// make available for SwXMeta
    using SwXText::Invalidate;

    // XInterface
    virtual void SAL_CALL acquire() throw() override { cppu::OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw() override { cppu::OWeakObject::release(); }

    // XTypeProvider
    virtual uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId() override;

    // XText
    virtual uno::Reference< text::XTextCursor >  SAL_CALL
        createTextCursor() override;
    virtual uno::Reference< text::XTextCursor >  SAL_CALL
        createTextCursorByRange(
            const uno::Reference< text::XTextRange > & xTextPosition) override;

};

SwXMetaText::SwXMetaText(SwDoc & rDoc, SwXMeta & rMeta)
    : SwXText(&rDoc, CURSOR_META)
    , m_rMeta(rMeta)
{
}

const SwStartNode *SwXMetaText::GetStartNode() const
{
    SwXText const * const pParent(
            dynamic_cast<SwXText*>(m_rMeta.GetParentText().get()));
    return (pParent) ? pParent->GetStartNode() : nullptr;
}

void SwXMetaText::PrepareForAttach( uno::Reference<text::XTextRange> & xRange,
        const SwPaM & rPam)
{
    // create a new cursor to prevent modifying SwXTextRange
    xRange = static_cast<text::XWordCursor*>(
        new SwXTextCursor(*GetDoc(), &m_rMeta, CURSOR_META, *rPam.GetPoint(),
                (rPam.HasMark()) ? rPam.GetMark() : nullptr));
}

bool SwXMetaText::CheckForOwnMemberMeta(const SwPaM & rPam, const bool bAbsorb)
{
    return m_rMeta.CheckForOwnMemberMeta(rPam, bAbsorb);
}

uno::Reference< text::XTextCursor > SwXMetaText::CreateCursor()
{
    uno::Reference< text::XTextCursor > xRet;
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
            xRet = static_cast<text::XWordCursor*>(
                    new SwXTextCursor(*GetDoc(), &m_rMeta, CURSOR_META, aPos));
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
uno::Reference< text::XTextCursor > SAL_CALL
SwXMetaText::createTextCursor()
{
    return CreateCursor();
}

uno::Reference< text::XTextCursor > SAL_CALL
SwXMetaText::createTextCursorByRange(
        const uno::Reference<text::XTextRange> & xTextPosition)
{
    const uno::Reference<text::XTextCursor> xCursor( CreateCursor() );
    xCursor->gotoRange(xTextPosition, false);
    return xCursor;
}

// the Meta has a cached list of text portions for its contents
// this list is created by SwXTextPortionEnumeration
// the Meta listens at the SwTextNode and throws away the cache when it changes
class SwXMeta::Impl
    : public SwClient
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper2

public:
    uno::WeakReference<uno::XInterface> m_wThis;
    ::comphelper::OInterfaceContainerHelper2 m_EventListeners;
    std::unique_ptr<const TextRangeList_t> m_pTextPortions;
    // 3 possible states: not attached, attached, disposed
    bool m_bIsDisposed;
    bool m_bIsDescriptor;
    uno::Reference<text::XText> m_xParentText;
    rtl::Reference<SwXMetaText> m_xText;

    Impl(   SwXMeta & rThis, SwDoc & rDoc,
            ::sw::Meta * const pMeta,
            uno::Reference<text::XText> const& xParentText,
            TextRangeList_t const * const pPortions)
        : SwClient(pMeta)
        , m_EventListeners(m_Mutex)
        , m_pTextPortions( pPortions )
        , m_bIsDisposed( false )
        , m_bIsDescriptor(nullptr == pMeta)
        , m_xParentText(xParentText)
        , m_xText(new SwXMetaText(rDoc, rThis))
    {
    }

    inline const ::sw::Meta * GetMeta() const;
    // only for SwXMetaField!
    inline const ::sw::MetaField * GetMetaField() const;
protected:
    // SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

};

inline const ::sw::Meta * SwXMeta::Impl::GetMeta() const
{
    return static_cast< const ::sw::Meta * >(GetRegisteredIn());
}

// SwModify
void SwXMeta::Impl::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    m_pTextPortions.reset(); // throw away cache (SwTextNode changed)

    ClientModify(this, pOld, pNew);

    if (GetRegisteredIn())
    {
        return; // core object still alive
    }

    m_bIsDisposed = true;
    m_xText->Invalidate();
    uno::Reference<uno::XInterface> const xThis(m_wThis);
    if (!xThis.is())
    {   // fdo#72695: if UNO object is already dead, don't revive it with event
        return;
    }
    lang::EventObject const ev(xThis);
    m_EventListeners.disposeAndClear(ev);
}

uno::Reference<text::XText> SwXMeta::GetParentText() const
{
    return m_pImpl->m_xParentText;
}

SwXMeta::SwXMeta(SwDoc *const pDoc, ::sw::Meta *const pMeta,
        uno::Reference<text::XText> const& xParentText,
        TextRangeList_t const*const pPortions)
    : m_pImpl( new SwXMeta::Impl(*this, *pDoc, pMeta, xParentText, pPortions) )
{
}

SwXMeta::SwXMeta(SwDoc *const pDoc)
    : m_pImpl( new SwXMeta::Impl(*this, *pDoc, nullptr, nullptr, nullptr) )
{
}

SwXMeta::~SwXMeta()
{
}

uno::Reference<rdf::XMetadatable>
SwXMeta::CreateXMeta(SwDoc & rDoc, bool const isField)
{
    SwXMeta *const pXMeta((isField)
            ? new SwXMetaField(& rDoc) : new SwXMeta(& rDoc));
    // this is why the constructor is private: need to acquire pXMeta here
    uno::Reference<rdf::XMetadatable> const xMeta(pXMeta);
    // need a permanent Reference to initialize m_wThis
    pXMeta->m_pImpl->m_wThis = xMeta;
    return xMeta;
}

uno::Reference<rdf::XMetadatable>
SwXMeta::CreateXMeta(::sw::Meta & rMeta,
            uno::Reference<text::XText> const& i_xParent,
            std::unique_ptr<TextRangeList_t const> && pPortions)
{
    // re-use existing SwXMeta
    // #i105557#: do not iterate over the registered clients: race condition
    uno::Reference<rdf::XMetadatable> xMeta(rMeta.GetXMeta());
    if (xMeta.is())
    {
        if (pPortions.get()) // set cache in the XMeta to the given portions
        {
            const uno::Reference<lang::XUnoTunnel> xUT(xMeta, uno::UNO_QUERY);
            SwXMeta *const pXMeta(
                ::sw::UnoTunnelGetImplementation<SwXMeta>(xUT));
            assert(pXMeta);
            // NB: the meta must always be created with the complete content
            // if SwXTextPortionEnumeration is created for a selection,
            // it must be checked that the Meta is contained in the selection!
            pXMeta->m_pImpl->m_pTextPortions = std::move(pPortions);
            // ??? is this necessary?
            if (pXMeta->m_pImpl->m_xParentText.get() != i_xParent.get())
            {
                SAL_WARN("sw.uno", "SwXMeta with different parent?");
                pXMeta->m_pImpl->m_xParentText.set(i_xParent);
            }
        }
        return xMeta;
    }

    // create new SwXMeta
    SwTextNode * const pTextNode( rMeta.GetTextNode() );
    SAL_WARN_IF(!pTextNode, "sw.uno", "CreateXMeta: no text node?");
    if (!pTextNode) { return nullptr; }
    uno::Reference<text::XText> xParentText(i_xParent);
    if (!xParentText.is())
    {
        SwTextMeta * const pTextAttr( rMeta.GetTextAttr() );
        SAL_WARN_IF(!pTextAttr, "sw.uno", "CreateXMeta: no text attr?");
        if (!pTextAttr) { return nullptr; }
        const SwPosition aPos(*pTextNode, pTextAttr->GetStart());
        xParentText.set( ::sw::CreateParentXText(*pTextNode->GetDoc(), aPos) );
    }
    if (!xParentText.is()) { return nullptr; }
    SwXMeta *const pXMeta( (RES_TXTATR_META == rMeta.GetFormatMeta()->Which())
        ? new SwXMeta     (pTextNode->GetDoc(), &rMeta, xParentText,
                            pPortions.release()) // temporarily un-unique_ptr :-(
        : new SwXMetaField(pTextNode->GetDoc(), &rMeta, xParentText,
                            pPortions.release()));
    // this is why the constructor is private: need to acquire pXMeta here
    xMeta.set(pXMeta);
    // in order to initialize the weak pointer cache in the core object
    rMeta.SetXMeta(xMeta);
    // need a permanent Reference to initialize m_wThis
    pXMeta->m_pImpl->m_wThis = xMeta;
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
    if (&pStartPos->nNode.GetNode() != pTextNode)
    {
        throw lang::IllegalArgumentException(
            "trying to insert into a nesting text content, but start "
                "of text range not in same paragraph as text content",
                nullptr, 0);
    }
    bool bForceExpandHints(false);
    const sal_Int32 nStartPos(pStartPos->nContent.GetIndex());
    // not <= but < because nMetaStart is behind dummy char!
    // not >= but > because == means insert at end!
    if ((nStartPos < nMetaStart) || (nStartPos > nMetaEnd))
    {
        throw lang::IllegalArgumentException(
            "trying to insert into a nesting text content, but start "
                "of text range not inside text content",
                nullptr, 0);
    }
    else if (nStartPos == nMetaEnd)
    {
        bForceExpandHints = true;
    }
    if (rPam.HasMark() && bAbsorb)
    {
        SwPosition const * const pEndPos( rPam.End() );
        if (&pEndPos->nNode.GetNode() != pTextNode)
        {
            throw lang::IllegalArgumentException(
                "trying to insert into a nesting text content, but end "
                    "of text range not in same paragraph as text content",
                    nullptr, 0);
        }
        const sal_Int32 nEndPos(pEndPos->nContent.GetIndex());
        // not <= but < because nMetaStart is behind dummy char!
        // not >= but > because == means insert at end!
        if ((nEndPos < nMetaStart) || (nEndPos > nMetaEnd))
        {
            throw lang::IllegalArgumentException(
                "trying to insert into a nesting text content, but end "
                    "of text range not inside text content",
                    nullptr, 0);
        }
        else if (nEndPos == nMetaEnd)
        {
            bForceExpandHints = true;
        }
    }
    return bForceExpandHints;
}

namespace
{
    class theSwXMetaUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXMetaUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXMeta::getUnoTunnelId()
{
    return theSwXMetaUnoTunnelId::get().getSeq();
}

// XUnoTunnel
sal_Int64 SAL_CALL
SwXMeta::getSomething( const uno::Sequence< sal_Int8 > & i_rId )
{
    return ::sw::UnoTunnelImpl<SwXMeta>(i_rId, this);
}

// XServiceInfo
OUString SAL_CALL
SwXMeta::getImplementationName()
{
    return OUString("SwXMeta");
}

static char const*const g_ServicesMeta[] =
{
    "com.sun.star.text.TextContent",
    "com.sun.star.text.InContentMetadata",
};

static const size_t g_nServicesMeta(SAL_N_ELEMENTS(g_ServicesMeta));

sal_Bool SAL_CALL
SwXMeta::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXMeta::getSupportedServiceNames()
{
    return ::sw::GetSupportedServiceNamesImpl(g_nServicesMeta, g_ServicesMeta);
}

// XComponent
void SAL_CALL
SwXMeta::addEventListener(
        uno::Reference< lang::XEventListener> const & xListener )
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL
SwXMeta::removeEventListener(
        uno::Reference< lang::XEventListener> const & xListener )
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

void SAL_CALL
SwXMeta::dispose()
{
    SolarMutexGuard g;

    if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->m_pTextPortions.reset();
        lang::EventObject const ev(static_cast< ::cppu::OWeakObject&>(*this));
        m_pImpl->m_EventListeners.disposeAndClear(ev);
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
            SwDoc * const pDoc( pTextNode->GetDoc() );
            pDoc->getIDocumentContentOperations().DeleteAndJoin( aPam );

            // removal should call Modify and do the dispose
            assert(m_pImpl->m_bIsDisposed);
        }
    }
}

void SAL_CALL
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
            "SwXMeta::attach(): already attached",
            static_cast< ::cppu::OWeakObject* >(this));
    }

    uno::Reference<lang::XUnoTunnel> xRangeTunnel(i_xTextRange, uno::UNO_QUERY);
    if (!xRangeTunnel.is())
    {
        throw lang::IllegalArgumentException(
            "SwXMeta::attach(): argument is no XUnoTunnel",
            static_cast< ::cppu::OWeakObject* >(this), 0);
    }
    SwXTextRange *const pRange(
            ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel));
    OTextCursorHelper *const pCursor( (pRange) ? nullptr :
            ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel));
    if (!pRange && !pCursor)
    {
        throw lang::IllegalArgumentException(
            "SwXMeta::attach(): argument not supported type",
            static_cast< ::cppu::OWeakObject* >(this), 0);
    }

    SwDoc * const pDoc(
            pRange ? &pRange->GetDoc() : pCursor->GetDoc());
    if (!pDoc)
    {
        throw lang::IllegalArgumentException(
            "SwXMeta::attach(): argument has no SwDoc",
            static_cast< ::cppu::OWeakObject* >(this), 0);
    }

    SwUnoInternalPaM aPam(*pDoc);
    ::sw::XTextRangeToSwPaM(aPam, i_xTextRange);

    UnoActionContext aContext(pDoc);

    SwXTextCursor const*const pTextCursor(
            dynamic_cast<SwXTextCursor*>(pCursor));
    const bool bForceExpandHints(pTextCursor && pTextCursor->IsAtEndOfMeta());
    const SetAttrMode nInsertFlags( (bForceExpandHints)
        ?   ( SetAttrMode::FORCEHINTEXPAND
            | SetAttrMode::DONTEXPAND)
        : SetAttrMode::DONTEXPAND );

    const std::shared_ptr< ::sw::Meta> pMeta( (RES_TXTATR_META == i_nWhich)
        ? std::make_shared< ::sw::Meta>( )
        : std::shared_ptr< ::sw::Meta>(
            pDoc->GetMetaFieldManager().makeMetaField()) );
    SwFormatMeta meta(pMeta, i_nWhich); // this is cloned by Insert!
    const bool bSuccess( pDoc->getIDocumentContentOperations().InsertPoolItem( aPam, meta, nInsertFlags ) );
    SwTextAttr * const pTextAttr( pMeta->GetTextAttr() );
    if (!bSuccess)
    {
        throw lang::IllegalArgumentException(
            "SwXMeta::attach(): cannot create meta: range invalid?",
            static_cast< ::cppu::OWeakObject* >(this), 1);
    }
    if (!pTextAttr)
    {
        OSL_FAIL("meta inserted, but has no text attribute?");
        throw uno::RuntimeException(
            "SwXMeta::attach(): cannot create meta",
            static_cast< ::cppu::OWeakObject* >(this));
    }

    pMeta->Add(m_pImpl.get());
    pMeta->SetXMeta(uno::Reference<rdf::XMetadatable>(this));

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
                "SwXMeta::getAnchor(): not inserted",
                static_cast< ::cppu::OWeakObject* >(this));
    }

    SwTextNode * pTextNode;
    sal_Int32 nMetaStart;
    sal_Int32 nMetaEnd;
    const bool bSuccess(SetContentRange(pTextNode, nMetaStart, nMetaEnd));
    OSL_ENSURE(bSuccess, "no pam?");
    if (!bSuccess)
    {
        throw lang::DisposedException(
                "SwXMeta::getAnchor(): not attached",
                static_cast< ::cppu::OWeakObject* >(this));
    }

    const SwPosition start(*pTextNode, nMetaStart - 1); // -1 due to CH_TXTATR
    const SwPosition end(*pTextNode, nMetaEnd);
    return SwXTextRange::CreateXTextRange(*pTextNode->GetDoc(), start, &end);
}

// XTextRange
uno::Reference< text::XText > SAL_CALL
SwXMeta::getText()
{
    SolarMutexGuard g;
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
    throw lang::NoSupportException("setting parent not supported", *this);
}

// XElementAccess
uno::Type SAL_CALL
SwXMeta::getElementType()
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL
SwXMeta::hasElements()
{
    SolarMutexGuard g;

    return m_pImpl->GetRegisteredIn() != nullptr;
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
                "createEnumeration(): not inserted",
                static_cast< ::cppu::OWeakObject* >(this));
    }

    SwTextNode * pTextNode;
    sal_Int32 nMetaStart;
    sal_Int32 nMetaEnd;
    const bool bSuccess(SetContentRange(pTextNode, nMetaStart, nMetaEnd));
    OSL_ENSURE(bSuccess, "no pam?");
    if (!bSuccess)
        throw lang::DisposedException();

    SwPaM aPam(*pTextNode, nMetaStart);

    if (!m_pImpl->m_pTextPortions.get())
    {
        return new SwXTextPortionEnumeration(
                    aPam, GetParentText(), nMetaStart, nMetaEnd);
    }
    else // cached!
    {
        return new SwXTextPortionEnumeration(aPam, *m_pImpl->m_pTextPortions);
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
            SwDocShell const * const pShell(pTextNode->GetDoc()->GetDocShell());
            return (pShell) ? pShell->GetModel() : nullptr;
        }
    }
    return nullptr;
}

inline const ::sw::MetaField * SwXMeta::Impl::GetMetaField() const
{
    return static_cast< const ::sw::MetaField * >(GetRegisteredIn());
}

SwXMetaField::SwXMetaField(SwDoc *const pDoc, ::sw::Meta *const pMeta,
        uno::Reference<text::XText> const& xParentText,
        TextRangeList_t const*const pPortions)
    : SwXMetaField_Base(pDoc, pMeta, xParentText, pPortions)
{
    OSL_ENSURE(pMeta && dynamic_cast< ::sw::MetaField* >(pMeta),
        "SwXMetaField created for wrong hint!");
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
    return OUString("SwXMetaField");
}

static char const*const g_ServicesMetaField[] =
{
    "com.sun.star.text.TextContent",
    "com.sun.star.text.TextField",
    "com.sun.star.text.textfield.MetadataField",
};

static const size_t g_nServicesMetaField(SAL_N_ELEMENTS(g_ServicesMetaField));

sal_Bool SAL_CALL
SwXMetaField::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXMetaField::getSupportedServiceNames()
{
    return ::sw::GetSupportedServiceNamesImpl(
            g_nServicesMetaField, g_ServicesMetaField);
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
        throw beans::UnknownPropertyException();
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
        throw beans::UnknownPropertyException();
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
lcl_getURI(const bool bPrefix)
{
    static uno::Reference< uno::XComponentContext > xContext(
        ::comphelper::getProcessComponentContext());
    static uno::Reference< rdf::XURI > xOdfPrefix(
        rdf::URI::createKnown(xContext, rdf::URIs::ODF_PREFIX),
        uno::UNO_SET_THROW);
    static uno::Reference< rdf::XURI > xOdfSuffix(
        rdf::URI::createKnown(xContext, rdf::URIs::ODF_SUFFIX),
        uno::UNO_SET_THROW);
    return (bPrefix) ? xOdfPrefix : xOdfSuffix;
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
        OUString *const o_pPrefix, OUString *const o_pSuffix)
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
            *o_pPrefix = lcl_getPrefixOrSuffix(xRepo, xMeta, lcl_getURI(true));
        }
        if (o_pSuffix)
        {
            *o_pSuffix = lcl_getPrefixOrSuffix(xRepo, xMeta, lcl_getURI(false));
        }
    } catch (uno::RuntimeException &) {
        throw;
    } catch (const uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            "getPrefixAndSuffix: exception",
            nullptr, uno::makeAny(e));
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
        const OUString content( this->getString() );
        OUString prefix;
        OUString suffix;
        getPrefixAndSuffix(GetModel(), this, &prefix, &suffix);
        return prefix + content + suffix;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
