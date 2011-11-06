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



#ifndef _SFX_FLTLST_HXX
#define _SFX_FLTLST_HXX

//*****************************************************************************************************************
//  includes
//*****************************************************************************************************************
#include <sfx2/fcontnr.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/XRefreshListener.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

//*****************************************************************************************************************
//  declarations
//*****************************************************************************************************************
class SfxFilterListener : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XRefreshListener >
{
    // member
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshable >  m_xFilterCache;

    // c++ interface
    public:
              SfxFilterListener();
             ~SfxFilterListener();

    // uno interface
    public:
        // XRefreshListener
        virtual void SAL_CALL refreshed( const ::com::sun::star::lang::EventObject& aSource ) throw( ::com::sun::star::uno::RuntimeException );
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aSource ) throw( ::com::sun::star::uno::RuntimeException );

};  // SfxFilterListener

#endif // _SFX_FLTLST_HXX
