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
#ifndef INCLUDED_COMPHELPER_PACKAGE_ZIPPACKAGEHELPER_HXX
#define INCLUDED_COMPHELPER_PACKAGE_ZIPPACKAGEHELPER_HXX

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XInputStream.hpp>


class ZipPackageHelper
{
public:
    ZipPackageHelper( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const OUString& sPackageURL);

    void savePackage();

    void addFile( const OUString& rSourceFile )
        throw( css::uno::Exception, std::exception );

    void addStream( css::uno::Reference< css::io::XInputStream > const & xInput,
                    const OUString& aName ) throw( css::uno::Exception );

    css::uno::Reference< css::uno::XInterface > addFolder( const OUString& rName )
        throw( css::uno::Exception, std::exception );

    void addFolderWithContent( const OUString& rDirURL );

private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::container::XHierarchicalNameAccess > mxHNameAccess;
    css::uno::Reference< css::lang::XSingleServiceFactory > mxFactory;
    css::uno::Reference< css::uno::XInterface > mxRootFolder;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
