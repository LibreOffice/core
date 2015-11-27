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
    css::chart2::XDiagram,
    css::lang::XServiceInfo,
    css::chart2::XCoordinateSystemContainer,
    css::chart2::XTitled,
    css::chart::X3DDefaultSetter,
    css::util::XModifyBroadcaster,
    css::util::XModifyListener,
    css::util::XCloneable >
    Diagram_Base;
}

class Diagram :
    public MutexContainer,
    public impl::Diagram_Base,
    public ::property::OPropertySet
{
public:
    Diagram( css::uno::Reference< css::uno::XComponentContext > const & xContext );
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
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(css::beans::UnknownPropertyException) override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XFastPropertySet ____
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const css::uno::Any& rValue )
        throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    virtual void SAL_CALL getFastPropertyValue(
        css::uno::Any& rValue, sal_Int32 nHandle ) const override;

    // ____ XDiagram ____
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getWall()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getFloor()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::chart2::XLegend > SAL_CALL getLegend()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLegend( const css::uno::Reference<
                                     css::chart2::XLegend >& xLegend )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::chart2::XColorScheme > SAL_CALL getDefaultColorScheme()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultColorScheme(
        const css::uno::Reference< css::chart2::XColorScheme >& xColorScheme )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDiagramData(
        const css::uno::Reference< css::chart2::data::XDataSource >& xDataSource,
        const css::uno::Sequence< css::beans::PropertyValue >& aArguments )
            throw (css::uno::RuntimeException, std::exception) override;

    // ____ XCoordinateSystemContainer ____
    virtual void SAL_CALL addCoordinateSystem(
        const css::uno::Reference< css::chart2::XCoordinateSystem >& aCoordSys )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeCoordinateSystem(
        const css::uno::Reference< css::chart2::XCoordinateSystem >& aCoordSys )
        throw (css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::XCoordinateSystem > > SAL_CALL getCoordinateSystems()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCoordinateSystems(
        const css::uno::Sequence< css::uno::Reference< css::chart2::XCoordinateSystem > >& aCoordinateSystems )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) override;

    // ____ XTitled ____
    virtual css::uno::Reference<
        css::chart2::XTitle > SAL_CALL getTitleObject()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTitleObject( const css::uno::Reference<
                                          css::chart2::XTitle >& Title )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ X3DDefaultSetter ____
    virtual void SAL_CALL set3DSettingsToDefault() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultRotation() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultIllumination() throw (css::uno::RuntimeException, std::exception) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;
    using OPropertySet::disposing;

    void fireModifyEvent();

private:
     css::uno::Reference< css::uno::XComponentContext >                m_xContext;

    typedef
        ::std::vector< css::uno::Reference< css::chart2::XCoordinateSystem > >
        tCoordinateSystemContainerType;

    tCoordinateSystemContainerType m_aCoordSystems;

    css::uno::Reference< css::beans::XPropertySet >
                        m_xWall;

    css::uno::Reference< css::beans::XPropertySet >
                        m_xFloor;

    css::uno::Reference< css::chart2::XTitle >
                        m_xTitle;

    css::uno::Reference< css::chart2::XLegend >
                        m_xLegend;

    css::uno::Reference< css::chart2::XColorScheme >
                        m_xColorScheme;

    css::uno::Reference< css::util::XModifyListener > m_xModifyEventForwarder;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_INC_DIAGRAM_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
