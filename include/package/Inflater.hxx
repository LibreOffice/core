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

#ifndef INCLUDED_PACKAGE_INFLATER_HXX
#define INCLUDED_PACKAGE_INFLATER_HXX

#include <config_options.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <package/packagedllapi.hxx>
#include <memory>

namespace ZipUtils {

class UNLESS_MERGELIBS(DLLPUBLIC_PACKAGE) Inflater
{
public:
    Inflater() = default;
    virtual ~Inflater() = default;

    virtual void setInput(const css::uno::Sequence<sal_Int8>& rBuffer) = 0;
    virtual bool needsDictionary() const = 0;
    virtual bool finished() const = 0;
    virtual sal_Int32 doInflateSegment(css::uno::Sequence<sal_Int8>& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength) = 0;
    virtual void end() = 0;
    virtual sal_Int32 getLastInflateError() const = 0;
};

class UNLESS_MERGELIBS(DLLPUBLIC_PACKAGE) InflaterBytes
{
public:
    InflaterBytes() = default;
    virtual ~InflaterBytes() = default;

    virtual void setInput(const sal_Int8* pBuffer, sal_Int32 nLen) = 0;
    virtual bool finished() const = 0;
    virtual sal_Int32 doInflateSegment(sal_Int8* pOutBuffer, sal_Int32 nBufLen, sal_Int32 nNewOffset, sal_Int32 nNewLength) = 0;
    virtual void end() = 0;
};

} // namespace ZipUtils

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
