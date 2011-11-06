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



#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/awt/XCallback.hpp>
#include <cppuhelper/implbase1.hxx>

#include <rtl/ref.hxx>

class OleEmbeddedObject;

#define OLECOMP_ONVIEWCHANGE    1
#define OLECOMP_ONCLOSE         2

class MainThreadNotificationRequest :  public cppu::WeakImplHelper1< com::sun::star::awt::XCallback >
{
    OleEmbeddedObject* m_pObject;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::embed::XEmbeddedObject > m_xObject;

    sal_uInt16 m_nNotificationType;
    sal_uInt32 m_nAspect;

public:
    virtual void SAL_CALL notify (const com::sun::star::uno::Any& rUserData)
        throw (com::sun::star::uno::RuntimeException);
    MainThreadNotificationRequest( const ::rtl::Reference< OleEmbeddedObject >& xObj, sal_uInt16 nNotificationType, sal_uInt32 nAspect = 0 );
    ~MainThreadNotificationRequest();
};


