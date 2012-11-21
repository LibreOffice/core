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

#ifndef _SVX_DLG_DLGUNIT_HXX
#define _SVX_DLG_DLGUNIT_HXX

#include <svx/svdtrans.hxx>

inline String GetUnitString( long nVal_100, FieldUnit eFieldUnit, sal_Unicode cSep )
{
    String aVal = rtl::OUString::valueOf(static_cast<sal_Int64>(
        MetricField::ConvertValue(nVal_100, 2, MAP_100TH_MM, eFieldUnit)));

    while( aVal.Len() < 3 )
        aVal.Insert( sal_Unicode('0'), 0 );

    aVal.Insert( cSep, aVal.Len() - 2 );
    aVal += sal_Unicode(' ');
    aVal += SdrFormatter::GetUnitStr( eFieldUnit );

    return aVal;
}

#endif // _SVX_DLG_DLGUNIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
