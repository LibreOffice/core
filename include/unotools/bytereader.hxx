/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <unotools/unotoolsdllapi.h>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>

namespace utl
{
/**
 * Interface that we can cast to, to bypass the inefficiency of using Sequence<sal_Int8>
 * when reading via XInputStream.
 */
class UNOTOOLS_DLLPUBLIC ByteReader : public cppu::WeakImplHelper<css::lang::XUnoTunnel>
{
public:
    virtual ~ByteReader();
    virtual sal_Int32 readSomeBytes(sal_Int8* aData, sal_Int32 nBytesToRead) = 0;

    // XUnoTunnel
    virtual SAL_CALL sal_Int64
    getSomething(const css::uno::Sequence<sal_Int8>& _rIdentifier) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
};

} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
