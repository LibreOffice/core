/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HelperCollections.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:02:11 $
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
#include "HelperCollections.hxx"
#endif

#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
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
    OPrivateColumns::OPrivateColumns(const ::vos::ORef< ::connectivity::OSQLColumns>& _rColumns,
                        sal_Bool _bCase,
                        ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< ::rtl::OUString> &_rVector,
                        sal_Bool _bUseAsIndex
                    ) : sdbcx::OCollection(_rParent,_bCase,_rMutex,_rVector,_bUseAsIndex)
                        ,m_aColumns(_rColumns)
    {
    }

    // -------------------------------------------------------------------------
    OPrivateColumns* OPrivateColumns::createWithIntrinsicNames( const ::vos::ORef< ::connectivity::OSQLColumns >& _rColumns,
        sal_Bool _bCase, ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex )
    {
        ::std::vector< ::rtl::OUString > aNames; aNames.reserve( _rColumns->size() );

        ::rtl::OUString sColumName;
        for (   ::connectivity::OSQLColumns::const_iterator column = _rColumns->begin();
                column != _rColumns->end();
                ++column
            )
        {
            Reference< XPropertySet > xColumn( *column, UNO_QUERY_THROW );
            xColumn->getPropertyValue( PROPERTY_NAME ) >>= sColumName;
            aNames.push_back( sColumName );
        }
        return new OPrivateColumns( _rColumns, _bCase, _rParent, _rMutex, aNames, sal_False );
    }

    // -------------------------------------------------------------------------
    void SAL_CALL OPrivateColumns::disposing(void)
    {
        m_aColumns = NULL;
        clear_NoDispose();
            // we're not owner of the objects we're holding, instead the object we got in our ctor is
            // So we're not allowed to dispose our elements.
        OPrivateColumns_Base::disposing();
    }
    // -------------------------------------------------------------------------
    connectivity::sdbcx::ObjectType OPrivateColumns::createObject(const ::rtl::OUString& _rName)
    {
        if ( m_aColumns.isValid() )
        {
            ::connectivity::OSQLColumns::const_iterator aIter = find(m_aColumns->begin(),m_aColumns->end(),_rName,isCaseSensitive());
            if(aIter == m_aColumns->end())
                aIter = findRealName(m_aColumns->begin(),m_aColumns->end(),_rName,isCaseSensitive());

            if(aIter != m_aColumns->end())
                return connectivity::sdbcx::ObjectType(*aIter,UNO_QUERY);

            OSL_ENSURE(0,"Column not found in collection!");
        }
        return NULL;
    }
    // -------------------------------------------------------------------------
    connectivity::sdbcx::ObjectType OPrivateTables::createObject(const ::rtl::OUString& _rName)
    {
        if ( !m_aTables.empty() )
        {
            OSQLTables::iterator aIter = m_aTables.find(_rName);
            OSL_ENSURE(aIter != m_aTables.end(),"Table not found!");
            OSL_ENSURE(aIter->second.is(),"Table is null!");
            return connectivity::sdbcx::ObjectType(m_aTables.find(_rName)->second,UNO_QUERY);
        }
        return NULL;
    }
    // -----------------------------------------------------------------------------
}
