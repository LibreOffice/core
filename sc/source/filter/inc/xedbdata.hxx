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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XEDBDATA_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XEDBDATA_HXX

#include "xeroot.hxx"
#include "xerecord.hxx"

class ScDBData;
class XclExpTablesManagerImpl;

class XclExpTables : public XclExpRecordBase, protected XclExpRoot
{
public:
                        XclExpTables( const XclExpRoot& rRoot );
    virtual             ~XclExpTables();

    void                AppendTable( const ScDBData* pData, sal_Int32 nTableId );

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
    virtual             ~XclExpTablesManager();

    void                Initialize();
    ::std::shared_ptr< XclExpTables > GetTablesBySheet( SCTAB nTab );

private:
    typedef ::std::map< SCTAB, ::std::shared_ptr< XclExpTables > > TablesMapType;
    TablesMapType maTablesMap;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
