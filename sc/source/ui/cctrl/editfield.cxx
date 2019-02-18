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
#include <editfield.hxx>
#include <comphelper/string.hxx>
#include <rtl/math.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/builder.hxx>
#include <global.hxx>

namespace {

sal_Unicode lclGetDecSep()
{
    return ScGlobal::GetpLocaleData()->getNumDecimalSep()[0];
}

} // namespace

ScDoubleField::ScDoubleField( vcl::Window* pParent, WinBits nStyle ) :
    Edit( pParent, nStyle )
{
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeScDoubleField(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    BuilderUtils::ensureDefaultWidthChars(rMap);
    rRet = VclPtr<ScDoubleField>::Create(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK);
}

bool ScDoubleField::GetValue( double& rfValue ) const
{
    OUString aStr(comphelper::string::strip(GetText(), ' '));
    bool bOk = !aStr.isEmpty();
    if( bOk )
    {
        rtl_math_ConversionStatus eStatus;
        sal_Int32 nEnd;
        rfValue = ScGlobal::GetpLocaleData()->stringToDouble( aStr, true, &eStatus, &nEnd );
        bOk = (eStatus == rtl_math_ConversionStatus_Ok) && (nEnd == aStr.getLength() );
    }
    return bOk;
}

void ScDoubleField::SetValue( double fValue, sal_Int32 nDecPlaces )
{
    SetText( ::rtl::math::doubleToUString( fValue, rtl_math_StringFormat_G,
        nDecPlaces, lclGetDecSep(), true/*bEraseTrailingDecZeros*/ ) );
}

DoubleField::DoubleField(std::unique_ptr<weld::Entry> xEntry)
    : m_xEntry(std::move(xEntry))
{
}

bool DoubleField::GetValue( double& rfValue ) const
{
    OUString aStr(comphelper::string::strip(m_xEntry->get_text(), ' '));
    bool bOk = !aStr.isEmpty();
    if( bOk )
    {
        rtl_math_ConversionStatus eStatus;
        sal_Int32 nEnd;
        rfValue = ScGlobal::GetpLocaleData()->stringToDouble( aStr, true, &eStatus, &nEnd );
        bOk = (eStatus == rtl_math_ConversionStatus_Ok) && (nEnd == aStr.getLength() );
    }
    return bOk;
}

void DoubleField::SetValue( double fValue, sal_Int32 nDecPlaces )
{
    m_xEntry->set_text( ::rtl::math::doubleToUString( fValue, rtl_math_StringFormat_G,
        nDecPlaces, lclGetDecSep(), true/*bEraseTrailingDecZeros*/ ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
