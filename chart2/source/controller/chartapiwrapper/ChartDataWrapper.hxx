/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartDataWrapper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:00:24 $
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
#ifndef CHART_CHARTDATAWRAPPER_HXX
#define CHART_CHARTDATAWRAPPER_HXX

#include "ServiceMacros.hxx"

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDATAARRAY_HPP_
#include <com/sun/star/chart/XChartDataArray.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

namespace chart
{
namespace wrapper
{

class ChartDataWrapper : public
    ::cppu::WeakImplHelper3<
    com::sun::star::chart::XChartDataArray,
    com::sun::star::lang::XServiceInfo,
    com::sun::star::lang::XEventListener >
{
public:
    ChartDataWrapper( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::chart2::XChartDocument > & xModel,
                  const ::com::sun::star::uno::Reference<
                      ::com::sun::star::uno::XComponentContext > & xContext,
                  ::osl::Mutex & rMutex );
    virtual ~ChartDataWrapper();

    ::osl::Mutex & GetMutex() const;

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

protected:
    // ____ XChartDataArray ____
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence<
        double > > SAL_CALL getData()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence<
                                   ::com::sun::star::uno::Sequence<
                                   double > >& aData )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::rtl::OUString > SAL_CALL getRowDescriptions()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowDescriptions( const ::com::sun::star::uno::Sequence<
                                              ::rtl::OUString >& aRowDescriptions )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::rtl::OUString > SAL_CALL getColumnDescriptions()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setColumnDescriptions( const ::com::sun::star::uno::Sequence<
                                                 ::rtl::OUString >& aColumnDescriptions )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XChartData (base of XChartDataArray) ____
    virtual void SAL_CALL addChartDataChangeEventListener( const ::com::sun::star::uno::Reference<
                                                           ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChartDataChangeEventListener( const ::com::sun::star::uno::Reference<
                                                              ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getNotANumber()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);


    void fireChartDataChangeEvent( ::com::sun::star::chart::ChartDataChangeEvent& aEvent );

private:
    mutable ::osl::Mutex &    m_rMutex;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >
                        m_xContext;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartDocument >
                        m_xChartDoc;

    ::cppu::OInterfaceContainerHelper
                        m_aEventListenerContainer;
};

} //  namespace wrapper
} //  namespace chart

// CHART_CHARTDATAWRAPPER_HXX
#endif
