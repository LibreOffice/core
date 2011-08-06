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
#ifndef CHART_DIAGRAM_HXX
#define CHART_DIAGRAM_HXX

#include "OPropertySet.hxx"
#include "MutexContainer.hxx"
#include <cppuhelper/implbase8.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart/X3DDefaultSetter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include "ServiceMacros.hxx"

#include <map>
#include <vector>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper8<
    ::com::sun::star::chart2::XDiagram,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::chart2::XCoordinateSystemContainer,
    ::com::sun::star::chart2::XTitled,
    ::com::sun::star::chart::X3DDefaultSetter,
    ::com::sun::star::util::XModifyBroadcaster,
    ::com::sun::star::util::XModifyListener,
    ::com::sun::star::util::XCloneable >
    Diagram_Base;
}

class Diagram :
    public MutexContainer,
    public impl::Diagram_Base,
    public ::property::OPropertySet
{
public:
    Diagram( ::com::sun::star::uno::Reference<
             ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~Diagram();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( Diagram )

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    explicit Diagram( const Diagram & rOther );

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XFastPropertySet ____
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    virtual void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

    // ____ XDiagram ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getWall()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getFloor()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XLegend > SAL_CALL getLegend()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLegend( const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::chart2::XLegend >& xLegend )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XColorScheme > SAL_CALL getDefaultColorScheme()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultColorScheme(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XColorScheme >& xColorScheme )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDiagramData(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >& xDataSource,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
            throw (::com::sun::star::uno::RuntimeException);

    // ____ XCoordinateSystemContainer ____
    virtual void SAL_CALL addCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >& aCoordSys )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >& aCoordSys )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XCoordinateSystem > > SAL_CALL getCoordinateSystems()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCoordinateSystems(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XCoordinateSystem > >& aCoordinateSystems )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XTitled ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTitle > SAL_CALL getTitleObject()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitleObject( const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::chart2::XTitle >& Title )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ X3DDefaultSetter ____
    virtual void SAL_CALL set3DSettingsToDefault() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultRotation() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultIllumination() throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent();
    using OPropertySet::disposing;

    void fireModifyEvent();

private:
     ::com::sun::star::uno::Reference<
         ::com::sun::star::uno::XComponentContext >                m_xContext;

    typedef
        ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem > >
        tCoordinateSystemContainerType;

    tCoordinateSystemContainerType m_aCoordSystems;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xWall;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xFloor;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTitle >
                        m_xTitle;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XLegend >
                        m_xLegend;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XColorScheme >
                        m_xColorScheme;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyEventForwarder;
};

} //  namespace chart

// CHART_DIAGRAM_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
