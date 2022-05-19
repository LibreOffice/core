/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <comphelper/comphelperdllapi.h>
#include <com/sun/star/uno/Sequence.hxx>

namespace comphelper
{
/**
 * Interface that we can cast to, to bypass the inefficiency of using Sequence<sal_Int8>
 * when reading via XInputStream.
 */
class COMPHELPER_DLLPUBLIC ByteReader
{
public:
    virtual ~ByteReader();
    virtual sal_Int32 readSomeBytes(sal_Int8* aData, sal_Int32 nBytesToRead) = 0;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
};

/**
 * Interface that we can cast to, to bypass the inefficiency of using Sequence<sal_Int8>
 * when writing via XOutputStream.
 */
class COMPHELPER_DLLPUBLIC ByteWriter
{
public:
    virtual ~ByteWriter();
    virtual sal_Int32 writeSomeBytes(const sal_Int8* aData, sal_Int32 nBytesToWrite) = 0;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
};

} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
