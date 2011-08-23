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
#include "globopt.hxx"

#include <stdio.h>

#include <bf_svtools/syslocale.hxx>
#include <unotools/configitem.hxx>
namespace binfilter {

// ================================================================================

/*N*/ namespace
/*N*/ {
/*N*/ 
/*N*/ bool IsMetric()
/*N*/ {
/*N*/     SvtSysLocale aSysLocale;
/*N*/     const LocaleDataWrapper* pLocWrapper = aSysLocale.GetLocaleDataPtr();
/*N*/     MeasurementSystem eSys = pLocWrapper->getMeasurementSystemEnum();
/*N*/ 
/*N*/     return ( eSys == MEASURE_METRIC );
/*N*/ }

// ----------------------------------------

/*N*/ class CalcConfigItem : public ::utl::ConfigItem
/*N*/ {
/*N*/ public:
/*N*/     CalcConfigItem() :
/*N*/             ConfigItem( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Office.Calc/Layout" )))
/*N*/     {}
/*N*/ 
/*N*/     FieldUnit GetMeasureUnit();
        void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
        void Commit();
/*N*/ };

/*N*/ FieldUnit CalcConfigItem::GetMeasureUnit()
/*N*/ {
/*N*/     FieldUnit eResult( FUNIT_CM );
/*N*/ 
/*N*/     ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames( 1 );
/*N*/     if( IsMetric() )
/*N*/         aNames[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Other/MeasureUnit/Metric" ));
/*N*/     else
/*N*/         aNames[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Other/MeasureUnit/NonMetric" ));
/*N*/ 
/*N*/     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aResult(
/*N*/         GetProperties( aNames ));
/*N*/ 
/*N*/     if( aResult[ 0 ].hasValue() )
/*N*/     {
/*N*/         sal_Int32 nValue;
/*N*/         aResult[ 0 ] >>= nValue;
/*N*/         eResult = static_cast< FieldUnit >( nValue );
/*N*/     }
/*N*/ 
/*N*/     return eResult;
/*N*/ }

void CalcConfigItem::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames ) {}
void CalcConfigItem::Commit() {}


}  // anonymous namespace

// ================================================================================

namespace sch
{

namespace util
{

/*N*/ FieldUnit GetMeasureUnit()
/*N*/ {
/*N*/     static CalcConfigItem aCfgItem;
/*N*/     FieldUnit aUnit( aCfgItem.GetMeasureUnit() );
/*N*/ 
/*N*/     return aCfgItem.GetMeasureUnit();
/*N*/ }
 
}  // namespace util
}  // namespace sch
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
