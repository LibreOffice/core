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


#ifdef SC_DLLIMPLEMENTATION
#undef SC_DLLIMPLEMENTATION
#endif
#include "editfield.hxx"
#include <comphelper/string.hxx>
#include <rtl/math.hxx>
#include <unotools/localedatawrapper.hxx>
#include "global.hxx"

// ============================================================================

namespace {

sal_Unicode lclGetDecSep()
{
    return ScGlobal::GetpLocaleData()->getNumDecimalSep()[0];
}

sal_Unicode lclGetGroupSep()
{
    return ScGlobal::GetpLocaleData()->getNumThousandSep()[0];
}

} // namespace

// ============================================================================

ScDoubleField::ScDoubleField( Window* pParent, const ResId& rResId ) :
    Edit( pParent, rResId )
{
}

bool ScDoubleField::GetValue( double& rfValue ) const
{
    OUString aStr(comphelper::string::strip(GetText(), ' '));
    bool bOk = !aStr.isEmpty();
    if( bOk )
    {
        rtl_math_ConversionStatus eStatus;
        sal_Int32 nEnd;
        rfValue = rtl::math::stringToDouble( aStr, lclGetDecSep(), lclGetGroupSep(), &eStatus, &nEnd );
        bOk = (eStatus == rtl_math_ConversionStatus_Ok) && (nEnd == aStr.getLength() );
    }
    return bOk;
}

void ScDoubleField::SetValue( double fValue, sal_Int32 nDecPlaces, bool bEraseTrailingDecZeros )
{
    SetText( ::rtl::math::doubleToUString( fValue, rtl_math_StringFormat_G,
        nDecPlaces, lclGetDecSep(), bEraseTrailingDecZeros ) );
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
