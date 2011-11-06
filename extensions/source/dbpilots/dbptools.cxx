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
#include "precompiled_extensions.hxx"
#include "dbptools.hxx"
#include <tools/debug.hxx>

//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;

    //---------------------------------------------------------------------
    void disambiguateName(const Reference< XNameAccess >& _rxContainer, ::rtl::OUString& _rElementsName)
    {
        DBG_ASSERT(_rxContainer.is(), "::dbp::disambiguateName: invalid container!");
        if (!_rxContainer.is())
            return;

        try
        {
            ::rtl::OUString sBase(_rElementsName);
            for (sal_Int32 i=1; i<0x7FFFFFFF; ++i)
            {
                _rElementsName = sBase;
                _rElementsName += ::rtl::OUString::valueOf((sal_Int32)i);
                if (!_rxContainer->hasByName(_rElementsName))
                    return;
            }
            // can't do anything ... no free names
            _rElementsName = sBase;
        }
        catch(Exception&)
        {
            DBG_ERROR("::dbp::disambiguateName: something went (strangely) wrong!");
        }
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

