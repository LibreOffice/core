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
#ifndef CHART_CHARTDOCUMENTWRAPPER_HXX
#define CHART_CHARTDOCUMENTWRAPPER_HXX

#include "WrappedPropertySet.hxx"
#include "ServiceMacros.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <cppuhelper/implbase5.hxx>
#include <osl/mutex.hxx>
#include <unotools/eventlisteneradapter.hxx>
#include <comphelper/uno3.hxx>

#include <boost/shared_ptr.hpp>

namespace chart
{

namespace wrapper
{

class Chart2ModelContact;

class ChartDocumentWrapper_Base : public ::cppu::ImplInheritanceHelper5
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
    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(ChartDocumentWrapper)

    void setAddIn( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshable >& xAddIn );
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshable > getAddIn() const;

    void setUpdateAddIn( sal_Bool bUpdateAddIn );
    sal_Bool getUpdateAddIn() const;

    void setBaseDiagram( const rtl::OUString& rBaseDiagram );
    rtl::OUString getBaseDiagram() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > getAdditionalShapes() const;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage > impl_getDrawPage() const
        throw (::com::sun::star::uno::RuntimeException);

protected:

    // ____ chart::XChartDocument ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getSubTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getLegend()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getArea()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XDiagram > SAL_CALL getDiagram()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDiagram( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::chart::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart::XChartData > SAL_CALL getData()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL attachData( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::chart::XChartData >& xData )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModel ____
    virtual sal_Bool SAL_CALL attachResource( const ::rtl::OUString& URL, const ::com::sun::star::uno::Sequence<
                                              ::com::sun::star::beans::PropertyValue >& Arguments )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getURL()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue > SAL_CALL getArgs()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL connectController( const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::frame::XController >& Controller )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disconnectController( const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::frame::XController >& Controller )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL lockControllers()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unlockControllers()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasControllersLocked()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XController > SAL_CALL getCurrentController()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCurrentController( const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::frame::XController >& Controller )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > SAL_CALL getCurrentSelection()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::lang::XEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::lang::XEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XInterface (for new interfaces) ____
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ ::utl::OEventListenerAdapter ____
    virtual void _disposing( const ::com::sun::star::lang::EventObject& rSource );

    // ____ XDrawPageSupplier ____
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage > SAL_CALL getDrawPage()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XMultiServiceFactory ____
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier )
        throw (::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments(
                const ::rtl::OUString& ServiceSpecifier,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments )
        throw (::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
            ::rtl::OUString > SAL_CALL getAvailableServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XAggregation ____
    virtual void SAL_CALL setDelegator(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface >& rDelegator )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ WrappedPropertySet ____
    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& getPropertySequence();
    virtual const std::vector< WrappedProperty* > createWrappedProperties();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getInnerPropertySet();

private: //methods
    void impl_resetAddIn();

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   m_xDelegator;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   m_xTitle;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   m_xSubTitle;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   m_xLegend;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartData > m_xChartData;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram >   m_xDiagram;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xArea;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshable > m_xAddIn;
    rtl::OUString   m_aBaseDiagram;
    sal_Bool        m_bUpdateAddIn;

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
