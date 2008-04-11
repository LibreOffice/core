/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pyuno_util.cxx,v $
 * $Revision: 1.9 $
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

#include "pyuno_impl.hxx"

#include <time.h>
#include <osl/thread.h>

#include <typelib/typedescription.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/time.h>

#include <com/sun/star/beans/XMaterialHolder.hpp>

using rtl::OUStringToOString;
using rtl::OUString;
using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUStringBuffer;


using com::sun::star::uno::TypeDescription;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;
using com::sun::star::uno::Type;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::TypeClass;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;
using com::sun::star::lang::XSingleServiceFactory;
using com::sun::star::script::XTypeConverter;
using com::sun::star::beans::XMaterialHolder;

#define USTR_ASCII(x) OUString( RTL_CONSTASCII_USTRINGPARAM( x ) )
namespace pyuno
{
PyRef ustring2PyUnicode( const OUString & str )
{
    PyRef ret;
#if Py_UNICODE_SIZE == 2
    // YD force conversion since python/2 uses wchar_t
    ret = PyRef( PyUnicode_FromUnicode( (const Py_UNICODE*)str.getStr(), str.getLength() ), SAL_NO_ACQUIRE );
#else
    OString sUtf8(OUStringToOString(str, RTL_TEXTENCODING_UTF8));
    ret = PyRef( PyUnicode_DecodeUTF8( sUtf8.getStr(), sUtf8.getLength(), NULL) , SAL_NO_ACQUIRE );
#endif
    return ret;
}

PyRef ustring2PyString( const OUString &str )
{
    OString o = OUStringToOString( str, osl_getThreadTextEncoding() );
    return PyRef( PyString_FromString( o.getStr() ), SAL_NO_ACQUIRE );
}

OUString pyString2ustring( PyObject *pystr )
{
    OUString ret;
    if( PyUnicode_Check( pystr ) )
    {
#if Py_UNICODE_SIZE == 2
    ret = OUString( (sal_Unicode * ) PyUnicode_AS_UNICODE( pystr ) );
#else
    PyObject* pUtf8 = PyUnicode_AsUTF8String(pystr);
    ret = OUString(PyString_AsString(pUtf8), PyString_Size(pUtf8), RTL_TEXTENCODING_UTF8);
    Py_DECREF(pUtf8);
#endif
    }
    else
    {
        char *name = PyString_AsString(pystr );
        ret = OUString( name, strlen(name), osl_getThreadTextEncoding() );
    }
    return ret;
}

PyRef getObjectFromUnoModule( const Runtime &runtime, const char * func )
    throw ( RuntimeException )
{
    PyRef object(PyDict_GetItemString( runtime.getImpl()->cargo->getUnoModule().get(), (char*)func ) );
    if( !object.is() )
    {
        OUStringBuffer buf;
        buf.appendAscii( "couldn't find core function " );
        buf.appendAscii( func );
        throw RuntimeException(buf.makeStringAndClear(),Reference< XInterface >());
    }
    return object;
}


//------------------------------------------------------------------------------------
// Logging
//------------------------------------------------------------------------------------

bool isLog( RuntimeCargo * cargo, sal_Int32 loglevel )
{
    return cargo && cargo->logFile && loglevel <= cargo->logLevel;
}

void log( RuntimeCargo * cargo, sal_Int32 level, const rtl::OUString &logString )
{
    log( cargo, level, OUStringToOString( logString, osl_getThreadTextEncoding() ).getStr() );
}

void log( RuntimeCargo * cargo, sal_Int32 level, const char *str )
{
    if( isLog( cargo, level ) )
    {
        static const char *strLevel[] = { "NONE", "CALL", "ARGS" };

        TimeValue systemTime;
        TimeValue localTime;
        oslDateTime localDateTime;

        osl_getSystemTime( &systemTime );
        osl_getLocalTimeFromSystemTime( &systemTime, &localTime );
        osl_getDateTimeFromTimeValue( &localTime, &localDateTime );

        fprintf( cargo->logFile,
                 "%4i-%02i-%02i %02i:%02i:%02i,%03lu [%s,tid %ld]: %s\n",
                 localDateTime.Year,
                 localDateTime.Month,
                 localDateTime.Day,
                 localDateTime.Hours,
                 localDateTime.Minutes,
                 localDateTime.Seconds,
                 sal::static_int_cast< unsigned long >(
                     localDateTime.NanoSeconds/1000000),
                 strLevel[level],
                 sal::static_int_cast< long >(
                     (sal_Int32) osl_getThreadIdentifier( 0)),
                 str );
    }
}

namespace {

void appendPointer(rtl::OUStringBuffer & buffer, void * pointer) {
    buffer.append(
        sal::static_int_cast< sal_Int64 >(
            reinterpret_cast< sal_IntPtr >(pointer)),
        16);
}

}

void logException( RuntimeCargo *cargo, const char *intro,
                   void * ptr, const rtl::OUString &aFunctionName,
                   const void * data, const com::sun::star::uno::Type & type )
{
    if( isLog( cargo, LogLevel::CALL ) )
    {
        rtl::OUStringBuffer buf( 128 );
        buf.appendAscii( intro );
        appendPointer(buf, ptr);
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("].") );
        buf.append( aFunctionName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " = " ) );
        buf.append(
            val2str( data, type.getTypeLibType(), VAL2STR_MODE_SHALLOW ) );
        log( cargo,LogLevel::CALL, buf.makeStringAndClear() );
    }

}

void logReply(
    RuntimeCargo *cargo,
    const char *intro,
    void * ptr,
    const rtl::OUString & aFunctionName,
    const Any &returnValue,
    const Sequence< Any > & aParams )
{
    rtl::OUStringBuffer buf( 128 );
    buf.appendAscii( intro );
    appendPointer(buf, ptr);
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("].") );
    buf.append( aFunctionName );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("()=") );
    if( isLog( cargo, LogLevel::ARGS ) )
    {
        buf.append(
            val2str( returnValue.getValue(), returnValue.getValueTypeRef(), VAL2STR_MODE_SHALLOW) );
        for( int i = 0; i < aParams.getLength() ; i ++ )
        {
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", " ) );
            buf.append(
                val2str( aParams[i].getValue(), aParams[i].getValueTypeRef(), VAL2STR_MODE_SHALLOW) );
        }
    }
    log( cargo,LogLevel::CALL, buf.makeStringAndClear() );

}

void logCall( RuntimeCargo *cargo, const char *intro,
              void * ptr, const rtl::OUString & aFunctionName,
              const Sequence< Any > & aParams )
{
    rtl::OUStringBuffer buf( 128 );
    buf.appendAscii( intro );
    appendPointer(buf, ptr);
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("].") );
    buf.append( aFunctionName );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("(") );
    if( isLog( cargo, LogLevel::ARGS ) )
    {
        for( int i = 0; i < aParams.getLength() ; i ++ )
        {
            if( i > 0 )
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", " ) );
            buf.append(
                val2str( aParams[i].getValue(), aParams[i].getValueTypeRef(), VAL2STR_MODE_SHALLOW) );
        }
    }
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(")") );
    log( cargo,LogLevel::CALL, buf.makeStringAndClear() );
}


}
