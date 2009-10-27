/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChartData.hxx,v $
 * $Revision: 1.3 $
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
#ifndef CHART2_CHARTDATA_HXX
#define CHART2_CHARTDATA_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

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
        external data provider this reference must be set to 0
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider >
        m_xInternalDataProvider;
};

} //  namespace chart

// CHART2_CHARTDATA_HXX
#endif
