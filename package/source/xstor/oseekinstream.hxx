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

#ifndef INCLUDED_PACKAGE_SOURCE_XSTOR_OSEEKINSTREAM_HXX
#define INCLUDED_PACKAGE_SOURCE_XSTOR_OSEEKINSTREAM_HXX

#include <com/sun/star/io/XSeekable.hpp>

#include "ocompinstream.hxx"

class OInputSeekStream final : public OInputCompStream
                        , public css::io::XSeekable
{
    css::uno::Reference < css::io::XSeekable > m_xSeekable;

public:
    OInputSeekStream( OWriteStream_Impl& pImpl,
                      css::uno::Reference < css::io::XInputStream > const & xStream,
                      const css::uno::Sequence< css::beans::PropertyValue >& aProps,
                      sal_Int32 nStorageType );

    OInputSeekStream( css::uno::Reference < css::io::XInputStream > const & xStream,
                      const css::uno::Sequence< css::beans::PropertyValue >& aProps,
                      sal_Int32 nStorageType );

    virtual ~OInputSeekStream() override;

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    //XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) override;
    virtual sal_Int64 SAL_CALL getPosition() override;
    virtual sal_Int64 SAL_CALL getLength() override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
