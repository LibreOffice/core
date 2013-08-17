/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef DBA_HELPERCOLLECTIONS_HXX
#define DBA_HELPERCOLLECTIONS_HXX

#include "connectivity/sdbcx/VCollection.hxx"
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>
#include <connectivity/PColumn.hxx>
#include <rtl/ref.hxx>

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

    typedef connectivity::sdbcx::OCollection OPrivateColumns_Base;
    class OPrivateColumns : public OPrivateColumns_Base
    {
        ::rtl::Reference< ::connectivity::OSQLColumns>  m_aColumns;
    protected:
        virtual connectivity::sdbcx::ObjectType createObject(const OUString& _rName);
        virtual void impl_refresh() throw(RuntimeException) {}
        virtual Reference< XPropertySet > createDescriptor()
        {
            return NULL;
        }
    public:
        OPrivateColumns(const ::rtl::Reference< ::connectivity::OSQLColumns>& _rColumns,
                        sal_Bool _bCase,
                        ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< OUString> &_rVector,
                        sal_Bool _bUseAsIndex = sal_False
                    );

        /** creates a columns instance as above, but taking the names from the columns itself
        */
        static OPrivateColumns* createWithIntrinsicNames(
            const ::rtl::Reference< ::connectivity::OSQLColumns >& _rColumns,
            sal_Bool _bCase,
            ::cppu::OWeakObject& _rParent,
            ::osl::Mutex& _rMutex
        );

        virtual void SAL_CALL disposing(void);
    };
    typedef connectivity::sdbcx::OCollection OPrivateTables_BASE;

    // OPrivateTables
    class OPrivateTables : public OPrivateTables_BASE
    {
        OSQLTables  m_aTables;
    protected:
        virtual connectivity::sdbcx::ObjectType createObject(const OUString& _rName);
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
                        const ::std::vector< OUString> &_rVector
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
