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



#ifndef _CONNECTIVITY_ADO_COLUMNS_HXX_
#define _CONNECTIVITY_ADO_COLUMNS_HXX_

#include "connectivity/sdbcx/VCollection.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "connectivity/sdbcx/IRefreshable.hxx"
#include "ado/Awrapadox.hxx"

namespace connectivity
{
    namespace ado
    {
        class OConnection;
        class OColumns : public sdbcx::OCollection
        {
        protected:
            WpADOColumns    m_aCollection;
            OConnection*    m_pConnection;

            virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
            virtual sdbcx::ObjectType appendObject( const ::rtl::OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
            virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);
        public:
            OColumns(   ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const TStringVector &_rVector,
                        const WpADOColumns& _rCollection,
                        sal_Bool _bCase,
                        OConnection* _pConnection) : sdbcx::OCollection(_rParent,_bCase,_rMutex,_rVector)
                        ,m_aCollection(_rCollection)
                        ,m_pConnection(_pConnection)
            {
            }

        };
    }
}
#endif // _CONNECTIVITY_ADO_COLUMNS_HXX_

