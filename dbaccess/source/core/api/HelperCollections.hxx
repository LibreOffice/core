/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HelperCollections.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:02:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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
