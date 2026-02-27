/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ClipboardBase.hxx>

#include <cppuhelper/supportsservice.hxx>

ClipboardBase::ClipboardBase(ClipboardSelectionType eSelectionType)
    : cppu::WeakComponentImplHelper<css::datatransfer::clipboard::XSystemClipboard,
                                    css::lang::XServiceInfo>(m_aMutex)
    , m_eSelectionType(eSelectionType)
{
}

ClipboardBase::~ClipboardBase() {}

css::uno::Sequence<OUString> ClipboardBase::getSupportedServiceNames()
{
    return { u"com.sun.star.datatransfer.clipboard.SystemClipboard"_ustr };
}

sal_Bool ClipboardBase::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

OUString ClipboardBase::getName()
{
    switch (m_eSelectionType)
    {
        case ClipboardSelectionType::Clipboard:
            return u"CLIPBOARD"_ustr;
        case ClipboardSelectionType::Primary:
            return u"PRIMARY"_ustr;
        default:
            assert(false && "unhandled clipboard selection type");
            return OUString();
    }
}

sal_Int8 ClipboardBase::getRenderingCapabilities() { return 0; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
