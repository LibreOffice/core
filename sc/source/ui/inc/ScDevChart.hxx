/*************************************************************************
 *
 *  $RCSfile: ScDevChart.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-02-11 09:57:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef INCLUDED_DEVCHARTCONFIG_HXX
#define INCLUDED_DEVCHARTCONFIG_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

namespace ScDevChart
{

class DevChartConfigItem : public ::utl::ConfigItem
{
public:
    DevChartConfigItem() :
            ConfigItem( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Internal" )),
                        CONFIG_MODE_IMMEDIATE_UPDATE )
    {}

    bool UseDevelopmentChart();
};

bool DevChartConfigItem::UseDevelopmentChart()
{
    bool bResult = false;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames( 1 );
    aNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DevelopmentChart" ));

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aResult( GetProperties( aNames ));

    OSL_ASSERT( aResult.getLength());
    aResult[0] >>= bResult;

    return bResult;
}

/** States whether the new chart implementation or the old one should be used.
    If <TRUE/> is returned the newly developed chart (chart2) should be used.
    If <FALSE/> is returned, the old chart (sch) should be used.

    Config-Item: Office.Common/Internal:DevelopmentChart

    This function (the complete header) is only for a transitional period.  It
    will be deprecated after the new chart is definitely integrated into the
    product.
 */
bool UseDevChart()
{
    static DevChartConfigItem aCfgItem;
    return aCfgItem.UseDevelopmentChart();
}

} // namespace ScDevChart

// INCLUDED_DEVCHARTCONFIG_HXX
#endif
