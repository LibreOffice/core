/*************************************************************************
 *
 *  $RCSfile: ConfigurationAccess.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: iha $ $Date: 2003-12-10 18:11:33 $
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

#include "ConfigurationAccess.hxx"
#include "macros.hxx"

// header for class SvtSysLocale
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
// header for class ConfigItem
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif


//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

namespace
{
bool lcl_IsMetric()
{
    SvtSysLocale aSysLocale;
    const LocaleDataWrapper* pLocWrapper = aSysLocale.GetLocaleDataPtr();
    MeasurementSystem eSys = pLocWrapper->getMeasurementSystemEnum();

    return ( eSys == MEASURE_METRIC );
}
}//end anonymous namespace

class CalcConfigItem : public ::utl::ConfigItem
{
public:
    CalcConfigItem();
    virtual ~CalcConfigItem();

    FieldUnit getFieldUnit();
};

CalcConfigItem::CalcConfigItem()
    : ConfigItem( ::rtl::OUString( C2U( "Office.Calc/Layout" )))
{
}
CalcConfigItem::~CalcConfigItem()
{
}

FieldUnit CalcConfigItem::getFieldUnit()
{
    FieldUnit eResult( FUNIT_CM );

    uno::Sequence< ::rtl::OUString > aNames( 1 );
    if( lcl_IsMetric() )
        aNames[ 0 ] = ::rtl::OUString( C2U( "Other/MeasureUnit/Metric" ));
    else
        aNames[ 0 ] = ::rtl::OUString( C2U( "Other/MeasureUnit/NonMetric" ));

    uno::Sequence< uno::Any > aResult( GetProperties( aNames ));
    sal_Int32 nValue;
    if( aResult[ 0 ] >>= nValue )
        eResult = static_cast< FieldUnit >( nValue );

    return eResult;
}

ConfigurationAccess::ConfigurationAccess()
    : m_pCalcConfigItem(0)
{
    m_pCalcConfigItem = new CalcConfigItem();
}
ConfigurationAccess::~ConfigurationAccess()
{
    delete m_pCalcConfigItem;
}
FieldUnit ConfigurationAccess::getFieldUnit()
{
    FieldUnit aUnit( m_pCalcConfigItem->getFieldUnit() );
    return aUnit;
}

//.............................................................................
} //namespace chart
//.............................................................................
