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



#ifndef __SO_CLOSELISTENER_HXX_
#define __SO_CLOSELISTENER_HXX_

#include <com/sun/star/util/XCloseListener.hpp>
#include <cppuhelper/implbase1.hxx>

class PluginDocumentClosePreventer : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XCloseListener >
{
    sal_Bool m_bPreventClose;

public:
    PluginDocumentClosePreventer()
    : m_bPreventClose( sal_True )
    {}

    void StopPreventClose() { m_bPreventClose = sal_False; }

    virtual void SAL_CALL queryClosing( const ::com::sun::star::lang::EventObject& aEvent, sal_Bool bDeliverOwnership )
        throw ( ::com::sun::star::uno::RuntimeException, ::com::sun::star::util::CloseVetoException );

    virtual void SAL_CALL notifyClosing( const ::com::sun::star::lang::EventObject& aEvent ) throw ( ::com::sun::star::uno::RuntimeException ) ;

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw ( ::com::sun::star::uno::RuntimeException ) ;
};

#endif

