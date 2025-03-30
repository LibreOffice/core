/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <package/Inflater.hxx>
#include <zlib.h>

namespace ZipUtils
{
class DLLPUBLIC_PACKAGE InflateZlib : public Inflater
{
private:
    bool bFinished;
    bool bNeedDict;
    sal_Int32 nOffset;
    sal_Int32 nLength;
    sal_Int32 nLastInflateError;
    css::uno::Sequence<sal_Int8> sInBuffer;
    std::unique_ptr<z_stream> pStream;

    sal_Int32 doInflateBytes(css::uno::Sequence<sal_Int8>& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength);

public:
    explicit InflateZlib(bool bNoWrap);
    virtual ~InflateZlib() override;

    virtual void setInput(const css::uno::Sequence<sal_Int8>& rBuffer) override;
    virtual bool needsDictionary() const override { return bNeedDict; }
    virtual bool finished() const override { return bFinished; }
    virtual sal_Int32 doInflateSegment(css::uno::Sequence<sal_Int8>& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength) override;
    virtual void end() override final;
    virtual sal_Int32 getLastInflateError() const override { return nLastInflateError; }
};

} // namespace ZipUtils

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
