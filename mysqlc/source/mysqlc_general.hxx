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

#ifndef _MYSQLC_GENERAL_
#define _MYSQLC_GENERAL_

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

#include <preextstl.h>
#include <cppconn/exception.h>
#include <postextstl.h>

namespace mysqlc_sdbc_driver
{
    rtl::OUString getStringFromAny(const ::com::sun::star::uno::Any& _rAny);

    void throwFeatureNotImplementedException(
            const sal_Char* _pAsciiFeatureName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
            const ::com::sun::star::uno::Any* _pNextException = NULL
        )
        throw (::com::sun::star::sdbc::SQLException);

    void throwInvalidArgumentException(
            const sal_Char* _pAsciiFeatureName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext,
            const ::com::sun::star::uno::Any* _pNextException = NULL
        )
        throw (::com::sun::star::sdbc::SQLException);

    void translateAndThrow(const ::sql::SQLException& _error, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _context, const rtl_TextEncoding encoding);

    int mysqlToOOOType(int mysqlType) throw ();


    ::rtl::OUString convert(const ::ext_std::string& _string, const rtl_TextEncoding encoding);

    ::ext_std::string convert(const ::rtl::OUString& _string, const rtl_TextEncoding encoding);
}

#endif
