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

#ifndef SC_XMLCODENAMEPROVIDER_HXX
#define SC_XMLCODENAMEPROVIDER_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/implbase1.hxx>

class ScDocument;

class XMLCodeNameProvider : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XNameAccess >
{
    ScDocument* mpDoc;
    OUString msDocName;
    OUString msCodeNameProp;

    static sal_Bool _getCodeName( const ::com::sun::star::uno::Any& aAny,
                           OUString& rCodeName );

public:
    XMLCodeNameProvider( ScDocument* pDoc );
    virtual ~XMLCodeNameProvider();

    virtual ::sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw (::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL hasElements()
        throw (::com::sun::star::uno::RuntimeException );

    static void set( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& xNameAccess, ScDocument *pDoc );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
