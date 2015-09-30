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
#ifndef INCLUDED_FILTER_SOURCE_XSLTDIALOG_XMLFILTERJAR_HXX
#define INCLUDED_FILTER_SOURCE_XSLTDIALOG_XMLFILTERJAR_HXX

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <vector>

class filter_info_impl;

typedef std::vector< filter_info_impl* > XMLFilterVector;

class XMLFilterJarHelper
{
public:
    XMLFilterJarHelper( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    bool savePackage( const OUString& rPackageURL, const XMLFilterVector& rFilters );
    void openPackage( const OUString& rPackageURL, XMLFilterVector& rFilters );

private:
    void addFile( css::uno::Reference< css::uno::XInterface > xRootFolder, css::uno::Reference< css::lang::XSingleServiceFactory > xFactory, const OUString& rSourceFile ) throw( css::uno::Exception );

    bool copyFile( css::uno::Reference< css::container::XHierarchicalNameAccess > xIfc, OUString& rURL, const OUString& rTargetURL );
    bool copyFiles( css::uno::Reference< css::container::XHierarchicalNameAccess > xIfc, filter_info_impl* pFilter );

    css::uno::Reference< css::uno::XComponentContext > mxContext;

    OUString sVndSunStarPackage;
    OUString sXSLTPath;
    OUString sTemplatePath;
    OUString sProgPath;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
