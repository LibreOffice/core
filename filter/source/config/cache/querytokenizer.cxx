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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "querytokenizer.hxx"

//_______________________________________________
// includes

//_______________________________________________
// namespace

namespace filter{
    namespace config{

//_______________________________________________
// definitions

/*-----------------------------------------------
    01.08.2003 10:42
-----------------------------------------------*/
QueryTokenizer::QueryTokenizer(const ::rtl::OUString& sQuery)
    : m_bValid(sal_True)
{
    sal_Int32 token = 0;
    while(token != -1)
    {
        ::rtl::OUString sToken = sQuery.getToken(0, ':', token);
        if (sToken.getLength())
        {
            sal_Int32 equal = sToken.indexOf('=');

            if (equal == 0)
                m_bValid = sal_False;
            OSL_ENSURE(m_bValid, "QueryTokenizer::QueryTokenizer()\nFound non boolean query parameter ... but its key is empty. Will be ignored!\n");

            ::rtl::OUString sKey;
            ::rtl::OUString sVal;

            sKey = sToken;
            if (equal > 0)
            {
                sKey = sToken.copy(0      , equal                       );
                sVal = sToken.copy(equal+1, sToken.getLength()-(equal+1));
            }

            if (find(sKey) != end())
                m_bValid = sal_False;
            OSL_ENSURE(m_bValid, "QueryTokenizer::QueryTokenizer()\nQuery contains same param more then once. Last one wins :-)\n");

            (*this)[sKey] = sVal;
        }
    }
}

/*-----------------------------------------------
    01.08.2003 10:28
-----------------------------------------------*/
QueryTokenizer::~QueryTokenizer()
{
    /*TODO*/
}

/*-----------------------------------------------
    01.08.2003 10:53
-----------------------------------------------*/
sal_Bool QueryTokenizer::valid() const
{
    return m_bValid;
}

    } // namespace config
} // namespace filter
