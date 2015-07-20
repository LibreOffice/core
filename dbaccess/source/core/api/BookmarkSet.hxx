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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_BOOKMARKSET_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_BOOKMARKSET_HXX

#include "CacheSet.hxx"
#include <com/sun/star/sdbc/XRowUpdate.hpp>

namespace dbaccess
{
    // this set is used when we have a bookmarkable set from the driver
    class OBookmarkSet : public OCacheSet
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate> m_xRowLocate;

        void updateColumn(sal_Int32 nPos,::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowUpdate > _xParameter,const connectivity::ORowSetValue& _rValue);
    public:
        explicit OBookmarkSet(sal_Int32 i_nMaxRows) : OCacheSet(i_nMaxRows)
        {}
        virtual ~OBookmarkSet()
        {
            m_xRowLocate = NULL;
        }

        virtual void construct(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet,const OUString& i_sRowSetFilter) SAL_OVERRIDE;
        virtual void reset(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet) SAL_OVERRIDE;
        virtual void fillValueRow(ORowSetRow& _rRow,sal_Int32 _nPosition) SAL_OVERRIDE;
        // ::com::sun::star::sdbcx::XRowLocate
        virtual ::com::sun::star::uno::Any SAL_CALL getBookmark() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
        virtual bool SAL_CALL moveToBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
        virtual bool SAL_CALL hasOrderedBookmarks(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL hashBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
        // ::com::sun::star::sdbc::XResultSetUpdate
        virtual void SAL_CALL insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL updateRow(const ORowSetRow& _rInsertRow,const ORowSetRow& _rOriginalRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL deleteRow(const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_BOOKMARKSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
