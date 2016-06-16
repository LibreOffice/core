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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_LANG_EXCEPTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_LANG_EXCEPTION_HXX

#include "java/lang/Throwable.hxx"

namespace connectivity
{


    //************ Class: java.lang.Exception

    class java_lang_Exception : public java_lang_Throwable{
    protected:
    // statis Data for the class
        static jclass theClass;
    public:
        virtual jclass getMyClass() const override;
        virtual ~java_lang_Exception() override;
        // a Constructor, that is needed for when Returning the Object is needed:
        java_lang_Exception( JNIEnv * pEnv, jobject myObj ) : java_lang_Throwable( pEnv, myObj ){}

    };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_LANG_EXCEPTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
