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

#include "java/lang/Throwable.hxx"
#include "java/tools.hxx"
using namespace connectivity;
//**************************************************************
//************ Class: java.lang.Throwable
//**************************************************************

jclass java_lang_Throwable::theClass = 0;

java_lang_Throwable::~java_lang_Throwable()
{}

jclass java_lang_Throwable::getMyClass() const
{
    return st_getMyClass();
}
jclass java_lang_Throwable::st_getMyClass()
{
    // the class needs to be fetched only once, that is why it is static
    if( !theClass )
        theClass = findMyClass("java/lang/Throwable");
    return theClass;
}
// -----------------------------------------------------------------------------

::rtl::OUString java_lang_Throwable::getMessage() const
{
    static jmethodID mID(NULL);
    return callStringMethod("getMessage",mID);
}
// -----------------------------------------------------------------------------

::rtl::OUString java_lang_Throwable::getLocalizedMessage() const
{
    static jmethodID mID(NULL);
    return callStringMethod("getLocalizedMessage",mID);
}

#if OSL_DEBUG_LEVEL > 0
void java_lang_Throwable::printStackTrace() const
{
    static jmethodID mID(NULL);
    return callVoidMethod("printStackTrace",mID);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
