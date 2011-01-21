/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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
#ifndef XMLOFF_COLORPROPERTYSET_HXX
#define XMLOFF_COLORPROPERTYSET_HXX

#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>

namespace oox
{
namespace drawingml
{

class ColorPropertySet : public ::cppu::WeakImplHelper2<
        ::com::sun::star::beans::XPropertySet,
        ::com::sun::star::beans::XPropertyState >
{
public:
    // if bFillColor == false, the color is a LineColor
    explicit ColorPropertySet( sal_Int32 nColor, bool bFillColor = true );
    virtual ~ColorPropertySet();

    void setColor( sal_Int32 nColor );
    sal_Int32 getColor();

protected:
    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue(
        const ::rtl::OUString& aPropertyName,
        const ::com::sun::star::uno::Any& aValue )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
        const ::rtl::OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener(
        const ::rtl::OUString& PropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener(
        const ::rtl::OUString& PropertyName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XPropertyState ____
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState(
        const ::rtl::OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault(
        const ::rtl::OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault(
        const ::rtl::OUString& aPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySetInfo > m_xInfo;
    ::rtl::OUString  m_aColorPropName;
    sal_Int32        m_nColor;
    bool             m_bIsFillColor;
    sal_Int32        m_nDefaultColor;
};

} //  namespace chart
} //  namespace xmloff

// XMLOFF_COLORPROPERTYSET_HXX
#endif
