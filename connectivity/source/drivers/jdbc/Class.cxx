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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "java/lang/Class.hxx"
#include "java/tools.hxx"
#include <rtl/ustring.hxx>

using namespace connectivity;
//**************************************************************
//************ Class: java.lang.Class
//**************************************************************

jclass java_lang_Class::theClass = 0;

java_lang_Class::~java_lang_Class()
{}

jclass java_lang_Class::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/lang/Class");
    return theClass;
}

java_lang_Class * java_lang_Class::forName( const ::rtl::OUString& _par0 )
{
    jobject out(NULL);
    SDBThreadAttach t;

    {
        ::rtl::OString sClassName = ::rtl::OUStringToOString(_par0, RTL_TEXTENCODING_JAVA_UTF8);
        sClassName = sClassName.replace('.','/');
        out = t.pEnv->FindClass(sClassName.getStr());
        ThrowSQLException(t.pEnv,0);
    } //t.pEnv
    // WARNING: the caller becomes the owner of the returned pointer
    return out==0 ? NULL : new java_lang_Class( t.pEnv, out );
}

jobject java_lang_Class::newInstanceObject()
{
    SDBThreadAttach t;
    static jmethodID mID(NULL);
    return callObjectMethod(t.pEnv,"newInstance","()Ljava/lang/Object;", mID);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
