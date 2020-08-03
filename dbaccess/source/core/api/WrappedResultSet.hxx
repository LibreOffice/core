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

#pragma once

#include "CacheSet.hxx"
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>

namespace dbaccess
{
    // this set is used when we have a bookmarkable set from the driver
    class WrappedResultSet : public OCacheSet
    {
        css::uno::Reference< css::sdbcx::XRowLocate>      m_xRowLocate;
        css::uno::Reference< css::sdbc::XResultSetUpdate> m_xUpd;
        css::uno::Reference< css::sdbc::XRowUpdate>       m_xUpdRow;

        void updateColumn(sal_Int32 nPos, const css::uno::Reference< css::sdbc::XRowUpdate >& _xParameter, const connectivity::ORowSetValue& _rValue);
    public:
        explicit WrappedResultSet(sal_Int32 i_nMaxRows) : OCacheSet(i_nMaxRows)
        {}
        virtual ~WrappedResultSet() override
        {
            m_xRowLocate = nullptr;
        }

        virtual void construct(const css::uno::Reference< css::sdbc::XResultSet>& _xDriverSet,const OUString& i_sRowSetFilter) override;
        virtual void reset(const css::uno::Reference< css::sdbc::XResultSet>& _xDriverSet) override;
        // css::sdbcx::XRowLocate
        virtual css::uno::Any getBookmark() override;
        virtual bool moveToBookmark( const css::uno::Any& bookmark ) override;
        virtual sal_Int32 compareBookmarks( const css::uno::Any& first, const css::uno::Any& second ) override;
        virtual bool hasOrderedBookmarks(  ) override;
        virtual sal_Int32 hashBookmark( const css::uno::Any& bookmark ) override;
        // css::sdbc::XResultSetUpdate
        virtual void insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) override;
        virtual void updateRow(const ORowSetRow& _rInsertRow,const ORowSetRow& _rOriginalRow,const connectivity::OSQLTable& _xTable   ) override;
        virtual void deleteRow(const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable   ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
