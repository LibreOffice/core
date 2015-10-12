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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_INC_DIAGRAM_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_INC_DIAGRAM_HXX

#include "OPropertySet.hxx"
#include "MutexContainer.hxx"
#include <cppuhelper/implbase.hxx>
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

#include <map>
#include <vector>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper<
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

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    explicit Diagram( const Diagram & rOther );

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException) override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XFastPropertySet ____
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    virtual void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const override;

    // ____ XDiagram ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getWall()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getFloor()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XLegend > SAL_CALL getLegend()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLegend( const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::chart2::XLegend >& xLegend )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XColorScheme > SAL_CALL getDefaultColorScheme()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultColorScheme(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XColorScheme >& xColorScheme )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDiagramData(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >& xDataSource,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XCoordinateSystemContainer ____
    virtual void SAL_CALL addCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >& aCoordSys )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >& aCoordSys )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XCoordinateSystem > > SAL_CALL getCoordinateSystems()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCoordinateSystems(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XCoordinateSystem > >& aCoordinateSystems )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XTitled ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTitle > SAL_CALL getTitleObject()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTitleObject( const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::chart2::XTitle >& Title )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ X3DDefaultSetter ____
    virtual void SAL_CALL set3DSettingsToDefault() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultRotation() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultIllumination() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;
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

// INCLUDED_CHART2_SOURCE_MODEL_INC_DIAGRAM_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
