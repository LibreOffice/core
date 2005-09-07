/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartDocumentWrapper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:21:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART_CHARTDOCUMENTWRAPPER_HXX
#define CHART_CHARTDOCUMENTWRAPPER_HXX

#include "MutexContainer.hxx"
#include "OPropertySet.hxx"
#include "ServiceMacros.hxx"

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XAGGREGATION_HPP_
#include <com/sun/star/uno/XAggregation.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _UNOTOOLS_EVENTLISTENERADAPTER_HXX_
#include <unotools/eventlisteneradapter.hxx>
#endif

namespace com { namespace sun { namespace star {
namespace chart2
{
    class XChartDocument;
}
}}}

namespace com { namespace sun { namespace star {
namespace drawing
{
    class XShape;
}
namespace chart
{
    class XDiagram;
}
}}}



namespace chart
{
namespace wrapper
{

namespace impl
{
typedef
    ::cppu::WeakImplHelper6<
        ::com::sun::star::chart::XChartDocument,
        ::com::sun::star::util::XNumberFormatsSupplier,
        ::com::sun::star::drawing::XDrawPageSupplier,
        ::com::sun::star::lang::XMultiServiceFactory,
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::uno::XAggregation >
    ChartDocumentWrapper_Base;
}

class ChartDocumentWrapper :
        public impl::ChartDocumentWrapper_Base,
        public ::utl::OEventListenerAdapter
{
public:
    ChartDocumentWrapper(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~ChartDocumentWrapper();

    ::osl::Mutex & GetMutex() const;

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( ChartDocumentWrapper )

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

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
    virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource );

    // ____ XNumberFormatsSupplier ____
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > SAL_CALL getNumberFormatSettings()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XNumberFormats > SAL_CALL getNumberFormats()
        throw (::com::sun::star::uno::RuntimeException);

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

private:
    mutable ::osl::Mutex      m_aMutex;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >
                        m_xContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        m_xDelegator;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartDocument >
                        m_xChartDoc;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel >
                        m_xModel;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        m_xTitle;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        m_xSubTitle;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        m_xLegend;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartData >
                        m_xChartData;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram >
                        m_xDiagram;

    bool                m_bIsDisposed;
};

} //  namespace wrapper
} //  namespace chart

// CHART_CHARTDOCUMENT_HXX
#endif
