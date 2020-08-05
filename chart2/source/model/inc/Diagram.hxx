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
#pragma once

#include <OPropertySet.hxx>
#include <MutexContainer.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart/X3DDefaultSetter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <vector>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::chart2::data { class XDataSource; }
namespace com::sun::star::uno { class XComponentContext; }

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

class Diagram final :
    public MutexContainer,
    public impl::Diagram_Base,
    public ::property::OPropertySet
{
public:
    Diagram( css::uno::Reference< css::uno::XComponentContext > const & xContext );
    virtual ~Diagram() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

private:
    explicit Diagram( const Diagram & rOther );

    // ____ OPropertySet ____
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;

    // ____ XFastPropertySet ____
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const css::uno::Any& rValue ) override;

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    virtual void SAL_CALL getFastPropertyValue(
        css::uno::Any& rValue, sal_Int32 nHandle ) const override;

    // ____ XDiagram ____
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getWall() override;
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getFloor() override;
    virtual css::uno::Reference< css::chart2::XLegend > SAL_CALL getLegend() override;
    virtual void SAL_CALL setLegend( const css::uno::Reference<
                                     css::chart2::XLegend >& xLegend ) override;
    virtual css::uno::Reference< css::chart2::XColorScheme > SAL_CALL getDefaultColorScheme() override;
    virtual void SAL_CALL setDefaultColorScheme(
        const css::uno::Reference< css::chart2::XColorScheme >& xColorScheme ) override;
    virtual void SAL_CALL setDiagramData(
        const css::uno::Reference< css::chart2::data::XDataSource >& xDataSource,
        const css::uno::Sequence< css::beans::PropertyValue >& aArguments ) override;

    // ____ XCoordinateSystemContainer ____
    virtual void SAL_CALL addCoordinateSystem(
        const css::uno::Reference< css::chart2::XCoordinateSystem >& aCoordSys ) override;
    virtual void SAL_CALL removeCoordinateSystem(
        const css::uno::Reference< css::chart2::XCoordinateSystem >& aCoordSys ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::XCoordinateSystem > > SAL_CALL getCoordinateSystems() override;
    virtual void SAL_CALL setCoordinateSystems(
        const css::uno::Sequence< css::uno::Reference< css::chart2::XCoordinateSystem > >& aCoordinateSystems ) override;

    // ____ XTitled ____
    virtual css::uno::Reference<
        css::chart2::XTitle > SAL_CALL getTitleObject() override;
    virtual void SAL_CALL setTitleObject( const css::uno::Reference<
                                          css::chart2::XTitle >& Title ) override;

    // ____ X3DDefaultSetter ____
    virtual void SAL_CALL set3DSettingsToDefault() override;
    virtual void SAL_CALL setDefaultRotation() override;
    virtual void SAL_CALL setDefaultIllumination() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;
    using OPropertySet::disposing;

    void fireModifyEvent();

     css::uno::Reference< css::uno::XComponentContext >                m_xContext;

    typedef
        std::vector< css::uno::Reference< css::chart2::XCoordinateSystem > >
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
