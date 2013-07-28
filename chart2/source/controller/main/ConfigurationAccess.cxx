/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "ConfigurationAccess.hxx"
#include "macros.hxx"

// header for class SvtSysLocale
#include <unotools/syslocale.hxx>
// header for class ConfigItem
#include <unotools/configitem.hxx>
// header for rtl::Static
#include <rtl/instance.hxx>

namespace chart
{
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
    virtual void                    Commit();
    virtual void                    Notify( const uno::Sequence<OUString>& aPropertyNames);
};

CalcConfigItem::CalcConfigItem()
    : ConfigItem( OUString( "Office.Calc/Layout" ))
{
}

CalcConfigItem::~CalcConfigItem()
{
}

void CalcConfigItem::Commit() {}
void CalcConfigItem::Notify( const uno::Sequence<OUString>& ) {}

FieldUnit CalcConfigItem::getFieldUnit()
{
    FieldUnit eResult( FUNIT_CM );

    uno::Sequence< OUString > aNames( 1 );
    if( lcl_IsMetric() )
        aNames[ 0 ] = "Other/MeasureUnit/Metric";
    else
        aNames[ 0 ] = "Other/MeasureUnit/NonMetric";

    uno::Sequence< uno::Any > aResult( GetProperties( aNames ));
    sal_Int32 nValue = 0;
    if( aResult[ 0 ] >>= nValue )
        eResult = static_cast< FieldUnit >( nValue );

    return eResult;
}

namespace
{
    //a CalcConfigItem Singleton
    struct theCalcConfigItem : public rtl::Static< CalcConfigItem, theCalcConfigItem > {};
}

namespace ConfigurationAccess
{
    FieldUnit getFieldUnit()
    {
        FieldUnit aUnit( theCalcConfigItem::get().getFieldUnit() );
        return aUnit;
    }
} //namespace ConfigurationAccess

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
