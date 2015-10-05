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
#ifndef INCLUDED_FILTER_SOURCE_XMLFILTERADAPTOR_STREAMWRAP_HXX
#define INCLUDED_FILTER_SOURCE_XMLFILTERADAPTOR_STREAMWRAP_HXX

#include <osl/mutex.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <cppuhelper/implbase.hxx>

namespace osl
{
    class File;
}

namespace foo
{

//= OOutputStreamWrapper

typedef ::cppu::WeakImplHelper<css::io::XOutputStream> OutputStreamWrapper_Base;
    // needed for some compilers
class OOutputStreamWrapper : public OutputStreamWrapper_Base
{
    ::osl::File&        rStream;

public:
    explicit OOutputStreamWrapper(osl::File& _rStream) :rStream(_rStream) { }

// css::io::XOutputStream
    virtual void SAL_CALL writeBytes(const css::uno::Sequence< sal_Int8 >& aData) throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL flush() throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL closeOutput() throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

}   // namespace utl


#endif // INCLUDED_FILTER_SOURCE_XMLFILTERADAPTOR_STREAMWRAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
