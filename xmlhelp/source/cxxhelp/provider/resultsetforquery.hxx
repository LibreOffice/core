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


#ifndef _RESULTSETFORQUERY_HXX
#define _RESULTSETFORQUERY_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>

#include "resultsetbase.hxx"
#include "urlparameter.hxx"

namespace chelp {

    class Databases;

    class ResultSetForQuery
        : public ResultSetBase
    {
    public:

        ResultSetForQuery( const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>&  xMSF,
                           const com::sun::star::uno::Reference<com::sun::star::ucb::XContentProvider>&  xProvider,
                           sal_Int32 nOpenMode,
                           const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& seq,
                           const com::sun::star::uno::Sequence< com::sun::star::ucb::NumberedSortingInfo >& seqSort,
                           URLParameter& aURLParameter,
                           Databases* pDatabases );


    private:

        Databases*   m_pDatabases;
        URLParameter m_aURLParameter;
    };

}


#endif
