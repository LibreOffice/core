/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "xeroot.hxx"
#include "xerecord.hxx"

class ScDBData;

class XclExpTables : public XclExpRecordBase, protected XclExpRoot
{
public:
                        XclExpTables( const XclExpRoot& rRoot );
    virtual             ~XclExpTables() override;

    void                AppendTable( const ScDBData* pData, sal_Int32 nTableId );
    void                GetHeaderRows( ::std::vector<ScRange>& rRanges ) const;

protected:
    struct Entry
    {
        const ScDBData* mpData;
        sal_Int32       mnTableId;  /// used as [n] in table[n].xml part name.

        Entry( const ScDBData* pData, sal_Int32 nTableId );
    };

    typedef ::std::vector<Entry> TablesType;
    TablesType maTables;

    static void         SaveTableXml( XclExpXmlStream& rStrm, const Entry& rEntry );
};

/** Stores all data for database ranges (tables in Excel speak).
    Only OOXML export, BIFF not implemented.*/
class XclExpTablesManager : protected XclExpRoot
{
public:
    explicit            XclExpTablesManager( const XclExpRoot& rRoot );
    virtual             ~XclExpTablesManager() override;

    void                Initialize();
    rtl::Reference< XclExpTables > GetTablesBySheet( SCTAB nTab );

private:
    typedef ::std::map< SCTAB, rtl::Reference< XclExpTables > > TablesMapType;
    TablesMapType maTablesMap;
};

namespace oox::xls { struct QueryTableModel; }

class XclExpQueryTables : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpQueryTables( const XclExpRoot& rRoot );
    virtual             ~XclExpQueryTables() override;

    void                AppendQueryTable( std::shared_ptr<oox::xls::QueryTableModel> pModel, sal_Int32 nQueryTableId );

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    struct Entry
    {
        std::shared_ptr<oox::xls::QueryTableModel> mpModel;
        sal_Int32       mnQueryTableId;

        Entry( std::shared_ptr<oox::xls::QueryTableModel> pModel, sal_Int32 nId );
    };

    std::vector<Entry>  maQueryTables;

    static void         SaveQueryTableXml( XclExpXmlStream& rStrm, const Entry& rEntry );
};

class XclExpQueryTablesManager : protected XclExpRoot
{
public:
    explicit            XclExpQueryTablesManager( const XclExpRoot& rRoot );
    virtual             ~XclExpQueryTablesManager() override;

    void                Initialize();
    rtl::Reference< XclExpQueryTables > GetQueryTablesBySheet( SCTAB nTab );

private:
    typedef ::std::map< SCTAB, rtl::Reference< XclExpQueryTables > > QueryTablesMapType;
    QueryTablesMapType  maQueryTablesMap;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
