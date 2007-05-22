/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSource.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:15:18 $
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
#ifndef CHART2_DATASOURCE_HXX
#define CHART2_DATASOURCE_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XDATASOURCE_HPP_
#include <com/sun/star/chart2/data/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XDATASINK_HPP_
#include <com/sun/star/chart2/data/XDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _APPHELPER_SERVICEMACROS_HXX
#include "ServiceMacros.hxx"
#endif

namespace chart
{

class DataSource : public
    ::cppu::WeakImplHelper3<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::chart2::data::XDataSource,
        ::com::sun::star::chart2::data::XDataSink >
{
public:
    explicit DataSource(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    explicit DataSource(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XLabeledDataSequence > > & rSequences );
    explicit DataSource(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataSequence > > & rSequences );
    explicit DataSource(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataSequence > > & rSequences,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataSequence > > & rLabel );

    virtual ~DataSource();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( DataSource )
    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()

protected:
    // ____ XDataSource ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > > SAL_CALL
        getDataSequences()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XDataSink ____
    virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > >& aData )
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence > >
        m_aDataSeq;
};

} //  namespace chart

// CHART2_DATASOURCE_HXX
#endif
