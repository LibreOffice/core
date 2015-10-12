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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_STATICSET_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_STATICSET_HXX

#include "CacheSet.hxx"

namespace dbaccess
{
    // is used when nothing is supported by the driver
    // we use a snapshot
    class OStaticSet : public OCacheSet
    {
        ORowSetMatrix           m_aSet;
        ORowSetMatrix::iterator m_aSetIter;
        bool                    m_bEnd;
        bool fetchRow();
        void fillAllRows();
    public:
        explicit OStaticSet(sal_Int32 i_nMaxRows) : OCacheSet(i_nMaxRows)
            , m_aSetIter(m_aSet.end())
            , m_bEnd(false)
        {
            m_aSet.push_back(NULL); // this is the beforefirst record
        }

        virtual void reset(const css::uno::Reference< css::sdbc::XResultSet>& _xDriverSet) override;

        virtual void fillValueRow(ORowSetRow& _rRow,sal_Int32 _nPosition) override;
        // css::sdbcx::XRowLocate
        virtual css::uno::Any SAL_CALL getBookmark() throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL moveToBookmark( const css::uno::Any& bookmark ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual sal_Int32 SAL_CALL compareBookmarks( const css::uno::Any& first, const css::uno::Any& second ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL hasOrderedBookmarks(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual sal_Int32 SAL_CALL hashBookmark( const css::uno::Any& bookmark ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

        // css::sdbc::XResultSet
        virtual bool SAL_CALL next(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL isBeforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL isAfterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual void SAL_CALL beforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual void SAL_CALL afterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL first(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL last(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual sal_Int32 SAL_CALL getRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL absolute( sal_Int32 row ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL previous(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual void SAL_CALL refreshRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL rowUpdated(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL rowInserted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual bool SAL_CALL rowDeleted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        // css::sdbc::XResultSetUpdate
        virtual void SAL_CALL insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        virtual void SAL_CALL updateRow(const ORowSetRow& _rInsertRow,const ORowSetRow& _rOriginalRow,const connectivity::OSQLTable& _xTable  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL deleteRow(const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_STATICSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
