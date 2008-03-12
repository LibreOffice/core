/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnumPropertyHdl.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:41:50 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_ENUMPROPERTYHANDLER_HXX
#include <xmloff/EnumPropertyHdl.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

/*
#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _INC_LIMITS
#include <limits.h>
#endif


#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
*/

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLEnumPropertyHdl
//

XMLEnumPropertyHdl::~XMLEnumPropertyHdl()
{
    // Nothing to do
}

sal_Bool XMLEnumPropertyHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    USHORT nValue = 0;

    if( SvXMLUnitConverter::convertEnum( nValue, rStrImpValue, mpEnumMap ) )
    {
        switch( mrType.getTypeClass() )
        {
        case TypeClass_ENUM:
            rValue = ::cppu::int2enum( nValue, mrType );
            break;
        case TypeClass_LONG:
            rValue <<= (sal_Int32) nValue;
            break;
        case TypeClass_SHORT:
            rValue <<= (sal_Int16) nValue;
            break;
        case TypeClass_BYTE:
            rValue <<= (sal_Int8) nValue;
            break;
        default:
            DBG_ERROR( "Wrong type for enum property handler!" );
            return sal_False;
        }
        return sal_True;
    }

    return sal_False;
}

sal_Bool XMLEnumPropertyHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    if(!(rValue >>= nValue ))
        if(!::cppu::enum2int(nValue, rValue) )
            return sal_False;

    OUStringBuffer aOut;

    if(!SvXMLUnitConverter::convertEnum( aOut, nValue, mpEnumMap ))
        return sal_False;

    rStrExpValue = aOut.makeStringAndClear();
    return sal_True;
}

