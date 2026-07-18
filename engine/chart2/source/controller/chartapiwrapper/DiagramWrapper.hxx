/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <WrappedPropertySet.hxx>

#include <comphelper/interfacecontainer4.hxx>
#include <com/sun/star/chart/XDiagramPositioning.hpp>
#include <com/sun/star/chart2/XDiagramProvider.hpp>
#include <com/sun/star/chart/XDiagram.hpp>
#include <com/sun/star/chart/XAxisSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/chart/XSecondAxisTitleSupplier.hpp>
#include <com/sun/star/chart/X3DDefaultSetter.hpp>
#include <rtl/ref.hxx>
#include <memory>

namespace com::sun::star::lang { class XEventListener; }
namespace chart { class Diagram; }

namespace chart::wrapper
{
class AxisWrapper;
class WallFloorWrapper;
class MinMaxLineWrapper;
class UpDownBarWrapper;
class Chart2ModelContact;

class DiagramWrapper : public cppu::ImplInheritanceHelper<
                      WrappedPropertySet
                     , css::chart::XDiagram
                     , css::chart::XAxisSupplier
                     , css::chart::XAxisZSupplier
                     , css::chart::XTwoAxisXSupplier   //  : XAxisXSupplier
                     , css::chart::XTwoAxisYSupplier   //  : XAxisYSupplier
                     , css::chart::XStatisticDisplay
                     , css::chart::X3DDisplay
                     , css::chart::X3DDefaultSetter
                     , css::lang::XServiceInfo
                     , css::lang::XComponent
                     , css::chart::XDiagramPositioning
                     , css::chart2::XDiagramProvider
                     , css::chart::XSecondAxisTitleSupplier
                    >
{
public:
    explicit DiagramWrapper(std::shared_ptr<Chart2ModelContact> spChart2ModelContact);
    virtual ~DiagramWrapper() override;

    /// XServiceInfo declarations
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // ____ XComponent ____
    virtual void dispose() override;
    virtual void addEventListener( const css::uno::Reference<
                                            css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const css::uno::Reference<
                                               css::lang::XEventListener >& aListener ) override;

    // ____ XDiagram ____
    virtual OUString getDiagramType() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getDataRowProperties( sal_Int32 nRow ) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getDataPointProperties( sal_Int32 nCol, sal_Int32 nRow ) override;

    // ____ XShape (base of XDiagram) ____
    virtual css::awt::Point getPosition() override;
    virtual void setPosition( const css::awt::Point& aPosition ) override;
    virtual css::awt::Size getSize() override;
    virtual void setSize( const css::awt::Size& aSize ) override;

    // ____ XShapeDescriptor (base of XShape) ____
    virtual OUString getShapeType() override;

    // ____ XAxisSupplier ____
    virtual css::uno::Reference<
        css::chart::XAxis > getAxis( sal_Int32 nDimensionIndex ) override;
    virtual css::uno::Reference<
        css::chart::XAxis > getSecondaryAxis( sal_Int32 nDimensionIndex ) override;

    // ____ XAxisZSupplier ____
    virtual css::uno::Reference<
        css::drawing::XShape > getZAxisTitle() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getZMainGrid() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getZHelpGrid() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getZAxis() override;

    // ____ XTwoAxisXSupplier ____
    virtual css::uno::Reference<
        css::beans::XPropertySet > getSecondaryXAxis() override;

    // ____ XAxisXSupplier (base of XTwoAxisXSupplier) ____
    virtual css::uno::Reference<
        css::drawing::XShape > getXAxisTitle() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getXAxis() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getXMainGrid() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getXHelpGrid() override;

    // ____ XTwoAxisYSupplier ____
    virtual css::uno::Reference<
        css::beans::XPropertySet > getSecondaryYAxis() override;

    // ____ XAxisYSupplier (base of XTwoAxisYSupplier) ____
    virtual css::uno::Reference<
        css::drawing::XShape > getYAxisTitle() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getYAxis() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getYHelpGrid() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getYMainGrid() override;

   // ____ XSecondAxisTitleSupplier ____
    virtual css::uno::Reference<
        css::drawing::XShape > getSecondXAxisTitle() override;
    virtual css::uno::Reference<
        css::drawing::XShape > getSecondYAxisTitle() override;

    // ____ XStatisticDisplay ____
    virtual css::uno::Reference<
        css::beans::XPropertySet > getUpBar() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getDownBar() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getMinMaxLine() override;

    // ____ X3DDisplay ____
    virtual css::uno::Reference<
        css::beans::XPropertySet > getWall() override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > getFloor() override;

    // ____ X3DDefaultSetter ____
    virtual void set3DSettingsToDefault() override;
    virtual void setDefaultRotation() override;
    virtual void setDefaultIllumination() override;

    // ____ XDiagramPositioning ____
    virtual void setAutomaticDiagramPositioning(  ) override;
    virtual bool isAutomaticDiagramPositioning(  ) override;
    virtual void setDiagramPositionExcludingAxes( const css::awt::Rectangle& PositionRect ) override;
    virtual bool isExcludingDiagramPositioning(  ) override;
    virtual css::awt::Rectangle calculateDiagramPositionExcludingAxes(  ) override;
    virtual void setDiagramPositionIncludingAxes( const css::awt::Rectangle& PositionRect ) override;
    virtual css::awt::Rectangle calculateDiagramPositionIncludingAxes(  ) override;
    virtual void setDiagramPositionIncludingAxesAndAxisTitles( const css::awt::Rectangle& PositionRect ) override;
    virtual css::awt::Rectangle calculateDiagramPositionIncludingAxesAndAxisTitles(  ) override;

    // ____ XDiagramProvider ____
    virtual css::uno::Reference< css::chart2::XDiagram > getDiagram() override;
    virtual void setDiagram( const css::uno::Reference< css::chart2::XDiagram >& xDiagram ) override;

    rtl::Reference< ::chart::Diagram > getUnderlyingDiagram();

protected:
    // ____ WrappedPropertySet ____
    virtual const cpo::uno::Sequence< css::beans::Property >& getPropertySequence() override;
    virtual std::vector< std::unique_ptr<WrappedProperty> > createWrappedProperties() override;
    virtual css::uno::Reference< css::beans::XPropertySet > getInnerPropertySet() override;

private:
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_aEventListenerContainer;

    rtl::Reference< AxisWrapper >        m_xXAxis;
    rtl::Reference< AxisWrapper >        m_xYAxis;
    rtl::Reference< AxisWrapper >        m_xZAxis;
    rtl::Reference< AxisWrapper >        m_xSecondXAxis;
    rtl::Reference< AxisWrapper >        m_xSecondYAxis;
    rtl::Reference< WallFloorWrapper > m_xWall;
    rtl::Reference< WallFloorWrapper > m_xFloor;
    rtl::Reference< MinMaxLineWrapper > m_xMinMaxLineWrapper;
    rtl::Reference< UpDownBarWrapper > m_xUpBarWrapper;
    rtl::Reference< UpDownBarWrapper > m_xDownBarWrapper;
};

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
