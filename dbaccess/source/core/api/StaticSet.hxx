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
            m_aSet.push_back(nullptr); // this is the beforefirst record
        }

        virtual void reset(const css::uno::Reference< css::sdbc::XResultSet>& _xDriverSet) override;

        virtual void fillValueRow(ORowSetRow& _rRow,sal_Int32 _nPosition) override;
        // css::sdbcx::XRowLocate
        virtual css::uno::Any getBookmark() override;
        virtual bool moveToBookmark( const css::uno::Any& bookmark ) override;
        virtual sal_Int32 compareBookmarks( const css::uno::Any& first, const css::uno::Any& second ) override;
        virtual bool hasOrderedBookmarks(  ) override;
        virtual sal_Int32 hashBookmark( const css::uno::Any& bookmark ) override;

        bool isBeforeFirst(  );
        bool isAfterLast(  );

        // css::sdbc::XResultSet
        virtual bool next() override;
        virtual void beforeFirst(  ) override;
        virtual void afterLast(  ) override;
        virtual bool first() override;
        virtual bool last() override;
        virtual sal_Int32 getRow(  ) override;
        virtual bool absolute( sal_Int32 row ) override;
        virtual bool previous(  ) override;
        virtual void refreshRow(  ) override;
        virtual bool rowUpdated(  ) override;
        virtual bool rowInserted(  ) override;
        virtual bool rowDeleted(  ) override;
        // css::sdbc::XResultSetUpdate
        virtual void insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) override;
        virtual void deleteRow(const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable  ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
