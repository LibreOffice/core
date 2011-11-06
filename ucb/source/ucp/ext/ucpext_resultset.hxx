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



#ifndef UCB_UCPEXT_RESULT_SET_HXX
#define UCB_UCPEXT_RESULT_SET_HXX

#include <rtl/ref.hxx>
#include <ucbhelper/resultsethelper.hxx>

//......................................................................................................................
namespace ucb { namespace ucp { namespace ext
{
//......................................................................................................................

    class Content;

    //==================================================================================================================
    //= ResultSet
    //==================================================================================================================
    class ResultSet : public ::ucbhelper::ResultSetImplHelper
    {
    public:
        ResultSet(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& i_rORB,
              const rtl::Reference< Content >& i_rContent,
            const com::sun::star::ucb::OpenCommandArgument2& i_rCommand,
              const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& i_rEnv
        );

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >  m_xEnvironment;
        ::rtl::Reference< Content >                                                     m_xContent;

    private:
        virtual void initStatic();
        virtual void initDynamic();
    };

//......................................................................................................................
} } }   // namespace ucp::ext
//......................................................................................................................

#endif  // UCB_UCPEXT_RESULT_SET_HXX
