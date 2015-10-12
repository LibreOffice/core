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

#ifndef XMLOFF_COLORPROPERTYSET_HXX
#define XMLOFF_COLORPROPERTYSET_HXX

// FIXME? this file is identical to xmloff/source/chart/ColorPropertySet.hxx

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>

namespace oox
{
namespace drawingml
{

class ColorPropertySet : public ::cppu::WeakImplHelper<
        ::com::sun::star::beans::XPropertySet,
        ::com::sun::star::beans::XPropertyState >
{
public:
    // if bFillColor == false, the color is a LineColor
    explicit ColorPropertySet( sal_Int32 nColor, bool bFillColor = true );
    virtual ~ColorPropertySet();

protected:
    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(
        const OUString& aPropertyName,
        const ::com::sun::star::uno::Any& aValue )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
        const OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener(
        const OUString& aPropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener(
        const OUString& aPropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener(
        const OUString& PropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener(
        const OUString& PropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XPropertyState ____
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState(
        const OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates(
        const ::com::sun::star::uno::Sequence< OUString >& aPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault(
        const OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault(
        const OUString& aPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySetInfo > m_xInfo;
    OUString  m_aColorPropName;
    sal_Int32        m_nColor;
    bool             m_bIsFillColor;
    sal_Int32        m_nDefaultColor;
};

} //  namespace chart
} //  namespace xmloff

// XMLOFF_COLORPROPERTYSET_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
