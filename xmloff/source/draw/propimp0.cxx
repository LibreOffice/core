/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propimp0.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:33:41 $
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
#include "precompiled_xmloff.hxx"

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _PROPIMP0_HXX
#include "propimp0.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif

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
