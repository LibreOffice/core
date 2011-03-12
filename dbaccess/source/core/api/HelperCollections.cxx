/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "HelperCollections.hxx"

#include "dbastrings.hrc"

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

    OPrivateColumns::OPrivateColumns(const ::rtl::Reference< ::connectivity::OSQLColumns>& _rColumns,
                        sal_Bool _bCase,
                        ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< ::rtl::OUString> &_rVector,
                        sal_Bool _bUseAsIndex
                    ) : sdbcx::OCollection(_rParent,_bCase,_rMutex,_rVector,_bUseAsIndex)
                        ,m_aColumns(_rColumns)
    {
    }

    OPrivateColumns* OPrivateColumns::createWithIntrinsicNames( const ::rtl::Reference< ::connectivity::OSQLColumns >& _rColumns,
        sal_Bool _bCase, ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex )
    {
        ::std::vector< ::rtl::OUString > aNames; aNames.reserve( _rColumns->get().size() );

        ::rtl::OUString sColumName;
        for (   ::connectivity::OSQLColumns::Vector::const_iterator column = _rColumns->get().begin();
                column != _rColumns->get().end();
                ++column
            )
        {
            Reference< XPropertySet > xColumn( *column, UNO_QUERY_THROW );
            xColumn->getPropertyValue( PROPERTY_NAME ) >>= sColumName;
            aNames.push_back( sColumName );
        }
        return new OPrivateColumns( _rColumns, _bCase, _rParent, _rMutex, aNames, sal_False );
    }

    void SAL_CALL OPrivateColumns::disposing(void)
    {
        m_aColumns = NULL;
        clear_NoDispose();
            // we're not owner of the objects we're holding, instead the object we got in our ctor is
            // So we're not allowed to dispose our elements.
        OPrivateColumns_Base::disposing();
    }

    connectivity::sdbcx::ObjectType OPrivateColumns::createObject(const ::rtl::OUString& _rName)
    {
        if ( m_aColumns.is() )
        {
            ::connectivity::OSQLColumns::Vector::const_iterator aIter = find(m_aColumns->get().begin(),m_aColumns->get().end(),_rName,isCaseSensitive());
            if(aIter == m_aColumns->get().end())
                aIter = findRealName(m_aColumns->get().begin(),m_aColumns->get().end(),_rName,isCaseSensitive());

            if(aIter != m_aColumns->get().end())
                return connectivity::sdbcx::ObjectType(*aIter,UNO_QUERY);

            OSL_FAIL("Column not found in collection!");
        }
        return NULL;
    }

    connectivity::sdbcx::ObjectType OPrivateTables::createObject(const ::rtl::OUString& _rName)
    {
        if ( !m_aTables.empty() )
        {
            OSQLTables::iterator aIter = m_aTables.find(_rName);
            OSL_ENSURE(aIter != m_aTables.end(),"Table not found!");
            OSL_ENSURE(aIter->second.is(),"Table is null!");
            (void)aIter;
            return connectivity::sdbcx::ObjectType(m_aTables.find(_rName)->second,UNO_QUERY);
        }
        return NULL;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
