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

#ifndef INCLUDED_COMPHELPER_PROPERTYCONTAINER_HXX
#define INCLUDED_COMPHELPER_PROPERTYCONTAINER_HXX

#include <comphelper/propertycontainerhelper.hxx>
#include <cppuhelper/propshlp.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{


/** a OPropertySetHelper implementation which is just a simple container for properties represented
    by class members, usually in a derived class.
    <BR>
    A restriction of this class is that no value conversions are made on a setPropertyValue call. Though
    the base class supports this with the convertFastPropertyValue method, the OPropertyContainer accepts only
    values which already have the correct type, it's unable to convert, for instance, a long to a short.
*/
class COMPHELPER_DLLPUBLIC OPropertyContainer
            :public cppu::OPropertySetHelper
            ,public OPropertyContainerHelper
{
public:
    // this dtor is needed otherwise we can get a wrong delete operator
    virtual ~OPropertyContainer();

protected:
    OPropertyContainer(::cppu::OBroadcastHelper& _rBHelper);

    /// for scripting : the types of the interfaces supported by this class
    static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > getBaseTypes() throw (::com::sun::star::uno::RuntimeException, std::exception);

// OPropertySetHelper overridables
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                            ::com::sun::star::uno::Any & rConvertedValue,
                            ::com::sun::star::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const ::com::sun::star::uno::Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException) override;

    virtual void SAL_CALL   setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue
                            )
                            throw (::com::sun::star::uno::Exception,
                                   std::exception) override;

    using OPropertySetHelper::getFastPropertyValue;
    virtual void SAL_CALL getFastPropertyValue(
                                ::com::sun::star::uno::Any& rValue,
                                sal_Int32 nHandle
                                     ) const override;

    // disambiguate a base class method (XFastPropertySet)
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
};


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_PROPERTYCONTAINER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
