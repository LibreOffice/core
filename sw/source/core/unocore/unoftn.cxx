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
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>

#include <unomid.h>
#include <unofootnote.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unoparagraph.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <doc.hxx>
#include <ftnidx.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <ndtxt.hxx>
#include <unocrsr.hxx>
#include <hints.hxx>

using namespace ::com::sun::star;

class SwXFootnote::Impl
    : public SwClient
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper2

public:

    SwXFootnote &               m_rThis;
    uno::WeakReference<uno::XInterface> m_wThis;
    const bool                  m_bIsEndnote;
    ::comphelper::OInterfaceContainerHelper2  m_EventListeners;
    bool                                m_bIsDescriptor;
    const SwFormatFootnote *            m_pFormatFootnote;
    OUString             m_sLabel;

    Impl(   SwXFootnote & rThis,
            SwFormatFootnote *const pFootnote,
            const bool bIsEndnote)
        : SwClient(pFootnote)
        , m_rThis(rThis)
        , m_bIsEndnote(bIsEndnote)
        , m_EventListeners(m_Mutex)
        , m_bIsDescriptor(nullptr == pFootnote)
        , m_pFormatFootnote(pFootnote)
    {
    }

    const SwFormatFootnote* GetFootnoteFormat() const {
        return m_rThis.GetDoc() ? m_pFormatFootnote : nullptr;
    }

    SwFormatFootnote const& GetFootnoteFormatOrThrow() {
        SwFormatFootnote const*const pFootnote( GetFootnoteFormat() );
        if (!pFootnote) {
            throw uno::RuntimeException("SwXFootnote: disposed or invalid", nullptr);
        }
        return *pFootnote;
    }

    void    Invalidate();
protected:
    // SwClient
    virtual void Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew) override;

};

void SwXFootnote::Impl::Invalidate()
{
    if (GetRegisteredIn())
    {
        GetRegisteredIn()->Remove(this);
    }
    m_pFormatFootnote = nullptr;
    m_rThis.SetDoc(nullptr);
    uno::Reference<uno::XInterface> const xThis(m_wThis);
    if (!xThis.is())
    {   // fdo#72695: if UNO object is already dead, don't revive it with event
        return;
    }
    lang::EventObject const ev(xThis);
    m_EventListeners.disposeAndClear(ev);
}

void SwXFootnote::Impl::Modify(const SfxPoolItem *pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);

    if (!GetRegisteredIn()) // removed => dispose
    {
        Invalidate();
    }
}

SwXFootnote::SwXFootnote(const bool bEndnote)
    : SwXText(nullptr, CursorType::Footnote)
    , m_pImpl( new SwXFootnote::Impl(*this, nullptr, bEndnote) )
{
}

SwXFootnote::SwXFootnote(SwDoc & rDoc, SwFormatFootnote & rFormat)
    : SwXText(& rDoc, CursorType::Footnote)
    , m_pImpl( new SwXFootnote::Impl(*this, &rFormat, rFormat.IsEndNote()) )
{
}

SwXFootnote::~SwXFootnote()
{
}

uno::Reference<text::XFootnote>
SwXFootnote::CreateXFootnote(SwDoc & rDoc, SwFormatFootnote *const pFootnoteFormat,
        bool const isEndnote)
{
    // i#105557: do not iterate over the registered clients: race condition
    uno::Reference<text::XFootnote> xNote;
    if (pFootnoteFormat)
    {
        xNote = pFootnoteFormat->GetXFootnote();
    }
    if (!xNote.is())
    {
        SwXFootnote *const pNote((pFootnoteFormat)
                ? new SwXFootnote(rDoc, *pFootnoteFormat)
                : new SwXFootnote(isEndnote));
        xNote.set(pNote);
        if (pFootnoteFormat)
        {
            pFootnoteFormat->SetXFootnote(xNote);
        }
        // need a permanent Reference to initialize m_wThis
        pNote->m_pImpl->m_wThis = xNote;
    }
    return xNote;
}

namespace
{
    class theSwXFootnoteUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXFootnoteUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXFootnote::getUnoTunnelId()
{
    return theSwXFootnoteUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL
SwXFootnote::getSomething(const uno::Sequence< sal_Int8 >& rId)
{
    const sal_Int64 nRet( ::sw::UnoTunnelImpl<SwXFootnote>(rId, this) );
    return (nRet) ? nRet : SwXText::getSomething(rId);
}

OUString SAL_CALL
SwXFootnote::getImplementationName()
{
    return OUString("SwXFootnote");
}

static char const*const g_ServicesFootnote[] =
{
    "com.sun.star.text.TextContent",
    "com.sun.star.text.Footnote",
    "com.sun.star.text.Text",
    "com.sun.star.text.Endnote", // NB: only supported for endnotes!
};

static const size_t g_nServicesEndnote( SAL_N_ELEMENTS(g_ServicesFootnote) );

static const size_t g_nServicesFootnote( g_nServicesEndnote - 1 ); // NB: omit!

sal_Bool SAL_CALL SwXFootnote::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXFootnote::getSupportedServiceNames()
{
    SolarMutexGuard g;
    return ::sw::GetSupportedServiceNamesImpl(
            (m_pImpl->m_bIsEndnote) ? g_nServicesEndnote : g_nServicesFootnote,
            g_ServicesFootnote);
}

uno::Sequence< uno::Type > SAL_CALL
SwXFootnote::getTypes()
{
    const uno::Sequence< uno::Type > aTypes = SwXFootnote_Base::getTypes();
    const uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();
    return ::comphelper::concatSequences(aTypes, aTextTypes);
}

uno::Sequence< sal_Int8 > SAL_CALL
SwXFootnote::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Any SAL_CALL
SwXFootnote::queryInterface(const uno::Type& rType)
{
    const uno::Any ret = SwXFootnote_Base::queryInterface(rType);
    return (ret.getValueType() == cppu::UnoType<void>::get())
        ?   SwXText::queryInterface(rType)
        :   ret;
}

OUString SAL_CALL SwXFootnote::getLabel()
{
    SolarMutexGuard aGuard;

    OUString sRet;
    SwFormatFootnote const*const pFormat = m_pImpl->GetFootnoteFormat();
    if(pFormat)
    {
        sRet = pFormat->GetNumStr();
    }
    else if (m_pImpl->m_bIsDescriptor)
    {
        sRet = m_pImpl->m_sLabel;
    }
    else
    {
        throw uno::RuntimeException();
    }
    return sRet;
}

void SAL_CALL
SwXFootnote::setLabel(const OUString& aLabel)
{
    SolarMutexGuard aGuard;
    OUString newLabel(aLabel);
    //new line must not occur as footnote label
    if(newLabel.indexOf('\n') >=0 )
    {
       newLabel = newLabel.replace('\n', ' ');
    }
    SwFormatFootnote const*const pFormat = m_pImpl->GetFootnoteFormat();
    if(pFormat)
    {
        const SwTextFootnote* pTextFootnote = pFormat->GetTextFootnote();
        OSL_ENSURE(pTextFootnote, "kein TextNode?");
        SwTextNode& rTextNode = (SwTextNode&)pTextFootnote->GetTextNode();

        SwPaM aPam(rTextNode, pTextFootnote->GetStart());
        GetDoc()->SetCurFootnote(aPam, newLabel, pFormat->GetNumber(), pFormat->IsEndNote());
    }
    else if (m_pImpl->m_bIsDescriptor)
    {
        m_pImpl->m_sLabel = newLabel;
    }
    else
    {
        throw uno::RuntimeException();
    }
}

void SAL_CALL
SwXFootnote::attach(const uno::Reference< text::XTextRange > & xTextRange)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }
    const uno::Reference<lang::XUnoTunnel> xRangeTunnel(
            xTextRange, uno::UNO_QUERY);
    SwXTextRange *const pRange =
        ::sw::UnoTunnelGetImplementation<SwXTextRange>(xRangeTunnel);
    OTextCursorHelper *const pCursor =
        ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xRangeTunnel);
    SwDoc *const pNewDoc =
        (pRange) ? &pRange->GetDoc() : ((pCursor) ? pCursor->GetDoc() : nullptr);
    if (!pNewDoc)
    {
        throw lang::IllegalArgumentException();
    }

    SwUnoInternalPaM aPam(*pNewDoc);
    // this now needs to return TRUE
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);

    UnoActionContext aCont(pNewDoc);
    pNewDoc->getIDocumentContentOperations().DeleteAndJoin(aPam);
    aPam.DeleteMark();
    SwFormatFootnote aFootNote(m_pImpl->m_bIsEndnote);
    if (!m_pImpl->m_sLabel.isEmpty())
    {
        aFootNote.SetNumStr(m_pImpl->m_sLabel);
    }

    SwXTextCursor const*const pTextCursor(
            dynamic_cast<SwXTextCursor*>(pCursor));
    const bool bForceExpandHints( pTextCursor && pTextCursor->IsAtEndOfMeta() );
    const SetAttrMode nInsertFlags = (bForceExpandHints)
        ? SetAttrMode::FORCEHINTEXPAND
        : SetAttrMode::DEFAULT;

    pNewDoc->getIDocumentContentOperations().InsertPoolItem(aPam, aFootNote, nInsertFlags);

    SwTextFootnote *const pTextAttr = static_cast<SwTextFootnote*>(
        aPam.GetNode().GetTextNode()->GetTextAttrForCharAt(
                aPam.GetPoint()->nContent.GetIndex()-1, RES_TXTATR_FTN ));

    if (pTextAttr)
    {
        const SwFormatFootnote& rFootnote = pTextAttr->GetFootnote();
        m_pImpl->m_pFormatFootnote = &rFootnote;
        const_cast<SwFormatFootnote*>(m_pImpl->m_pFormatFootnote)->Add(m_pImpl.get());
        // force creation of sequence id - is used for references
        if (pNewDoc->IsInReading())
        {
            pTextAttr->SetSeqNo(pNewDoc->GetFootnoteIdxs().size());
        }
        else
        {
            pTextAttr->SetSeqRefNo();
        }
    }
    m_pImpl->m_bIsDescriptor = false;
    SetDoc(pNewDoc);
}

uno::Reference< text::XTextRange > SAL_CALL
SwXFootnote::getAnchor()
{
    SolarMutexGuard aGuard;

    SwFormatFootnote const& rFormat( m_pImpl->GetFootnoteFormatOrThrow() );

    SwTextFootnote const*const pTextFootnote = rFormat.GetTextFootnote();
    SwPaM aPam( pTextFootnote->GetTextNode(), pTextFootnote->GetStart() );
    SwPosition aMark( *aPam.Start() );
    aPam.SetMark();
    ++aPam.GetMark()->nContent;
    const uno::Reference< text::XTextRange > xRet =
        SwXTextRange::CreateXTextRange(*GetDoc(), *aPam.Start(), aPam.End());
    return xRet;
}

void SAL_CALL SwXFootnote::dispose()
{
    SolarMutexGuard aGuard;

    SwFormatFootnote const& rFormat( m_pImpl->GetFootnoteFormatOrThrow() );

    SwTextFootnote const*const pTextFootnote = rFormat.GetTextFootnote();
    OSL_ENSURE(pTextFootnote, "no TextNode?");
    SwTextNode& rTextNode = const_cast<SwTextNode&>(pTextFootnote->GetTextNode());
    const sal_Int32 nPos = pTextFootnote->GetStart();
    SwPaM aPam(rTextNode, nPos, rTextNode, nPos+1);
    GetDoc()->getIDocumentContentOperations().DeleteAndJoin( aPam );
}

void SAL_CALL
SwXFootnote::addEventListener(
    const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL
SwXFootnote::removeEventListener(
    const uno::Reference< lang::XEventListener > & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

const SwStartNode *SwXFootnote::GetStartNode() const
{
    SwFormatFootnote const*const   pFormat = m_pImpl->GetFootnoteFormat();
    if(pFormat)
    {
        const SwTextFootnote* pTextFootnote = pFormat->GetTextFootnote();
        if( pTextFootnote )
        {
            return pTextFootnote->GetStartNode()->GetNode().GetStartNode();
        }
    }
    return nullptr;
}

uno::Reference< text::XTextCursor >
SwXFootnote::CreateCursor()
{
    return createTextCursor();
}

uno::Reference< text::XTextCursor > SAL_CALL
SwXFootnote::createTextCursor()
{
    SolarMutexGuard aGuard;

    SwFormatFootnote const& rFormat( m_pImpl->GetFootnoteFormatOrThrow() );

    SwTextFootnote const*const pTextFootnote = rFormat.GetTextFootnote();
    SwPosition aPos( *pTextFootnote->GetStartNode() );
    SwXTextCursor *const pXCursor =
        new SwXTextCursor(*GetDoc(), this, CursorType::Footnote, aPos);
    auto& rUnoCursor(pXCursor->GetCursor());
    rUnoCursor.Move(fnMoveForward, GoInNode);
    const uno::Reference< text::XTextCursor > xRet =
        static_cast<text::XWordCursor*>(pXCursor);
    return xRet;
}

uno::Reference< text::XTextCursor > SAL_CALL
SwXFootnote::createTextCursorByRange(
    const uno::Reference< text::XTextRange > & xTextPosition)
{
    SolarMutexGuard aGuard;

    SwFormatFootnote const& rFormat( m_pImpl->GetFootnoteFormatOrThrow() );

    SwUnoInternalPaM aPam(*GetDoc());
    if (!::sw::XTextRangeToSwPaM(aPam, xTextPosition))
    {
        throw uno::RuntimeException();
    }

    SwTextFootnote const*const pTextFootnote = rFormat.GetTextFootnote();
    SwNode const*const pFootnoteStartNode = &pTextFootnote->GetStartNode()->GetNode();

    const SwNode* pStart = aPam.GetNode().FindFootnoteStartNode();
    if (pStart != pFootnoteStartNode)
    {
        throw uno::RuntimeException();
    }

    const uno::Reference< text::XTextCursor > xRet =
        static_cast<text::XWordCursor*>(
                new SwXTextCursor(*GetDoc(), this, CursorType::Footnote,
                    *aPam.GetPoint(), aPam.GetMark()));
    return xRet;
}

uno::Reference< container::XEnumeration > SAL_CALL
SwXFootnote::createEnumeration()
{
    SolarMutexGuard aGuard;

    SwFormatFootnote const& rFormat( m_pImpl->GetFootnoteFormatOrThrow() );

    SwTextFootnote const*const pTextFootnote = rFormat.GetTextFootnote();
    SwPosition aPos( *pTextFootnote->GetStartNode() );
    auto pUnoCursor(GetDoc()->CreateUnoCursor(aPos));
    pUnoCursor->Move(fnMoveForward, GoInNode);
    return SwXParagraphEnumeration::Create(this, pUnoCursor, CursorType::Footnote);
}

uno::Type SAL_CALL SwXFootnote::getElementType()
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL SwXFootnote::hasElements()
{
    return true;
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXFootnote::getPropertySetInfo()
{
    SolarMutexGuard g;
    static uno::Reference< beans::XPropertySetInfo > xRet =
        aSwMapProvider.GetPropertySet(PROPERTY_MAP_FOOTNOTE)
            ->getPropertySetInfo();
    return xRet;
}

void SAL_CALL
SwXFootnote::setPropertyValue(const OUString&, const uno::Any&)
{
    //no values to be set
    throw lang::IllegalArgumentException();
}

uno::Any SAL_CALL
SwXFootnote::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    uno::Any aRet;
    if (! ::sw::GetDefaultTextContentValue(aRet, rPropertyName))
    {
        if (rPropertyName == UNO_NAME_START_REDLINE ||
            rPropertyName == UNO_NAME_END_REDLINE)
        {
            //redline can only be returned if it's a living object
            if (!m_pImpl->m_bIsDescriptor)
            {
                aRet = SwXText::getPropertyValue(rPropertyName);
            }
        }
        else if (rPropertyName == UNO_NAME_REFERENCE_ID)
        {
            SwFormatFootnote const*const pFormat = m_pImpl->GetFootnoteFormat();
            if (pFormat)
            {
                SwTextFootnote const*const pTextFootnote = pFormat->GetTextFootnote();
                OSL_ENSURE(pTextFootnote, "no TextNode?");
                aRet <<= static_cast<sal_Int16>(pTextFootnote->GetSeqRefNo());
            }
        }
        else
        {
            beans::UnknownPropertyException aExcept;
            aExcept.Message = rPropertyName;
            throw aExcept;
        }
    }
    return aRet;
}

void SAL_CALL
SwXFootnote::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXFootnote::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXFootnote::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXFootnote::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXFootnote::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXFootnote::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXFootnote::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    OSL_FAIL("SwXFootnote::removeVetoableChangeListener(): not implemented");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
