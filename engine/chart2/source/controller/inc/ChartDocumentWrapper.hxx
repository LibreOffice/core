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
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <unotools/eventlisteneradapter.hxx>
#include <rtl/ref.hxx>
#include <svx/unopage.hxx>
#include <memory>

namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::util { class XRefreshable; }
namespace chart { class ChartView; }

namespace chart::wrapper
{
class DiagramWrapper;
class LegendWrapper;
class TitleWrapper;
class Chart2ModelContact;

class ChartDocumentWrapper_Base : public ::cppu::ImplInheritanceHelper
                                < WrappedPropertySet
                                , css::chart::XChartDocument
                                , css::drawing::XDrawPageSupplier
                                , css::lang::XMultiServiceFactory
                                , css::lang::XServiceInfo
                                , css::uno::XAggregation
                                >
{
};

class ChartDocumentWrapper final : public ChartDocumentWrapper_Base
                           , public ::utl::OEventListenerAdapter
{
public:
    explicit ChartDocumentWrapper( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    virtual ~ChartDocumentWrapper() override;

    /// XServiceInfo declarations
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    void setAddIn( const css::uno::Reference< css::util::XRefreshable >& xAddIn );
    const css::uno::Reference< css::util::XRefreshable >& getAddIn() const { return m_xAddIn;}

    void setUpdateAddIn( bool bUpdateAddIn );
    bool getUpdateAddIn() const { return m_bUpdateAddIn;}

    void setBaseDiagram( const OUString& rBaseDiagram );
    const OUString& getBaseDiagram() const { return m_aBaseDiagram;}

    css::uno::Reference< css::drawing::XShapes > getAdditionalShapes() const;

    /// @throws css::uno::RuntimeException
    rtl::Reference<SvxDrawPage> impl_getDrawPage() const;

    // ____ chart::XChartDocument ____
    virtual css::uno::Reference< css::drawing::XShape > getTitle() override;
    virtual css::uno::Reference< css::drawing::XShape > getSubTitle() override;
    virtual css::uno::Reference< css::drawing::XShape > getLegend() override;
    virtual css::uno::Reference< css::beans::XPropertySet > getArea() override;
    virtual css::uno::Reference< css::chart::XDiagram > getDiagram() override;
    virtual void setDiagram( const css::uno::Reference<
                                      css::chart::XDiagram >& xDiagram ) override;
    virtual css::uno::Reference< css::chart::XChartData > getData() override;
    virtual void attachData( const css::uno::Reference<
                                      css::chart::XChartData >& xData ) override;

    // ____ XModel ____
    virtual bool attachResource( const OUString& URL,
                                              const cpo::uno::Sequence< css::beans::PropertyValue >& Arguments ) override;
    virtual OUString getURL() override;
    virtual cpo::uno::Sequence<
        css::beans::PropertyValue > getArgs() override;
    virtual void connectController( const css::uno::Reference<
                                             css::frame::XController >& Controller ) override;
    virtual void disconnectController( const css::uno::Reference<
                                                css::frame::XController >& Controller ) override;
    virtual void lockControllers() override;
    virtual void unlockControllers() override;
    virtual bool hasControllersLocked() override;
    virtual css::uno::Reference<
        css::frame::XController > getCurrentController() override;
    virtual void setCurrentController( const css::uno::Reference< css::frame::XController >& Controller ) override;
    virtual css::uno::Reference<css::uno::XInterface > getCurrentSelection() override;

    // ____ XComponent ____
    virtual void dispose() override;
    virtual void addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // ____ XInterface (for new interfaces) ____
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type& aType ) override;

    // ____ ::utl::OEventListenerAdapter ____
    virtual void _disposing( const css::lang::EventObject& rSource ) override;

    // ____ XDrawPageSupplier ____
    virtual css::uno::Reference< css::drawing::XDrawPage > getDrawPage() override;

    // ____ XMultiServiceFactory ____
    virtual css::uno::Reference< css::uno::XInterface > createInstance( const OUString& aServiceSpecifier ) override;
    virtual css::uno::Reference< css::uno::XInterface > createInstanceWithArguments(
                const OUString& ServiceSpecifier,
                const cpo::uno::Sequence< cpo::uno::Any >& Arguments ) override;
    virtual cpo::uno::Sequence< OUString > getAvailableServiceNames() override;

    // ____ XAggregation ____
    virtual void setDelegator(
        const css::uno::Reference< css::uno::XInterface >& rDelegator ) override;
    virtual cpo::uno::Any queryAggregation( const cpo::uno::Type& aType ) override;

    // ____ WrappedPropertySet ____
    virtual const cpo::uno::Sequence< css::beans::Property >& getPropertySequence() override;
    virtual std::vector< std::unique_ptr<WrappedProperty> > createWrappedProperties() override;
    virtual css::uno::Reference< css::beans::XPropertySet > getInnerPropertySet() override;

    // ____ XPropertySet ____
    virtual void setPropertyValue(const OUString& rPropertyName,
                                           const cpo::uno::Any& rValue) override;

private: //methods
    void impl_resetAddIn();

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;

    css::uno::Reference< css::uno::XInterface >   m_xDelegator;

    rtl::Reference< TitleWrapper >   m_xTitle;
    rtl::Reference< TitleWrapper >   m_xSubTitle;
    rtl::Reference< LegendWrapper >   m_xLegend;
    css::uno::Reference< css::chart::XChartData > m_xChartData;
    rtl::Reference< DiagramWrapper >   m_xDiagram;
    css::uno::Reference< css::beans::XPropertySet > m_xArea;

    css::uno::Reference< css::util::XRefreshable > m_xAddIn;
    OUString   m_aBaseDiagram;
    bool        m_bUpdateAddIn;

    rtl::Reference< ChartView >                   m_xChartView;
    css::uno::Reference< css::lang::XMultiServiceFactory>
                                                  m_xShapeFactory;

    bool                                          m_bIsDisposed;
};

} //  namespace chart::wrapper

// CHART_CHARTDOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
