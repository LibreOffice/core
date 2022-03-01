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

#include <unolinebreak.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weakref.hxx>
#include <sal/log.hxx>
#include <svl/listener.hxx>

#include <IDocumentContentOperations.hxx>
#include <doc.hxx>
#include <formatlinebreak.hxx>
#include <unotextrange.hxx>
#include <ndtxt.hxx>
#include <textlinebreak.hxx>

using namespace com::sun::star;

class SwXLineBreak::Impl : public SvtListener
{
public:
    SwXLineBreak& m_rThis;
    uno::WeakReference<uno::XInterface> m_wThis;
    bool m_bIsDescriptor;
    SwFormatLineBreak* m_pFormatLineBreak;

    Impl(SwXLineBreak& rThis, SwFormatLineBreak* const pLineBreak)
        : m_rThis(rThis)
        , m_bIsDescriptor(pLineBreak == nullptr)
        , m_pFormatLineBreak(pLineBreak)
    {
        if (m_pFormatLineBreak)
        {
            StartListening(m_pFormatLineBreak->GetNotifier());
        }
    }

    void Invalidate();

protected:
    void Notify(const SfxHint& rHint) override;
};

void SwXLineBreak::Impl::Invalidate()
{
    EndListeningAll();
    m_pFormatLineBreak = nullptr;
}

void SwXLineBreak::Impl::Notify(const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::Dying)
    {
        Invalidate();
    }
}

SwXLineBreak::SwXLineBreak(SwFormatLineBreak& rFormat)
    : m_pImpl(new SwXLineBreak::Impl(*this, &rFormat))
{
}

SwXLineBreak::SwXLineBreak()
    : m_pImpl(new SwXLineBreak::Impl(*this, nullptr))
{
}

SwXLineBreak::~SwXLineBreak() {}

uno::Reference<text::XTextContent> SwXLineBreak::CreateXLineBreak()
{
    uno::Reference<text::XTextContent> xLineBreak;

    rtl::Reference<SwXLineBreak> pLineBreak(new SwXLineBreak);
    xLineBreak.set(pLineBreak);
    pLineBreak->m_pImpl->m_wThis = xLineBreak;

    return xLineBreak;
}

OUString SAL_CALL SwXLineBreak::getImplementationName() { return "SwXLineBreak"; }

sal_Bool SAL_CALL SwXLineBreak::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL SwXLineBreak::getSupportedServiceNames()
{
    return { "com.sun.star.text.LineBreak" };
}

void SAL_CALL SwXLineBreak::attach(const uno::Reference<text::XTextRange>& xTextRange)
{
    SolarMutexGuard aGuard;
    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }

    uno::Reference<lang::XUnoTunnel> xRangeTunnel(xTextRange, uno::UNO_QUERY);
    auto pRange = dynamic_cast<SwXTextRange*>(xTextRange.get());
    if (!pRange)
    {
        throw lang::IllegalArgumentException();
    }

    SwDoc& rNewDoc = pRange->GetDoc();
    SwUnoInternalPaM aPam(rNewDoc);
    sw::XTextRangeToSwPaM(aPam, xTextRange);
    UnoActionContext aContext(&rNewDoc);
    SwFormatLineBreak aLineBreak(SwLineBreakClear::ALL);
    SetAttrMode nInsertFlags = SetAttrMode::DEFAULT;
    rNewDoc.getIDocumentContentOperations().InsertPoolItem(aPam, aLineBreak, nInsertFlags);
    auto pTextAttr
        = static_cast<SwTextLineBreak*>(aPam.GetNode().GetTextNode()->GetTextAttrForCharAt(
            aPam.GetPoint()->nContent.GetIndex() - 1, RES_TXTATR_LINEBREAK));
    if (pTextAttr)
    {
        m_pImpl->EndListeningAll();
        auto pLineBreak = const_cast<SwFormatLineBreak*>(&pTextAttr->GetLineBreak());
        m_pImpl->m_pFormatLineBreak = pLineBreak;
        m_pImpl->StartListening(pLineBreak->GetNotifier());
    }
    m_pImpl->m_bIsDescriptor = false;
}

uno::Reference<text::XTextRange> SAL_CALL SwXLineBreak::getAnchor()
{
    SolarMutexGuard aGuard;

    SAL_WARN("sw.uno", "SwXLineBreak::getAnnchor: not implemented");

    return {};
}

void SAL_CALL SwXLineBreak::dispose()
{
    SolarMutexGuard aGuard;

    SAL_WARN("sw.uno", "SwXLineBreak::dispose: not implemented");
}

void SAL_CALL
SwXLineBreak::addEventListener(const uno::Reference<lang::XEventListener>& /*xListener*/)
{
}

void SAL_CALL
SwXLineBreak::removeEventListener(const uno::Reference<lang::XEventListener>& /*xListener*/)
{
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL SwXLineBreak::getPropertySetInfo()
{
    SolarMutexGuard aGuard;

    SAL_WARN("sw.uno", "SwXLineBreak::getPropertySetInfo: not implemented");

    return {};
}

void SAL_CALL SwXLineBreak::setPropertyValue(const OUString& /*rPropertyName*/,
                                             const css::uno::Any& /*rValue*/)
{
    SolarMutexGuard aGuard;

    SAL_WARN("sw.uno", "SwXLineBreak::setPropertySetInfo: not implemented");
}

uno::Any SAL_CALL SwXLineBreak::getPropertyValue(const OUString& /*rPropertyName*/)
{
    SolarMutexGuard aGuard;

    SAL_WARN("sw.uno", "SwXLineBreak::getPropertyValue: not implemented");

    return {};
}

void SAL_CALL SwXLineBreak::addPropertyChangeListener(
    const OUString& /*rPropertyName*/,
    const uno::Reference<beans::XPropertyChangeListener>& /*xListener*/)
{
    SAL_WARN("sw.uno", "SwXLineBreak::addPropertyChangeListener: not implemented");
}

void SAL_CALL SwXLineBreak::removePropertyChangeListener(
    const OUString& /*rPropertyName*/,
    const uno::Reference<beans::XPropertyChangeListener>& /*xListener*/)
{
    SAL_WARN("sw.uno", "SwXLineBreak::removePropertyChangeListener: not implemented");
}

void SAL_CALL SwXLineBreak::addVetoableChangeListener(
    const OUString& /*rPropertyName*/,
    const uno::Reference<beans::XVetoableChangeListener>& /*xListener*/)
{
    SAL_WARN("sw.uno", "SwXLineBreak::addVetoableChangeListener: not implemented");
}

void SAL_CALL SwXLineBreak::removeVetoableChangeListener(
    const OUString& /*rPropertyName*/,
    const uno::Reference<beans::XVetoableChangeListener>& /*xListener*/)
{
    SAL_WARN("sw.uno", "SwXLineBreak::removeVetoableChangeListener: not implemented");
}

sal_Int64 SAL_CALL SwXLineBreak::getSomething(const css::uno::Sequence<sal_Int8>& rIdentifier)
{
    return comphelper::getSomethingImpl<SwXLineBreak>(rIdentifier, this);
}

const uno::Sequence<sal_Int8>& SwXLineBreak::getUnoTunnelId()
{
    static const comphelper::UnoIdInit theSwXLineBreakUnoTunnelId;
    return theSwXLineBreakUnoTunnelId.getSeq();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
