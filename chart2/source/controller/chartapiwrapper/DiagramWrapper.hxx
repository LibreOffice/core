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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_DIAGRAMWRAPPER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_DIAGRAMWRAPPER_HXX

#include "WrappedPropertySet.hxx"
#include "DiagramHelper.hxx"

#include <comphelper/uno3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>
#include <com/sun/star/chart2/XDiagramProvider.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#include <com/sun/star/chart2/XChartTypeManager.hpp>
#include <com/sun/star/chart/XDiagram.hpp>
#include <com/sun/star/chart/XAxisSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/chart/XSecondAxisTitleSupplier.hpp>

#include <com/sun/star/chart/X3DDefaultSetter.hpp>
#include <memory>

namespace chart
{
namespace wrapper
{

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
    explicit DiagramWrapper( std::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~DiagramWrapper();

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference<
                                            css::lang::XEventListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference<
                                               css::lang::XEventListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XDiagram ____
    virtual OUString SAL_CALL getDiagramType()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getDataRowProperties( sal_Int32 nRow )
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getDataPointProperties( sal_Int32 nCol, sal_Int32 nRow )
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException, std::exception) override;

    // ____ XShape (base of XDiagram) ____
    virtual css::awt::Point SAL_CALL getPosition()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize )
        throw (css::beans::PropertyVetoException,
               css::uno::RuntimeException, std::exception) override;

    // ____ XShapeDescriptor (base of XShape) ____
    virtual OUString SAL_CALL getShapeType()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XAxisSupplier ____
    virtual css::uno::Reference<
        css::chart::XAxis > SAL_CALL getAxis( sal_Int32 nDimensionIndex )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::chart::XAxis > SAL_CALL getSecondaryAxis( sal_Int32 nDimensionIndex )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XAxisZSupplier ____
    virtual css::uno::Reference<
        css::drawing::XShape > SAL_CALL getZAxisTitle()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getZMainGrid()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getZHelpGrid()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getZAxis()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XTwoAxisXSupplier ____
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getSecondaryXAxis()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XAxisXSupplier (base of XTwoAxisXSupplier) ____
    virtual css::uno::Reference<
        css::drawing::XShape > SAL_CALL getXAxisTitle()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getXAxis()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getXMainGrid()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getXHelpGrid()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XTwoAxisYSupplier ____
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getSecondaryYAxis()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XAxisYSupplier (base of XTwoAxisYSupplier) ____
    virtual css::uno::Reference<
        css::drawing::XShape > SAL_CALL getYAxisTitle()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getYAxis()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getYHelpGrid()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getYMainGrid()
        throw (css::uno::RuntimeException, std::exception) override;

   // ____ XSecondAxisTitleSupplier ____
    virtual css::uno::Reference<
        css::drawing::XShape > SAL_CALL getSecondXAxisTitle()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::drawing::XShape > SAL_CALL getSecondYAxisTitle()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XStatisticDisplay ____
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getUpBar()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getDownBar()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getMinMaxLine()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ X3DDisplay ____
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getWall()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::beans::XPropertySet > SAL_CALL getFloor()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ X3DDefaultSetter ____
    virtual void SAL_CALL set3DSettingsToDefault() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultRotation() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultIllumination() throw (css::uno::RuntimeException, std::exception) override;

    // ____ XDiagramPositioning ____
    virtual void SAL_CALL setAutomaticDiagramPositioning(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAutomaticDiagramPositioning(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDiagramPositionExcludingAxes( const css::awt::Rectangle& PositionRect ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isExcludingDiagramPositioning(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL calculateDiagramPositionExcludingAxes(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDiagramPositionIncludingAxes( const css::awt::Rectangle& PositionRect ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL calculateDiagramPositionIncludingAxes(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDiagramPositionIncludingAxesAndAxisTitles( const css::awt::Rectangle& PositionRect ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL calculateDiagramPositionIncludingAxesAndAxisTitles(  ) throw (css::uno::RuntimeException, std::exception) override;

    // ____ XDiagramProvider ____
    virtual css::uno::Reference< css::chart2::XDiagram > SAL_CALL getDiagram()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDiagram( const css::uno::Reference< css::chart2::XDiagram >& xDiagram )
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    // ____ WrappedPropertySet ____
    virtual const css::uno::Sequence< css::beans::Property >& getPropertySequence() override;
    virtual const std::vector< WrappedProperty* > createWrappedProperties() override;
    virtual css::uno::Reference< css::beans::XPropertySet > getInnerPropertySet() override;

private:
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::comphelper::OInterfaceContainerHelper2           m_aEventListenerContainer;

    css::uno::Reference<
        css::chart::XAxis >        m_xXAxis;
    css::uno::Reference<
        css::chart::XAxis >        m_xYAxis;
    css::uno::Reference<
        css::chart::XAxis >        m_xZAxis;
    css::uno::Reference<
        css::chart::XAxis >        m_xSecondXAxis;
    css::uno::Reference<
        css::chart::XAxis >        m_xSecondYAxis;

    css::uno::Reference<
        css::beans::XPropertySet > m_xWall;
    css::uno::Reference<
        css::beans::XPropertySet > m_xFloor;

    css::uno::Reference<
        css::beans::XPropertySet > m_xMinMaxLineWrapper;
    css::uno::Reference<
        css::beans::XPropertySet > m_xUpBarWrapper;
    css::uno::Reference<
        css::beans::XPropertySet > m_xDownBarWrapper;
};

} //  namespace wrapper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_DIAGRAMWRAPPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
