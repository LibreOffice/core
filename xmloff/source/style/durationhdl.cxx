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
#include "precompiled_xmloff.hxx"
#include "durationhdl.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmluconv.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

// ---------------------------------------------------------------------------


sal_Bool XMLDurationMS16PropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    DateTime aTime;
    if( !SvXMLUnitConverter::convertTime( aTime,  rStrImpValue ) )
        return false;

    const sal_Int16 nMS = ( ( aTime.Hours * 60 + aTime.Minutes ) * 60 + aTime.Seconds ) * 100 + aTime.HundredthSeconds;
    rValue <<= nMS;

    return sal_True;
}

sal_Bool XMLDurationMS16PropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Int16 nMS = sal_Int16();

    if(rValue >>= nMS)
    {
        OUStringBuffer aOut;
        DateTime aTime( nMS, 0, 0, 0, 0, 0, 0 );
        SvXMLUnitConverter::convertTime( aOut, aTime );
        rStrExpValue = aOut.makeStringAndClear();
        return sal_True;
    }

    return sal_False;
}

XMLDurationMS16PropHdl_Impl::~XMLDurationMS16PropHdl_Impl()
{
}
