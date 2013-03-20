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
#if 1

#include "WrappedPropertySet.hxx"
#include "ServiceMacros.hxx"
#include "DiagramHelper.hxx"

#ifndef INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_13
#define INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_13
#define COMPHELPER_IMPLBASE_INTERFACE_NUMBER 13
#include "comphelper/implbase_var.hxx"
#endif

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
#include <boost/shared_ptr.hpp>

namespace chart
{
namespace wrapper
{

class Chart2ModelContact;

class DiagramWrapper : public ::comphelper::ImplInheritanceHelper13<
                      WrappedPropertySet
                     , ::com::sun::star::chart::XDiagram
                     , ::com::sun::star::chart::XAxisSupplier
                     , ::com::sun::star::chart::XAxisZSupplier
                     , ::com::sun::star::chart::XTwoAxisXSupplier   //  : XAxisXSupplier
                     , ::com::sun::star::chart::XTwoAxisYSupplier   //  : XAxisYSupplier
                     , ::com::sun::star::chart::XStatisticDisplay
                     , ::com::sun::star::chart::X3DDisplay
                     , ::com::sun::star::chart::X3DDefaultSetter
                     , ::com::sun::star::lang::XServiceInfo
                     , ::com::sun::star::lang::XComponent
                     , ::com::sun::star::chart::XDiagramPositioning
                     , ::com::sun::star::chart2::XDiagramProvider
                     , ::com::sun::star::chart::XSecondAxisTitleSupplier
                    >
{
public:
    DiagramWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~DiagramWrapper();

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::lang::XEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::lang::XEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XDiagram ____
    virtual ::rtl::OUString SAL_CALL getDiagramType()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getDataRowProperties( sal_Int32 nRow )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getDataPointProperties( sal_Int32 nCol, sal_Int32 nRow )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XShape (base of XDiagram) ____
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize )
        throw (::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XShapeDescriptor (base of XShape) ____
    virtual ::rtl::OUString SAL_CALL getShapeType()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XAxisSupplier ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XAxis > SAL_CALL getAxis( sal_Int32 nDimensionIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XAxis > SAL_CALL getSecondaryAxis( sal_Int32 nDimensionIndex )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XAxisZSupplier ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getZAxisTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getZMainGrid()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getZHelpGrid()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getZAxis()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XTwoAxisXSupplier ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getSecondaryXAxis()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XAxisXSupplier (base of XTwoAxisXSupplier) ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getXAxisTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getXAxis()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getXMainGrid()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getXHelpGrid()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XTwoAxisYSupplier ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getSecondaryYAxis()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XAxisYSupplier (base of XTwoAxisYSupplier) ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getYAxisTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getYAxis()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getYHelpGrid()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getYMainGrid()
        throw (::com::sun::star::uno::RuntimeException);

   // ____ XSecondAxisTitleSupplier ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getSecondXAxisTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getSecondYAxisTitle()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XStatisticDisplay ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getUpBar()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getDownBar()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getMinMaxLine()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ X3DDisplay ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getWall()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getFloor()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ X3DDefaultSetter ____
    virtual void SAL_CALL set3DSettingsToDefault() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultRotation() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultIllumination() throw (::com::sun::star::uno::RuntimeException);

    // ____ XDiagramPositioning ____
    virtual void SAL_CALL setAutomaticDiagramPositioning(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isAutomaticDiagramPositioning(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDiagramPositionExcludingAxes( const ::com::sun::star::awt::Rectangle& PositionRect ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isExcludingDiagramPositioning(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL calculateDiagramPositionExcludingAxes(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDiagramPositionIncludingAxes( const ::com::sun::star::awt::Rectangle& PositionRect ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL calculateDiagramPositionIncludingAxes(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDiagramPositionIncludingAxesAndAxisTitles( const ::com::sun::star::awt::Rectangle& PositionRect ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL calculateDiagramPositionIncludingAxesAndAxisTitles(  ) throw (::com::sun::star::uno::RuntimeException);

    // ____ XDiagramProvider ____
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > SAL_CALL getDiagram()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDiagram( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::chart2::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException);

protected:
    // ____ WrappedPropertySet ____
    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& getPropertySequence();
    virtual const std::vector< WrappedProperty* > createWrappedProperties();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getInnerPropertySet();

private:
    void updateFromModel();

    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::cppu::OInterfaceContainerHelper           m_aEventListenerContainer;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XAxis >        m_xXAxis;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XAxis >        m_xYAxis;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XAxis >        m_xZAxis;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XAxis >        m_xSecondXAxis;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XAxis >        m_xSecondYAxis;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > m_xWall;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > m_xFloor;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > m_xMinMaxLineWrapper;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > m_xUpBarWrapper;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > m_xDownBarWrapper;
};

} //  namespace wrapper
} //  namespace chart

// CHART_DIAGRAMWRAPPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
