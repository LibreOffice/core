/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cppu_opt.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:20:00 $
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
#include "precompiled_cppu.hxx"

#include "typelib/typedescription.h"
#include "uno/any2.h"
#include "rtl/ustrbuf.hxx"


using namespace ::rtl;

//##################################################################################################
extern "C" rtl_uString * SAL_CALL cppu_unsatisfied_iquery_msg(
    typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C()
{
    OUStringBuffer buf( 64 );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("unsatisfied query for interface of type ") );
    buf.append( OUString::unacquired( &pType->pTypeName ) );
    buf.append( (sal_Unicode) '!' );
    OUString ret( buf.makeStringAndClear() );
    rtl_uString_acquire( ret.pData );
    return ret.pData;
}

//##################################################################################################
extern "C" rtl_uString * SAL_CALL cppu_unsatisfied_iset_msg(
    typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C()
{
    OUStringBuffer buf( 64 );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("invalid attempt to assign an empty interface of type ") );
    buf.append( OUString::unacquired( &pType->pTypeName ) );
    buf.append( (sal_Unicode) '!' );
    OUString ret( buf.makeStringAndClear() );
    rtl_uString_acquire( ret.pData );
    return ret.pData;
}

//##############################################################################
extern "C" rtl_uString * SAL_CALL cppu_Any_extraction_failure_msg(
    uno_Any * pAny, typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C()
{
    OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                         "Cannot extract an Any(") );
    buf.append( OUString::unacquired(&pAny->pType->pTypeName) );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(") to ") );
    buf.append( OUString::unacquired(&pType->pTypeName) );
    buf.append( static_cast<sal_Unicode>('!') );
    const OUString ret( buf.makeStringAndClear() );
    rtl_uString_acquire( ret.pData );
    return ret.pData;
}
