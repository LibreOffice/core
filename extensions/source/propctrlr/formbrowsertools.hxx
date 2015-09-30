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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_FORMBROWSERTOOLS_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_FORMBROWSERTOOLS_HXX

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <rtl/ustring.hxx>

#include <functional>
#include <set>


namespace pcr
{


    OUString GetUIHeadlineName(sal_Int16 _nClassId, const css::uno::Any& _rUnoObject);
    sal_Int16 classifyComponent( const css::uno::Reference< css::uno::XInterface >& _rxComponent );


    struct FindPropertyByHandle : public ::std::unary_function< css::beans::Property, bool >
    {
    private:
        sal_Int32 m_nId;

    public:
        FindPropertyByHandle( sal_Int32 _nId ) : m_nId ( _nId ) { }
        bool operator()( const css::beans::Property& _rProp ) const
        {
            return m_nId == _rProp.Handle;
        }
    };


    struct FindPropertyByName : public ::std::unary_function< css::beans::Property, bool >
    {
    private:
        OUString m_sName;

    public:
        FindPropertyByName( const OUString& _rName ) : m_sName( _rName ) { }
        bool operator()( const css::beans::Property& _rProp ) const
        {
            return m_sName == _rProp.Name;
        }
    };


    struct PropertyLessByName
                :public ::std::binary_function  <   css::beans::Property,
                                                    css::beans::Property,
                                                    bool
                                                >
    {
        bool operator() (const css::beans::Property& _rLhs, const css::beans::Property& _rRhs) const
        {
            return _rLhs.Name < _rRhs.Name;
        }
    };


    struct TypeLessByName
                :public ::std::binary_function  <   css::uno::Type,
                                                    css::uno::Type,
                                                    bool
                                                >
    {
        bool operator() (const css::uno::Type& _rLhs, const css::uno::Type& _rRhs) const
        {
            return _rLhs.getTypeName() < _rRhs.getTypeName();
        }
    };


    typedef ::std::set< css::beans::Property, PropertyLessByName > PropertyBag;


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_FORMBROWSERTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
