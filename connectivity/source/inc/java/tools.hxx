/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _CONNECTIVITY_JAVA_TOOLS_HXX_
#define _CONNECTIVITY_JAVA_TOOLS_HXX_

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.h>

#ifndef JNI_H
#include <jni.h>
#endif
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
    sal_Bool isExceptionOccured(JNIEnv *pEnv,sal_Bool _bClear);

    jobject createByteInputStream(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x,sal_Int32 length);
    jobject createCharArrayReader(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x,sal_Int32 length);
}

#endif // _CONNECTIVITY_JAVA_TOOLS_HXX_

