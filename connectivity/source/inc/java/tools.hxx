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

#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#define _CONNECTIVITY_JAVA_TOOLS_HXX_

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

    jstring convertwchar_tToJavaString(JNIEnv *pEnv,const ::rtl::OUString& _Temp);
    ::rtl::OUString JavaString2String(JNIEnv *pEnv,jstring _Str);
    class java_util_Properties;

    java_util_Properties* createStringPropertyArray(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    template<class T,class JT> ::com::sun::star::uno::Sequence< T > copyArrayAndDelete(JNIEnv *pEnv,jobjectArray _Array, const T*, const JT* )
    {
                ::com::sun::star::uno::Sequence< T > xOut;
        if(_Array)
        {
            jsize  nLen = pEnv->GetArrayLength(_Array);
            xOut.realloc(nLen);
            for(jsize i=0;i<nLen;++i)
            {
                JT xInfo(pEnv,pEnv->GetObjectArrayElement(_Array,i));
                java_lang_Object::ThrowSQLException(pEnv,NULL);
                xOut.getArray()[i] = xInfo;
            }
            pEnv->DeleteLocalRef(_Array);
        }
        return xOut;
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > Map2XNameAccess(JNIEnv *pEnv,jobject _pMap);
    jobject convertTypeMapToJavaMap(JNIEnv *pEnv,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & _rMap);

    /** return if a exception occurred
        @param  pEnv
            The native java env
        @param  _bClear
            <TRUE/> if the execption should be cleared
        @return
            <TRUE/> if an exception is occurred
    */
    sal_Bool isExceptionOccurred(JNIEnv *pEnv,sal_Bool _bClear);

    jobject createByteInputStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x,sal_Int32 length);
    jobject createCharArrayReader(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x,sal_Int32 length);
}

#endif // _CONNECTIVITY_JAVA_TOOLS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
