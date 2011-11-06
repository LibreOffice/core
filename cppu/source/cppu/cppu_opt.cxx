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
