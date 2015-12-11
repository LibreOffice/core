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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_CHARTDOCUMENTWRAPPER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_CHARTDOCUMENTWRAPPER_HXX

#include "WrappedPropertySet.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <unotools/eventlisteneradapter.hxx>
#include <comphelper/uno3.hxx>

#include <memory>

namespace chart
{

namespace wrapper
{

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

class ChartDocumentWrapper : public ChartDocumentWrapper_Base
                           , public ::utl::OEventListenerAdapter
{
public:
    explicit ChartDocumentWrapper( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    virtual ~ChartDocumentWrapper();

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    void setAddIn( const css::uno::Reference< css::util::XRefreshable >& xAddIn );
    const css::uno::Reference< css::util::XRefreshable >& getAddIn() const { return m_xAddIn;}

    void setUpdateAddIn( bool bUpdateAddIn );
    bool getUpdateAddIn() const { return m_bUpdateAddIn;}

    void setBaseDiagram( const OUString& rBaseDiagram );
    const OUString& getBaseDiagram() const { return m_aBaseDiagram;}

    css::uno::Reference< css::drawing::XShapes > getAdditionalShapes() const;

    css::uno::Reference< css::drawing::XDrawPage > impl_getDrawPage() const
        throw (css::uno::RuntimeException);

protected:

    // ____ chart::XChartDocument ____
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL getTitle()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL getSubTitle()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::drawing::XShape > SAL_CALL getLegend()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getArea()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::chart::XDiagram > SAL_CALL getDiagram()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDiagram( const css::uno::Reference<
                                      css::chart::XDiagram >& xDiagram )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::chart::XChartData > SAL_CALL getData()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL attachData( const css::uno::Reference<
                                      css::chart::XChartData >& xData )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XModel ____
    virtual sal_Bool SAL_CALL attachResource( const OUString& URL,
                                              const css::uno::Sequence< css::beans::PropertyValue >& Arguments )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getURL()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence<
        css::beans::PropertyValue > SAL_CALL getArgs()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL connectController( const css::uno::Reference<
                                             css::frame::XController >& Controller )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL disconnectController( const css::uno::Reference<
                                                css::frame::XController >& Controller )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL lockControllers()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL unlockControllers()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasControllersLocked()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
        css::frame::XController > SAL_CALL getCurrentController()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCurrentController( const css::uno::Reference< css::frame::XController >& Controller )
        throw (css::container::NoSuchElementException,
               css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<css::uno::XInterface > SAL_CALL getCurrentSelection()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XInterface (for new interfaces) ____
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ ::utl::OEventListenerAdapter ____
    virtual void _disposing( const css::lang::EventObject& rSource ) override;

    // ____ XDrawPageSupplier ____
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getDrawPage()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XMultiServiceFactory ____
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier )
        throw (css::uno::Exception,
               css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments(
                const OUString& ServiceSpecifier,
                const css::uno::Sequence< css::uno::Any >& Arguments )
        throw (css::uno::Exception,
               css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XAggregation ____
    virtual void SAL_CALL setDelegator(
        const css::uno::Reference< css::uno::XInterface >& rDelegator )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& aType )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ WrappedPropertySet ____
    virtual const css::uno::Sequence< css::beans::Property >& getPropertySequence() override;
    virtual const std::vector< WrappedProperty* > createWrappedProperties() override;
    virtual css::uno::Reference< css::beans::XPropertySet > getInnerPropertySet() override;

private: //methods
    void impl_resetAddIn();

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;

    css::uno::Reference< css::uno::XInterface >   m_xDelegator;

    css::uno::Reference< css::drawing::XShape >   m_xTitle;
    css::uno::Reference< css::drawing::XShape >   m_xSubTitle;
    css::uno::Reference< css::drawing::XShape >   m_xLegend;
    css::uno::Reference< css::chart::XChartData > m_xChartData;
    css::uno::Reference< css::chart::XDiagram >   m_xDiagram;
    css::uno::Reference< css::beans::XPropertySet > m_xArea;

    css::uno::Reference< css::util::XRefreshable > m_xAddIn;
    OUString   m_aBaseDiagram;
    bool        m_bUpdateAddIn;

    css::uno::Reference< css::uno::XInterface >   m_xChartView;
    css::uno::Reference< css::lang::XMultiServiceFactory>
                                                  m_xShapeFactory;

    bool                                          m_bIsDisposed;
};

} //  namespace wrapper
} //  namespace chart

// CHART_CHARTDOCUMENT_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
