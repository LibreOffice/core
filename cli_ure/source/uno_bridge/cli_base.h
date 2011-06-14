/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#if ! defined INCLUDED_CLI_BASE_H
#define INCLUDED_CLI_BASE_H

#pragma unmanaged
// Workaround: osl/mutex.h contains only a forward declaration of _oslMutexImpls.
// When using the inline class in Mutex in osl/mutex.hxx, the loader needs to find
// a declaration for the struct. If not found a TypeLoadException is being thrown.
struct _oslMutexImpl
{
};
#pragma managed
#include <memory>
#include "rtl/ustring.hxx"
#include "typelib/typedescription.hxx"

#using <mscorlib.dll>
#using <system.dll>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

namespace cli_uno
{
System::Type* loadCliType(System::String * typeName);
System::Type* mapUnoType(typelib_TypeDescription const * pTD);
System::Type* mapUnoType(typelib_TypeDescriptionReference const * pTD);
typelib_TypeDescriptionReference* mapCliType(System::Type* cliType);
rtl::OUString mapCliString(System::String const * data);
System::String* mapUnoString(rtl_uString const * data);
System::String* mapUnoTypeName(rtl_uString const * typeName);

__gc struct Constants
{
    static const System::String* sXInterfaceName= new System::String(
        S"unoidl.com.sun.star.uno.XInterface");
    static const System::String* sObject= new System::String(S"System.Object");
    static const System::String* sType= new System::String(S"System.Type");
    static const System::String* sUnoidl= new System::String(S"unoidl.");
    static const System::String* sVoid= new System::String(S"System.Void");
    static const System::String* sAny= new System::String(S"uno.Any");
    static const System::String* sArArray= new System::String(S"System.Array[]");
    static const System::String* sBoolean= new System::String(S"System.Boolean");
    static const System::String* sChar= new System::String(S"System.Char");
    static const System::String* sByte= new System::String(S"System.Byte");
    static const System::String* sInt16= new System::String(S"System.Int16");
    static const System::String* sUInt16= new System::String(S"System.UInt16");
    static const System::String* sInt32= new System::String(S"System.Int32");
    static const System::String* sUInt32= new System::String(S"System.UInt32");
    static const System::String* sInt64= new System::String(S"System.Int64");
    static const System::String* sUInt64= new System::String(S"System.UInt64");
    static const System::String* sString= new System::String(S"System.String");
    static const System::String* sSingle= new System::String(S"System.Single");
    static const System::String* sDouble= new System::String(S"System.Double");
    static const System::String* sArBoolean= new System::String(S"System.Boolean[]");
    static const System::String* sArChar= new System::String(S"System.Char[]");
    static const System::String* sArByte= new System::String(S"System.Byte[]");
    static const System::String* sArInt16= new System::String(S"System.Int16[]");
    static const System::String* sArUInt16= new System::String(S"System.UInt16[]");
    static const System::String* sArInt32= new System::String(S"System.Int32[]");
    static const System::String* sArUInt32= new System::String(S"System.UInt32[]");
    static const System::String* sArInt64= new System::String(S"System.Int64[]");
    static const System::String* sArUInt64= new System::String(S"System.UInt64[]");
    static const System::String* sArString= new System::String(S"System.String[]");
    static const System::String* sArSingle= new System::String(S"System.Single[]");
    static const System::String* sArDouble= new System::String(S"System.Double[]");
    static const System::String* sArType= new System::String(S"System.Type[]");
    static const System::String* sArObject= new System::String(S"System.Object[]");
    static const System::String* sBrackets= new System::String(S"[]");
    static const System::String* sAttributeSet= new System::String(S"set_");
    static const System::String* sAttributeGet= new System::String(S"get_");

    static const System::String* usXInterface = S"com.sun.star.uno.XInterface";
    static const System::String* usVoid = S"void";
    static const System::String* usType = S"type";
    static const System::String* usAny = S"any";
    static const System::String* usBrackets = S"[]";
    static const System::String* usBool = S"boolean";
    static const System::String* usByte = S"byte";
    static const System::String* usChar = S"char";
    static const System::String* usShort = S"short";
    static const System::String* usUShort = S"unsigned short";
    static const System::String* usLong = S"long";
    static const System::String* usULong = S"unsigned long";
    static const System::String* usHyper = S"hyper";
    static const System::String* usUHyper = S"unsigned hyper";
    static const System::String* usString = S"string";
    static const System::String* usFloat = S"float";
    static const System::String* usDouble = S"double";
};

struct BridgeRuntimeError
{
    ::rtl::OUString m_message;

    inline BridgeRuntimeError( ::rtl::OUString const & message )
        : m_message( message )
        {}
};

//==================================================================================================
struct rtl_mem
{
    inline static void * operator new ( size_t nSize )
        { return rtl_allocateMemory( nSize ); }
    inline static void operator delete ( void * mem )
        { if (mem) rtl_freeMemory( mem ); }
    inline static void * operator new ( size_t, void * mem )
        { return mem; }
    inline static void operator delete ( void *, void * )
        {}

    static inline ::std::auto_ptr< rtl_mem > allocate( ::std::size_t bytes );
};
//--------------------------------------------------------------------------------------------------
inline ::std::auto_ptr< rtl_mem > rtl_mem::allocate( ::std::size_t bytes )
{
    void * p = rtl_allocateMemory( bytes );
    if (0 == p)
        throw BridgeRuntimeError(OUSTR("out of memory!") );
    return ::std::auto_ptr< rtl_mem >( (rtl_mem *)p );
}

//==================================================================================================
class TypeDescr
{
    typelib_TypeDescription * m_td;

    TypeDescr( TypeDescr & ); // not impl
    void operator = ( TypeDescr ); // not impl

public:
    inline explicit TypeDescr( typelib_TypeDescriptionReference * td_ref );
    inline ~TypeDescr() SAL_THROW( () )
        { TYPELIB_DANGER_RELEASE( m_td ); }

    inline typelib_TypeDescription * get() const
        { return m_td; }
};

inline TypeDescr::TypeDescr( typelib_TypeDescriptionReference * td_ref )
    : m_td( 0 )
{
    TYPELIB_DANGER_GET( &m_td, td_ref );
    if (0 == m_td)
    {
        throw BridgeRuntimeError(
            OUSTR("cannot get comprehensive type description for ") +
            *reinterpret_cast< ::rtl::OUString const * >( &td_ref->pTypeName ) );
    }
}


} //end namespace cli_uno
 #endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
