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
#include <sal/log.hxx>
#include <svl/listener.hxx>
#include <svl/itemprop.hxx>

#include <IDocumentContentOperations.hxx>
#include <doc.hxx>
#include <formatlinebreak.hxx>
#include <unotextrange.hxx>
#include <ndtxt.hxx>
#include <textlinebreak.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>

using namespace com::sun::star;

/// The inner part SwXLineBreak, which is deleted with a locked SolarMutex.
class SwXLineBreak::Impl : public SvtListener
{
public:
    bool m_bIsDescriptor;
    SwFormatLineBreak* m_pFormatLineBreak;
    SwLineBreakClear m_eClear;

    Impl(SwFormatLineBreak* const pLineBreak)
        : m_bIsDescriptor(pLineBreak == nullptr)
        , m_pFormatLineBreak(pLineBreak)
        , m_eClear(SwLineBreakClear::NONE)
    {
        if (m_pFormatLineBreak)
        {
            StartListening(m_pFormatLineBreak->GetNotifier());
        }
    }

    const SwFormatLineBreak* GetLineBreakFormat() const;

    const SwFormatLineBreak& GetLineBreakFormatOrThrow() const;

    void Invalidate();

protected:
    void Notify(const SfxHint& rHint) override;
};

const SwFormatLineBreak* SwXLineBreak::Impl::GetLineBreakFormat() const
{
    return m_pFormatLineBreak;
}

const SwFormatLineBreak& SwXLineBreak::Impl::GetLineBreakFormatOrThrow() const
{
    const SwFormatLineBreak* pLineBreak(GetLineBreakFormat());
    if (!pLineBreak)
    {
        throw uno::RuntimeException(u"SwXLineBreak: disposed or invalid"_ustr, nullptr);
    }

    return *pLineBreak;
}

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
    : m_pImpl(new SwXLineBreak::Impl(&rFormat))
{
}

SwXLineBreak::SwXLineBreak()
    : m_pImpl(new SwXLineBreak::Impl(nullptr))
{
}

SwXLineBreak::~SwXLineBreak() {}

rtl::Reference<SwXLineBreak> SwXLineBreak::CreateXLineBreak(SwFormatLineBreak* pLineBreakFormat)
{
    rtl::Reference<SwXLineBreak> xLineBreak;
    if (pLineBreakFormat)
    {
        xLineBreak = pLineBreakFormat->GetXTextContent();
    }
    if (!xLineBreak.is())
    {
        if (pLineBreakFormat)
        {
            xLineBreak = new SwXLineBreak(*pLineBreakFormat);
            pLineBreakFormat->SetXLineBreak(xLineBreak);
        }
        else
            xLineBreak = new SwXLineBreak;
    }
    return xLineBreak;
}

OUString SAL_CALL SwXLineBreak::getImplementationName() { return u"SwXLineBreak"_ustr; }

sal_Bool SAL_CALL SwXLineBreak::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL SwXLineBreak::getSupportedServiceNames()
{
    return { u"com.sun.star.text.LineBreak"_ustr };
}

void SAL_CALL SwXLineBreak::attach(const uno::Reference<text::XTextRange>& xTextRange)
{
    SolarMutexGuard aGuard;
    if (!m_pImpl->m_bIsDescriptor)
    {
        throw uno::RuntimeException();
    }

    auto pRange = dynamic_cast<SwXTextRange*>(xTextRange.get());
    if (!pRange)
    {
        throw lang::IllegalArgumentException();
    }

    SwDoc& rNewDoc = pRange->GetDoc();
    SwUnoInternalPaM aPam(rNewDoc);
    sw::XTextRangeToSwPaM(aPam, xTextRange);
    UnoActionContext aContext(&rNewDoc);
    SwFormatLineBreak aLineBreak(m_pImpl->m_eClear);
    SetAttrMode nInsertFlags = SetAttrMode::DEFAULT;
    rNewDoc.getIDocumentContentOperations().InsertPoolItem(aPam, aLineBreak, nInsertFlags);
    auto pTextAttr
        = static_cast<SwTextLineBreak*>(aPam.GetPointNode().GetTextNode()->GetTextAttrForCharAt(
            aPam.GetPoint()->GetContentIndex() - 1, RES_TXTATR_LINEBREAK));
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

    return m_pImpl->GetLineBreakFormatOrThrow().GetAnchor();
}

void SAL_CALL SwXLineBreak::dispose()
{
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

    static uno::Reference<beans::XPropertySetInfo> xRet
        = aSwMapProvider.GetPropertySet(PROPERTY_MAP_LINEBREAK)->getPropertySetInfo();
    return xRet;
}

void SAL_CALL SwXLineBreak::setPropertyValue(const OUString& rPropertyName,
                                             const css::uno::Any& rValue)
{
    SolarMutexGuard aGuard;

    if (rPropertyName != UNO_NAME_CLEAR)
    {
        throw lang::IllegalArgumentException();
    }

    if (m_pImpl->m_bIsDescriptor)
    {
        sal_Int16 eValue{};
        if (rValue >>= eValue)
        {
            m_pImpl->m_eClear = static_cast<SwLineBreakClear>(eValue);
        }
    }
    else
    {
        m_pImpl->m_pFormatLineBreak->PutValue(rValue, 0);
    }
}

uno::Any SAL_CALL SwXLineBreak::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;

    uno::Any aRet;
    if (sw::GetDefaultTextContentValue(aRet, rPropertyName))
    {
        return aRet;
    }

    if (rPropertyName != UNO_NAME_CLEAR)
    {
        throw beans::UnknownPropertyException(rPropertyName);
    }

    if (m_pImpl->m_bIsDescriptor)
    {
        auto eValue = static_cast<sal_Int16>(m_pImpl->m_eClear);
        aRet <<= eValue;
    }
    else
    {
        aRet <<= m_pImpl->m_pFormatLineBreak->GetEnumValue();
    }
    return aRet;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
