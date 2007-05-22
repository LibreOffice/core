/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartData.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:32:13 $
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
#ifndef CHART2_CHARTDATA_HXX
#define CHART2_CHARTDATA_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XLABELEDDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif

#include <memory>

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
    }
    namespace embed {
        class XStorage;
        class XEmbeddedObject;
        class XEmbeddedClient;
    }
    namespace chart2 {
        namespace data {
            class XDataProvider;
        }
    }
    namespace util {
        class XCloseBroadcaster;
    }
}}}

namespace chart
{

class InternalDataProvider;

class ChartData
{
public:
    explicit ChartData(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    ~ChartData();

    void setDataProvider(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataProvider > & xDataProvider ) throw();

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider >
        getDataProvider() const throw();

    /** @return </TRUE>, if a new internal data provider has been created
     */
    bool createInternalData(
        bool bCloneOldData,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > & xChartDoc ) throw();

    bool hasInternalData() const;

    /** only works if currently an internal data provider is set
     */
    bool createDefaultData() throw();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider >
        m_xDataProvider;
    /** is only valid if m_xDataProvider is set. If m_xDataProvider is set to an
        external data provider this pointer must be set to 0
    */
    InternalDataProvider * m_pInternalDataProvider;

//     ::com::sun::star::uno::Reference<
//             ::com::sun::star::embed::XEmbeddedClient > m_xInternalData;
//     /// is only valid if m_xInternalData exists
//     InternalData *                                     m_pInternalData;
};

} //  namespace chart

// CHART2_CHARTDATA_HXX
#endif
