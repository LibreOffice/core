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
#ifndef _XMLFILTERJAR_HXX_
#define _XMLFILTERJAR_HXX_

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <vector>

class filter_info_impl;

typedef std::vector< filter_info_impl* > XMLFilterVector;

class XMLFilterJarHelper
{
public:
    XMLFilterJarHelper( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );

    bool savePackage( const OUString& rPackageURL, const XMLFilterVector& rFilters );
    void openPackage( const OUString& rPackageURL, XMLFilterVector& rFilters );

private:
    void addFile( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xRootFolder, com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory > xFactory, const OUString& rSourceFile ) throw( com::sun::star::uno::Exception );

    bool copyFile( com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess > xIfc, OUString& rURL, const OUString& rTargetURL );
    bool copyFiles( com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess > xIfc, filter_info_impl* pFilter );

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;

    OUString sVndSunStarPackage;
    OUString sXSLTPath;
    OUString sTemplatePath;
    OUString sSpecialConfigManager;
    OUString sPump;
    OUString sProgPath;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
