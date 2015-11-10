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

#include "java/lang/Class.hxx"
#include "java/tools.hxx"
#include <rtl/ustring.hxx>

using namespace connectivity;

//************ Class: java.lang.Class


jclass java_lang_Class::theClass = nullptr;

java_lang_Class::~java_lang_Class()
{}

jclass java_lang_Class::getMyClass() const
{
    // the class must be fetched only once, therefore static
    if( !theClass )
        theClass = findMyClass("java/lang/Class");
    return theClass;
}

java_lang_Class * java_lang_Class::forName( const OUString& _par0 )
{
    jobject out(nullptr);
    SDBThreadAttach t;

    {
        OString sClassName = OUStringToOString(_par0, RTL_TEXTENCODING_JAVA_UTF8);
        sClassName = sClassName.replace('.','/');
        out = t.pEnv->FindClass(sClassName.getStr());
        ThrowSQLException(t.pEnv,nullptr);
    } //t.pEnv
    // WARNING: the caller becomes the owner of the returned pointer
    return out==nullptr ? nullptr : new java_lang_Class( t.pEnv, out );
}

jobject java_lang_Class::newInstanceObject()
{
    SDBThreadAttach t;
    static jmethodID mID(nullptr);
    return callObjectMethod(t.pEnv,"newInstance","()Ljava/lang/Object;", mID);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
