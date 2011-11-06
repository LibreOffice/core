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
#include <tools/string.hxx>
#include <rtl/ustrbuf.hxx>
#include "propimp0.hxx"
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlimp.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// implementation of graphic property Stroke


//////////////////////////////////////////////////////////////////////////////
// implementation of presentation page property Change

//////////////////////////////////////////////////////////////////////////////
// implementation of an effect duration property handler


XMLDurationPropertyHdl::~XMLDurationPropertyHdl()
{
}

sal_Bool XMLDurationPropertyHdl::importXML(
    const OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& ) const
{
    util::DateTime aTime;
    SvXMLUnitConverter::convertTime( aTime,  rStrImpValue );

    const sal_Int32 nSeconds = ( aTime.Hours * 60 + aTime.Minutes ) * 60 + aTime.Seconds;
    rValue <<= nSeconds;

    return sal_True;
}

sal_Bool XMLDurationPropertyHdl::exportXML(
    OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& ) const
{
    sal_Int32 nVal = 0;

    if(rValue >>= nVal)
    {
        util::DateTime aTime( 0, (sal_uInt16)nVal, 0, 0, 0, 0, 0 );

        OUStringBuffer aOut;
        SvXMLUnitConverter::convertTime( aOut, aTime );
        rStrExpValue = aOut.makeStringAndClear();
        return sal_True;
    }

    return sal_False;
}

//////////////////////////////////////////////////////////////////////////////
// implementation of an opacity property handler


XMLOpacityPropertyHdl::XMLOpacityPropertyHdl( SvXMLImport* pImport )
: mpImport( pImport )
{
}

XMLOpacityPropertyHdl::~XMLOpacityPropertyHdl()
{
}

sal_Bool XMLOpacityPropertyHdl::importXML(
    const OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nValue = 0;

    if( rStrImpValue.indexOf( sal_Unicode('%') ) != -1 )
    {
        if( SvXMLUnitConverter::convertPercent( nValue, rStrImpValue ) )
            bRet = sal_True;
    }
    else
    {
        nValue = sal_Int32( rStrImpValue.toDouble() * 100.0 );
        bRet = sal_True;
    }

    if( bRet )
    {
        // check ranges
        if( nValue < 0 )
            nValue = 0;
        if( nValue > 100 )
            nValue = 100;

        // convert xml opacity to api transparency
        nValue = 100 - nValue;

        // #i42959#
        if( mpImport )
        {
            sal_Int32 nUPD, nBuild;
            if( mpImport->getBuildIds( nUPD, nBuild ) )
            {
                // correct import of documents written prior to StarOffice 8/OOO 2.0 final
                if( (nUPD == 680) && (nBuild < 8951) )
                    nValue = 100 - nValue;
            }
        }

        rValue <<= sal_uInt16(nValue);
    }

    return bRet;
}

sal_Bool XMLOpacityPropertyHdl::exportXML(
    OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nVal = sal_uInt16();

    if( rValue >>= nVal )
    {
        OUStringBuffer aOut;

        nVal = 100 - nVal;
        SvXMLUnitConverter::convertPercent( aOut, nVal );
        rStrExpValue = aOut.makeStringAndClear();
        bRet = sal_True;
    }

    return bRet;
}

//////////////////////////////////////////////////////////////////////////////
// implementation of an text animation step amount

XMLTextAnimationStepPropertyHdl::~XMLTextAnimationStepPropertyHdl()
{
}

sal_Bool XMLTextAnimationStepPropertyHdl::importXML(
    const OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int32 nValue = 0;

    const OUString aPX( RTL_CONSTASCII_USTRINGPARAM( "px" ) );
    sal_Int32 nPos = rStrImpValue.indexOf( aPX );
    if( nPos != -1 )
    {
        if( rUnitConverter.convertNumber( nValue, rStrImpValue.copy( 0, nPos ) ) )
        {
            rValue <<= sal_Int16( -nValue );
            bRet = sal_True;
        }
    }
    else
    {
        if( rUnitConverter.convertMeasure( nValue, rStrImpValue ) )
        {
            rValue <<= sal_Int16( nValue );
            bRet = sal_True;
        }
    }

    return bRet;
}

sal_Bool XMLTextAnimationStepPropertyHdl::exportXML(
    OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nVal = sal_Int16();

    if( rValue >>= nVal )
    {
        OUStringBuffer aOut;

        if( nVal < 0 )
        {
            const OUString aPX( RTL_CONSTASCII_USTRINGPARAM( "px" ) );
            rUnitConverter.convertNumber( aOut, (sal_Int32)-nVal );
            aOut.append( aPX );
        }
        else
        {
            rUnitConverter.convertMeasure( aOut, nVal );
        }

        rStrExpValue = aOut.makeStringAndClear();
        bRet = sal_True;
    }

    return bRet;
}

//////////////////////////////////////////////////////////////////////////////

#include "sdxmlexp_impl.hxx"

XMLDateTimeFormatHdl::XMLDateTimeFormatHdl( SvXMLExport* pExport )
: mpExport( pExport )
{
}

XMLDateTimeFormatHdl::~XMLDateTimeFormatHdl()
{
}

sal_Bool XMLDateTimeFormatHdl::importXML( const rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    rValue <<= rStrImpValue;
    return true;
}

sal_Bool XMLDateTimeFormatHdl::exportXML( rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nNumberFormat = 0;
    if( mpExport && (rValue >>= nNumberFormat) )
    {
        mpExport->addDataStyle( nNumberFormat );
        rStrExpValue = mpExport->getDataStyleName( nNumberFormat );
        return sal_True;
    }

    return sal_False;
}
