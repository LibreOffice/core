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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_SDBCORETOOLS_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_SDBCORETOOLS_HXX

#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace dbaccess
{

    void notifyDataSourceModified(const css::uno::Reference< css::uno::XInterface >& _rxObject,bool _bModified);

    css::uno::Reference< css::uno::XInterface >
        getDataSource( const css::uno::Reference< css::uno::XInterface >& _rxDependentObject );

    /** retrieves a to-be-displayed string for a given caught exception;
    */
    OUString extractExceptionMessage( const css::uno::Reference< css::uno::XComponentContext >& _rContext, const css::uno::Any& _rError );

    namespace tools
    {
        namespace stor
        {
            bool    storageIsWritable_nothrow(
                        const css::uno::Reference< css::embed::XStorage >& _rxStorage
                    );

            /// commits a given storage if it's not readonly
            bool    commitStorageIfWriteable(
                        const css::uno::Reference< css::embed::XStorage >& _rxStorage
                    );
        }

    }

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_SDBCORETOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
