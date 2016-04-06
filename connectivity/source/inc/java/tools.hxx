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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_TOOLS_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_TOOLS_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.h>

#include <jni.h>
#include "java/sql/SQLException.hxx"
#include <comphelper/uno3.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>


namespace connectivity
{

    jstring convertwchar_tToJavaString(JNIEnv *pEnv,const OUString& Temp);
    OUString JavaString2String(JNIEnv *pEnv,jstring Str);
    class java_util_Properties;

    java_util_Properties* createStringPropertyArray(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    jobject convertTypeMapToJavaMap(JNIEnv *pEnv,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & _rMap);

    /** return if a exception occurred
        @param  pEnv
            The native java env
        @param  _bClear
            <TRUE/> if the exception should be cleared
        @return
            <TRUE/> if an exception is occurred
    */
    bool isExceptionOccurred(JNIEnv *pEnv,bool _bClear);

    jobject createByteInputStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x,sal_Int32 length);
    jobject createCharArrayReader(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x,sal_Int32 length);
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_TOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
