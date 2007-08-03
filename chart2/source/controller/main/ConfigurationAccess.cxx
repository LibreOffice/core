/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigurationAccess.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 12:35:26 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

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

// ----------------------------------------

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
    sal_Int32 nValue = 0;
    if( aResult[ 0 ] >>= nValue )
        eResult = static_cast< FieldUnit >( nValue );

    return eResult;
}

// ----------------------------------------

ConfigurationAccess * ConfigurationAccess::m_pThis = 0;

// private, use static getConfigurationAccess method
ConfigurationAccess::ConfigurationAccess()
    : m_pCalcConfigItem(0)
{
    m_pCalcConfigItem = new CalcConfigItem();
}

// static
ConfigurationAccess * ConfigurationAccess::getConfigurationAccess()
{
    // note: not threadsafe
    if( !m_pThis )
        m_pThis = new ConfigurationAccess();
    return m_pThis;
}

ConfigurationAccess::~ConfigurationAccess()
{
    delete m_pCalcConfigItem;
}

FieldUnit ConfigurationAccess::getFieldUnit()
{
    OSL_ASSERT( m_pCalcConfigItem );
    FieldUnit aUnit( m_pCalcConfigItem->getFieldUnit() );
    return aUnit;
}

//.............................................................................
} //namespace chart
//.............................................................................
