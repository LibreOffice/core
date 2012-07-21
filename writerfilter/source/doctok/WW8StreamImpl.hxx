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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <doctok/WW8Document.hxx>
#include <resourcemodel/OutputWithDepth.hxx>

namespace writerfilter {
namespace doctok
{
using namespace ::com::sun::star;

class WW8StreamImpl : public WW8Stream
{
    uno::Reference<uno::XComponentContext> mrComponentContext;
    uno::Reference<io::XInputStream> mrStream;
    uno::Reference<container::XNameContainer> xOLESimpleStorage;
    uno::Reference<lang::XMultiComponentFactory> xFactory;

public:
    WW8StreamImpl(uno::Reference<uno::XComponentContext> rContext,
                  uno::Reference<io::XInputStream> rStream);
    virtual ~WW8StreamImpl();

    virtual WW8Stream::Pointer_t getSubStream(const OUString & rId);

    virtual Sequence get(sal_uInt32 nOffset, sal_uInt32 nCount)
        const;

    virtual string getSubStreamNames() const;
    virtual uno::Sequence<OUString> getSubStreamUNames() const;

    virtual void dump(OutputWithDepth<string> & o) const;
};
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
