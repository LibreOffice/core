/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NamedBoolPropertyHdl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:43:22 $
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

#ifndef _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
#include <xmloff/NamedBoolPropertyHdl.hxx>
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

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLNamedBoolPropertyHdl
//

XMLNamedBoolPropertyHdl::~XMLNamedBoolPropertyHdl()
{
    // Nothing to do
}

sal_Bool XMLNamedBoolPropertyHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    if( rStrImpValue == maTrueStr )
    {
        rValue = ::cppu::bool2any( sal_True );
        return sal_True;
    }

    if( rStrImpValue == maFalseStr )
    {
        rValue = ::cppu::bool2any( sal_False );
        return sal_True;
    }

    return sal_False;
}

sal_Bool XMLNamedBoolPropertyHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    if( ::cppu::any2bool( rValue ) )
    {
        rStrExpValue = maTrueStr;
    }
    else
    {
        rStrExpValue = maFalseStr;
    }

    return sal_True;
}

