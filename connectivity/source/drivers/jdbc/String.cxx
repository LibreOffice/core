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

#include "java/lang/String.hxx"
#include "java/tools.hxx"
using namespace connectivity;

//************ Class: java.lang.String


jclass java_lang_String::theClass = nullptr;

java_lang_String::~java_lang_String()
{}

jclass java_lang_String::getMyClass() const
{
    return st_getMyClass();
}
jclass java_lang_String::st_getMyClass()
{
    // the class needs to be fetched only once, that is why it is static
    if( !theClass )
        theClass = findMyClass("java/lang/String");
    return theClass;
}


java_lang_String::operator OUString()
{
    SDBThreadAttach t;
    if(!t.pEnv)
        return OUString();
    return JavaString2String(t.pEnv, static_cast<jstring>(object));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
