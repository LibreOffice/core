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
#pragma once

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vector>

class filter_info_impl;

class XMLFilterJarHelper
{
public:
    explicit XMLFilterJarHelper( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    bool savePackage( const OUString& rPackageURL, const std::vector<filter_info_impl*>& rFilters );
    void openPackage( const OUString& rPackageURL, std::vector< std::unique_ptr<filter_info_impl> >& rFilters );

private:
    /// @throws css::uno::Exception
    void addFile( css::uno::Reference< css::uno::XInterface > const & xRootFolder, css::uno::Reference< css::lang::XSingleServiceFactory > const & xFactory, const OUString& rSourceFile );

    static bool copyFile( const css::uno::Reference< css::container::XHierarchicalNameAccess >& xIfc, OUString& rURL, const OUString& rTargetURL );
    bool copyFiles( const css::uno::Reference< css::container::XHierarchicalNameAccess >& xIfc, filter_info_impl* pFilter );

    css::uno::Reference< css::uno::XComponentContext > mxContext;

    OUString sXSLTPath;
    OUString sTemplatePath;
    OUString sProgPath;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
