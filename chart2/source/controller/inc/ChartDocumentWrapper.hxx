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
                                , ::com::sun::star::chart::XChartDocument
                                , ::com::sun::star::drawing::XDrawPageSupplier
                                , ::com::sun::star::lang::XMultiServiceFactory
                                , ::com::sun::star::lang::XServiceInfo
                                , ::com::sun::star::uno::XAggregation
                                >
{
};

class ChartDocumentWrapper : public ChartDocumentWrapper_Base
                           , public ::utl::OEventListenerAdapter
{
public:
    explicit ChartDocumentWrapper(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~ChartDocumentWrapper();

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    void setAddIn( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshable >& xAddIn );
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshable > getAddIn() const { return m_xAddIn;}

    void setUpdateAddIn( bool bUpdateAddIn );
    bool getUpdateAddIn() const { return m_bUpdateAddIn;}

    void setBaseDiagram( const OUString& rBaseDiagram );
    OUString getBaseDiagram() const { return m_aBaseDiagram;}

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > getAdditionalShapes() const;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage > impl_getDrawPage() const
        throw (::com::sun::star::uno::RuntimeException);

protected:

    // ____ chart::XChartDocument ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getTitle()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getSubTitle()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getLegend()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getArea()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XDiagram > SAL_CALL getDiagram()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDiagram( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::chart::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XChartData > SAL_CALL getData()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL attachData( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::chart::XChartData >& xData )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XModel ____
    virtual sal_Bool SAL_CALL attachResource( const OUString& URL, const ::com::sun::star::uno::Sequence<
                                              ::com::sun::star::beans::PropertyValue >& Arguments )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getURL()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue > SAL_CALL getArgs()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL connectController( const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::frame::XController >& Controller )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL disconnectController( const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::frame::XController >& Controller )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL lockControllers()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL unlockControllers()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasControllersLocked()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XController > SAL_CALL getCurrentController()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCurrentController( const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::frame::XController >& Controller )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > SAL_CALL getCurrentSelection()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::lang::XEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::lang::XEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XInterface (for new interfaces) ____
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ ::utl::OEventListenerAdapter ____
    virtual void _disposing( const ::com::sun::star::lang::EventObject& rSource ) SAL_OVERRIDE;

    // ____ XDrawPageSupplier ____
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage > SAL_CALL getDrawPage()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XMultiServiceFactory ____
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier )
        throw (::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments(
                const OUString& ServiceSpecifier,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments )
        throw (::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence<
            OUString > SAL_CALL getAvailableServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XAggregation ____
    virtual void SAL_CALL setDelegator(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface >& rDelegator )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ WrappedPropertySet ____
    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& getPropertySequence() SAL_OVERRIDE;
    virtual const std::vector< WrappedProperty* > createWrappedProperties() SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getInnerPropertySet() SAL_OVERRIDE;

private: //methods
    void impl_resetAddIn();

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   m_xDelegator;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   m_xTitle;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   m_xSubTitle;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   m_xLegend;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartData > m_xChartData;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram >   m_xDiagram;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xArea;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshable > m_xAddIn;
    OUString   m_aBaseDiagram;
    bool        m_bUpdateAddIn;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   m_xChartView;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>
                                                                            m_xShapeFactory;

    bool                                                                    m_bIsDisposed;
};

} //  namespace wrapper
} //  namespace chart

// CHART_CHARTDOCUMENT_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
