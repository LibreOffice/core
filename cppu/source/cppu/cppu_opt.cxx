/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cppu_opt.cxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
