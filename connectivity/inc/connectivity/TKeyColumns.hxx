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



#ifndef CONNECTIVITY_TKEYCOLUMNS_HXX
#define CONNECTIVITY_TKEYCOLUMNS_HXX

#include "connectivity/sdbcx/VCollection.hxx"
#include "connectivity/TKey.hxx"
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS OKeyColumnsHelper : public connectivity::sdbcx::OCollection
    {
        OTableKeyHelper* m_pKey;
    protected:
        virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
        virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
    public:
        OKeyColumnsHelper(  OTableKeyHelper* _pKey,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< ::rtl::OUString> &_rVector);
    };
}
#endif // CONNECTIVITY_TKEYCOLUMNS_HXX
