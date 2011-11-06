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


#ifndef DBA_HELPERCOLLECTIONS_HXX
#define DBA_HELPERCOLLECTIONS_HXX

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include <connectivity/PColumn.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

namespace dbaccess
{
    using namespace dbtools;
    using namespace comphelper;
    using namespace connectivity;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::script;
    using namespace ::cppu;
    using namespace ::osl;
    // -----------------------------------------------------------------------------
    typedef connectivity::sdbcx::OCollection OPrivateColumns_Base;
    class OPrivateColumns : public OPrivateColumns_Base
    {
        ::vos::ORef< ::connectivity::OSQLColumns>   m_aColumns;
    protected:
        virtual connectivity::sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        virtual void impl_refresh() throw(RuntimeException) {}
        virtual Reference< XPropertySet > createDescriptor()
        {
            return NULL;
        }
    public:
        OPrivateColumns(const ::vos::ORef< ::connectivity::OSQLColumns>& _rColumns,
                        sal_Bool _bCase,
                        ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< ::rtl::OUString> &_rVector,
                        sal_Bool _bUseAsIndex = sal_False
                    );

        /** creates a columns instance as above, but taking the names from the columns itself
        */
        static OPrivateColumns* createWithIntrinsicNames(
            const ::vos::ORef< ::connectivity::OSQLColumns >& _rColumns,
            sal_Bool _bCase,
            ::cppu::OWeakObject& _rParent,
            ::osl::Mutex& _rMutex
        );

        virtual void SAL_CALL disposing(void);
    };
    typedef connectivity::sdbcx::OCollection OPrivateTables_BASE;

    //==========================================================================
    //= OPrivateTables
    //==========================================================================
    class OPrivateTables : public OPrivateTables_BASE
    {
        OSQLTables  m_aTables;
    protected:
        virtual connectivity::sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        virtual void impl_refresh() throw(RuntimeException) {}
        virtual Reference< XPropertySet > createDescriptor()
        {
            return NULL;
        }
    public:
        OPrivateTables( const OSQLTables& _rTables,
                        sal_Bool _bCase,
                        ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< ::rtl::OUString> &_rVector
                    ) : sdbcx::OCollection(_rParent,_bCase,_rMutex,_rVector)
                        ,m_aTables(_rTables)
        {
        }
        virtual void SAL_CALL disposing(void)
        {
            clear_NoDispose();
                // we're not owner of the objects we're holding, instead the object we got in our ctor is
                // So we're not allowed to dispose our elements.
            m_aTables.clear();
            OPrivateTables_BASE::disposing();
        }
    };
}
#endif // DBA_HELPERCOLLECTIONS_HXX
