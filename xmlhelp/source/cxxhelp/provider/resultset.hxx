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



#ifndef _RESULTSET_HXX
#define _RESULTSET_HXX

#include <ucbhelper/resultsethelper.hxx>
#include <vos/ref.hxx>

#include "content.hxx"

namespace chelp {

    class ResultSetFactory;

    class DynamicResultSet : public ::ucbhelper::ResultSetImplHelper
    {
        vos::ORef< Content > m_xContent;
        com::sun::star::uno::Reference<
        com::sun::star::ucb::XCommandEnvironment > m_xEnv;
        ResultSetFactory*                    m_pFactory;

    private:
        virtual void initStatic();
        virtual void initDynamic();

    public:
        DynamicResultSet(
            const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            const vos::ORef< Content >& rxContent,
            const com::sun::star::ucb::OpenCommandArgument2& rCommand,
            const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment >& rxEnv,
            ResultSetFactory* pFactory );

        ~DynamicResultSet();
    };

}

#endif

