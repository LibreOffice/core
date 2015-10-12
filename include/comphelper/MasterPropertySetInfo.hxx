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

#ifndef INCLUDED_COMPHELPER_MASTERPROPERTYSETINFO_HXX
#define INCLUDED_COMPHELPER_MASTERPROPERTYSETINFO_HXX
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <comphelper/PropertyInfoHash.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>

namespace comphelper
{
    class COMPHELPER_DLLPUBLIC MasterPropertySetInfo:
        public ::cppu::WeakImplHelper<
        ::com::sun::star::beans::XPropertySetInfo >
    {
    public:
        MasterPropertySetInfo( PropertyInfo const * pMap );

    private:
        virtual ~MasterPropertySetInfo()
            throw();

        void add( PropertyInfoHash &rHash, sal_uInt8 nMapId );

        // XPropertySetInfo
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties()
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const OUString& aName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        PropertyDataHash maMap;
        com::sun::star::uno::Sequence < com::sun::star::beans::Property > maProperties;

        friend class MasterPropertySet;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
