/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifdef SC_DLLIMPLEMENTATION
#undef SC_DLLIMPLEMENTATION
#endif
#include "editfield.hxx"
#include <rtl/math.hxx>
#include <unotools/localedatawrapper.hxx>
#include "global.hxx"

// ============================================================================

namespace {

sal_Unicode lclGetDecSep()
{
    return ScGlobal::GetpLocaleData()->getNumDecimalSep().GetChar( 0 );
}

sal_Unicode lclGetGroupSep()
{
    return ScGlobal::GetpLocaleData()->getNumThousandSep().GetChar( 0 );
}

} // namespace

// ============================================================================

ScDoubleField::ScDoubleField( Window* pParent, const ResId& rResId ) :
    Edit( pParent, rResId )
{
}

bool ScDoubleField::GetValue( double& rfValue ) const
{
    String aStr( GetText() );
    aStr.EraseLeadingAndTrailingChars( ' ' );
    bool bOk = aStr.Len() > 0;
    if( bOk )
    {
        rtl_math_ConversionStatus eStatus;
        sal_Int32 nEnd;
        rfValue = rtl::math::stringToDouble( aStr, lclGetDecSep(), lclGetGroupSep(), &eStatus, &nEnd );
        bOk = (eStatus == rtl_math_ConversionStatus_Ok) && (nEnd == static_cast< sal_Int32 >( aStr.Len() ));
    }
    return bOk;
}

void ScDoubleField::SetValue( double fValue, sal_Int32 nDecPlaces, bool bEraseTrailingDecZeros )
{
    SetText( ::rtl::math::doubleToUString( fValue, rtl_math_StringFormat_G,
        nDecPlaces, lclGetDecSep(), bEraseTrailingDecZeros ) );
}

// ============================================================================

