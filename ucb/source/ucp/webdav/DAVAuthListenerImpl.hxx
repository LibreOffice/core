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



#ifndef _DAVAUTHLISTENERIMPL_HXX_
#define _DAVAUTHLISTENERIMPL_HXX_

#include "DAVAuthListener.hxx"


namespace webdav_ucp
{

//=========================================================================

//=========================================================================
//=========================================================================
//
// class DAVAuthListenerImpl.
//
//=========================================================================
//=========================================================================


    class DAVAuthListener_Impl : public DAVAuthListener
    {
    public:

        DAVAuthListener_Impl(
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XCommandEnvironment>& xEnv,
            const ::rtl::OUString & inURL )
            : m_xEnv( xEnv ), m_aURL( inURL )
        {
        }

        virtual int authenticate( const ::rtl::OUString & inRealm,
                                  const ::rtl::OUString & inHostName,
                                  ::rtl::OUString & inoutUserName,
                                  ::rtl::OUString & outPassWord,
                                  sal_Bool bCanUseSystemCredentials );
    private:

        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > m_xEnv;
        const rtl::OUString m_aURL;

        rtl::OUString m_aPrevPassword;
        rtl::OUString m_aPrevUsername;
    };

}

#endif
