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
#ifndef INCLUDED_PACKAGE_SOURCE_ZIPPACKAGE_ZIPPACKAGEFOLDERENUMERATION_HXX
#define INCLUDED_PACKAGE_SOURCE_ZIPPACKAGE_ZIPPACKAGEFOLDERENUMERATION_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <HashMaps.hxx>

class ZipPackageFolderEnumeration : public cppu::WeakImplHelper
<
    com::sun::star::container::XEnumeration,
    com::sun::star::lang::XServiceInfo
>
{
protected:
    ContentHash& rContents;
    ContentHash::const_iterator aIterator;
public:
    //ZipPackageFolderEnumeration (unordered_map < OUString, com::sun::star::uno::Reference < com::sun::star::container::XNamed >, hashFunc, eqFunc > &rInput);
    ZipPackageFolderEnumeration (ContentHash &rInput);
    virtual ~ZipPackageFolderEnumeration();

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(  )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
