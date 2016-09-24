/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_FILTER_XEPIVOTXML_HXX
#define INCLUDED_SC_FILTER_XEPIVOTXML_HXX

#include <xerecord.hxx>
#include <xeroot.hxx>

#include <memory>
#include <map>
#include <unordered_map>

class ScDPCache;
class ScDPCollection;
class ScDPObject;

class XclExpXmlPivotCaches : public XclExpRecordBase, protected XclExpRoot
{
public:
    enum EntryType { Worksheet, Name, Database };

    struct Entry
    {
        const ScDPCache* mpCache;
        EntryType meType;
        ScRange maSrcRange;
    };

    XclExpXmlPivotCaches( const XclExpRoot& rRoot );
    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

    void SetCaches( const std::vector<Entry>& rCaches );
    bool HasCaches() const;
    const Entry* GetCache( sal_Int32 nCacheId ) const;

private:
    void SavePivotCacheXml( XclExpXmlStream& rStrm, const Entry& rEntry, sal_Int32 nCacheId );

private:
    std::vector<Entry> maCaches;
};

class XclExpXmlPivotTables : public XclExpRecordBase, protected XclExpRoot
{
    struct Entry
    {
        const ScDPObject* mpTable;
        sal_Int32 mnCacheId;
        sal_Int32 mnPivotId; /// used as [n] in pivotTable[n].xml part name.

        Entry( const ScDPObject* pTable, sal_Int32 nCacheId, sal_Int32 nPivotId );
    };

    const XclExpXmlPivotCaches& mrCaches;
    typedef std::vector<Entry> TablesType;
    TablesType maTables;

public:
    XclExpXmlPivotTables( const XclExpRoot& rRoot, const XclExpXmlPivotCaches& rCaches );

    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

    void AppendTable( const ScDPObject* pTable, sal_Int32 nCacheId, sal_Int32 nPivotId );

private:
    void SavePivotTableXml( XclExpXmlStream& rStrm, const ScDPObject& rObj, sal_Int32 nCacheId );
};

class XclExpXmlPivotTableManager : protected XclExpRoot
{
    typedef std::map<SCTAB, std::unique_ptr<XclExpXmlPivotTables>> TablesType;
    typedef std::unordered_map<const ScDPObject*, sal_Int32> CacheIdMapType;
public:
    XclExpXmlPivotTableManager( const XclExpRoot& rRoot );

    void Initialize();

    XclExpXmlPivotCaches& GetCaches();
    XclExpXmlPivotTables* GetTablesBySheet( SCTAB nTab );

private:
    XclExpXmlPivotCaches maCaches;
    TablesType m_Tables;
    CacheIdMapType maCacheIdMap;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
