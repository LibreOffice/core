/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccStatisticsObject.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:14:15 $
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
#ifndef _CHART2_ACCSTATISTICSOBJECT_HXX
#define _CHART2_ACCSTATISTICSOBJECT_HXX

#ifndef _CHART2_ACCESSIBLE_CHART_ELEMENT_HXX_
#include "AccessibleChartElement.hxx"
#endif

#include "ObjectIdentifier.hxx"

namespace chart
{

class AccStatisticsObject : public AccessibleChartElement
{
public:
    /*
    enum StatisticsObjectType
    {
        MEAN_VAL_LINE = OBJECTTYPE_AVERAGEVALUE,
        ERROR_BARS    = OBJECTTYPE_ERROR,
        REGRESSION    = OBJECTTYPE_REGRESSION
    };
    */

    AccStatisticsObject( const AccessibleElementInfo& rAccInfo );

    virtual ~AccStatisticsObject();

    // ________ XAccessibleContext ________
    virtual ::rtl::OUString SAL_CALL getAccessibleName() throw (::com::sun::star::uno::RuntimeException);

    // ________ XServiceInfo ________
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);

private:
    //const StatisticsObjectType    m_eType;
    //const sal_uInt16              m_nSeriesIndex;
};

}  // accessibility

#endif
