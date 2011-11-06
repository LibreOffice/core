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


#ifndef INCLUDED_MODELEVENTLISTENER_HXX
#define INCLUDED_MODELEVENTLISTENER_HXX

#include <WriterFilterDllApi.hxx>
#include <com/sun/star/document/XEventListener.hpp>
#include <cppuhelper/implbase1.hxx>

namespace writerfilter {
namespace dmapper{


class WRITERFILTER_DLLPRIVATE ModelEventListener :
    public cppu::WeakImplHelper1< ::com::sun::star::document::XEventListener >
{
public:
    ModelEventListener();
    ~ModelEventListener();

    virtual void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

};
}//namespace writerfilter
}//namespace dmapper
#endif //
