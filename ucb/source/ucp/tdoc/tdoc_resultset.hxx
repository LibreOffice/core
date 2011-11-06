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



#ifndef INCLUDED_TDOC_RESULTSET_HXX
#define INCLUDED_TDOC_RESULTSET_HXX

#include <rtl/ref.hxx>
#include <ucbhelper/resultsethelper.hxx>

namespace tdoc_ucp {

class Content;

class DynamicResultSet : public ::ucbhelper::ResultSetImplHelper
{
    rtl::Reference< Content > m_xContent;

private:
    virtual void initStatic();
    virtual void initDynamic();

public:
    DynamicResultSet(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            const rtl::Reference< Content >& rxContent,
            const com::sun::star::ucb::OpenCommandArgument2& rCommand );
};

}

#endif /* !INCLUDED_TDOC_RESULTSET_HXX */
