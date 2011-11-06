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
#include "precompiled_connectivity.hxx"
#include "charset_s.hxx"

//........................................................................
namespace connectivity
{
//........................................................................

    using namespace ::dbtools;

    //====================================================================
    //= ODataAccessCharSet
    //====================================================================
    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL ODataAccessCharSet::acquire()
    {
        return ORefBase::acquire();
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL ODataAccessCharSet::release()
    {
        return ORefBase::release();
    }

    //--------------------------------------------------------------------
    sal_Int32 ODataAccessCharSet::getSupportedTextEncodings( ::std::vector< rtl_TextEncoding >& _rEncs ) const
    {
        _rEncs.clear();

        OCharsetMap::const_iterator aLoop = m_aCharsetInfo.begin();
        OCharsetMap::const_iterator aLoopEnd = m_aCharsetInfo.end();
        while (aLoop != aLoopEnd)
        {
            _rEncs.push_back( (*aLoop).getEncoding() );
            ++aLoop;
        }

        return _rEncs.size();
    }

//........................................................................
}   // namespace connectivity
//........................................................................

