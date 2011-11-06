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



#ifndef DBACCESS_INDEXES_HXX_
#define DBACCESS_INDEXES_HXX_

#ifndef CONNECTIVITY_INDEXESHELPER_HXX
#include "connectivity/TIndexes.hxx"
#endif

namespace dbaccess
{
    class OIndexes : public connectivity::OIndexesHelper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xIndexes;
    protected:
        virtual connectivity::sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
        virtual connectivity::sdbcx::ObjectType appendObject( const ::rtl::OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
        virtual void dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName);
    public:
        OIndexes(connectivity::OTableHelper* _pTable,
                 ::osl::Mutex& _rMutex,
                 const ::std::vector< ::rtl::OUString> &_rVector,
                 const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxIndexes
                 ) : connectivity::OIndexesHelper(_pTable,_rMutex,_rVector)
            ,m_xIndexes(_rxIndexes)
        {}

        virtual void SAL_CALL disposing(void);
    };
}

#endif // DBACCESS_INDEXES_HXX_
