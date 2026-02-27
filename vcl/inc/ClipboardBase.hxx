/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ClipboardSelectionType.hxx"

#include <com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase.hxx>
#include <osl/mutex.hxx>
#include <vcl/dllapi.h>

#pragma once

class VCL_DLLPUBLIC ClipboardBase
    : public cppu::WeakComponentImplHelper<css::datatransfer::clipboard::XSystemClipboard,
                                           css::lang::XServiceInfo>
{
    const ClipboardSelectionType m_eSelectionType;

protected:
    osl::Mutex m_aMutex;

    explicit ClipboardBase(ClipboardSelectionType eSelectionType);
    virtual ~ClipboardBase() override;

    ClipboardSelectionType GetSelectionType() { return m_eSelectionType; }

public:
    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XClipboard
    virtual OUString SAL_CALL getName() override;

    // XClipboardEx
    virtual sal_Int8 SAL_CALL getRenderingCapabilities() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
