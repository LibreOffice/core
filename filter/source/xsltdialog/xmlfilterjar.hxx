/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

    bool savePackage( const rtl::OUString& rPackageURL, const XMLFilterVector& rFilters );
    void openPackage( const rtl::OUString& rPackageURL, XMLFilterVector& rFilters );

private:
    void addFile( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xRootFolder, com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory > xFactory, const ::rtl::OUString& rSourceFile ) throw( com::sun::star::uno::Exception );

    bool copyFile( com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess > xIfc, ::rtl::OUString& rURL, const ::rtl::OUString& rTargetURL );
    bool copyFiles( com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess > xIfc, filter_info_impl* pFilter );

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;

    ::rtl::OUString sVndSunStarPackage;
    ::rtl::OUString sXSLTPath;
    ::rtl::OUString sDTDPath;
    ::rtl::OUString sTemplatePath;
    ::rtl::OUString sSpecialConfigManager;
    ::rtl::OUString sPump;
    ::rtl::OUString sProgPath;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
