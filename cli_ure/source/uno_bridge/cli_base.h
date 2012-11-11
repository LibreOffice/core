/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CLI_BASE_H
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

#using <system.dll>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

namespace cli_uno
{
System::Type^ loadCliType(System::String ^ typeName);
System::Type^ mapUnoType(typelib_TypeDescription const * pTD);
System::Type^ mapUnoType(typelib_TypeDescriptionReference const * pTD);
typelib_TypeDescriptionReference* mapCliType(System::Type^ cliType);
rtl::OUString mapCliString(System::String ^ data);
System::String^ mapUnoString(rtl_uString const * data);
System::String^ mapUnoTypeName(rtl_uString const * typeName);

ref struct Constants
{
    static System::String^ sXInterfaceName= gcnew System::String(
        "unoidl.com.sun.star.uno.XInterface");
    static System::String^ sObject= gcnew System::String("System.Object");
    static System::String^ sType= gcnew System::String("System.Type");
    static System::String^ sUnoidl= gcnew System::String("unoidl.");
    static System::String^ sVoid= gcnew System::String("System.Void");
    static System::String^ sAny= gcnew System::String("uno.Any");
    static System::String^ sArArray= gcnew System::String("System.Array[]");
    static System::String^ sBoolean= gcnew System::String("System.Boolean");
    static System::String^ sChar= gcnew System::String("System.Char");
    static System::String^ sByte= gcnew System::String("System.Byte");
    static System::String^ sInt16= gcnew System::String("System.Int16");
    static System::String^ sUInt16= gcnew System::String("System.UInt16");
    static System::String^ sInt32= gcnew System::String("System.Int32");
    static System::String^ sUInt32= gcnew System::String("System.UInt32");
    static System::String^ sInt64= gcnew System::String("System.Int64");
    static System::String^ sUInt64= gcnew System::String("System.UInt64");
    static System::String^ sString= gcnew System::String("System.String");
    static System::String^ sSingle= gcnew System::String("System.Single");
    static System::String^ sDouble= gcnew System::String("System.Double");
    static System::String^ sArBoolean= gcnew System::String("System.Boolean[]");
    static System::String^ sArChar= gcnew System::String("System.Char[]");
    static System::String^ sArByte= gcnew System::String("System.Byte[]");
    static System::String^ sArInt16= gcnew System::String("System.Int16[]");
    static System::String^ sArUInt16= gcnew System::String("System.UInt16[]");
    static System::String^ sArInt32= gcnew System::String("System.Int32[]");
    static System::String^ sArUInt32= gcnew System::String("System.UInt32[]");
    static System::String^ sArInt64= gcnew System::String("System.Int64[]");
    static System::String^ sArUInt64= gcnew System::String("System.UInt64[]");
    static System::String^ sArString= gcnew System::String("System.String[]");
    static System::String^ sArSingle= gcnew System::String("System.Single[]");
    static System::String^ sArDouble= gcnew System::String("System.Double[]");
    static System::String^ sArType= gcnew System::String("System.Type[]");
    static System::String^ sArObject= gcnew System::String("System.Object[]");
    static System::String^ sBrackets= gcnew System::String("[]");
    static System::String^ sAttributeSet= gcnew System::String("set_");
    static System::String^ sAttributeGet= gcnew System::String("get_");

    static System::String^ usXInterface = "com.sun.star.uno.XInterface";
    static System::String^ usVoid = "void";
    static System::String^ usType = "type";
    static System::String^ usAny = "any";
    static System::String^ usBrackets = "[]";
    static System::String^ usBool = "boolean";
    static System::String^ usByte = "byte";
    static System::String^ usChar = "char";
    static System::String^ usShort = "short";
    static System::String^ usUShort = "unsigned short";
    static System::String^ usLong = "long";
    static System::String^ usULong = "unsigned long";
    static System::String^ usHyper = "hyper";
    static System::String^ usUHyper = "unsigned hyper";
    static System::String^ usString = "string";
    static System::String^ usFloat = "float";
    static System::String^ usDouble = "double";
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
    inline ~TypeDescr() SAL_THROW(())
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
