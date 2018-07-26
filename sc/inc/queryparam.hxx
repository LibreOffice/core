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

#ifndef INCLUDED_SC_INC_QUERYPARAM_HXX
#define INCLUDED_SC_INC_QUERYPARAM_HXX

#include <unotools/textsearch.hxx>
#include "address.hxx"
#include "types.hxx"

#include <memory>
#include <vector>
#include <ostream>

class SvNumberFormatter;

struct ScDBQueryParamInternal;
struct ScQueryEntry;

namespace svl {

class SharedStringPool;

}

struct ScQueryParamBase
{
    utl::SearchParam::SearchType eSearchType;
    bool            bHasHeader;
    bool            bByRow;
    bool            bInplace;
    bool            bCaseSens;
    bool            bDuplicate;
    bool            mbRangeLookup;      ///< for spreadsheet functions like MATCH, LOOKUP, HLOOKUP, VLOOKUP

    virtual ~ScQueryParamBase();

    virtual bool IsValidFieldIndex() const;

    SC_DLLPUBLIC SCSIZE GetEntryCount() const;
    SC_DLLPUBLIC const ScQueryEntry& GetEntry(SCSIZE n) const;
    SC_DLLPUBLIC ScQueryEntry& GetEntry(SCSIZE n);
    SC_DLLPUBLIC ScQueryEntry& AppendEntry();
    ScQueryEntry* FindEntryByField(SCCOLROW nField, bool bNew);
    std::vector<ScQueryEntry*> FindAllEntriesByField(SCCOLROW nField);
    SC_DLLPUBLIC bool RemoveEntryByField(SCCOLROW nField);
    SC_DLLPUBLIC void RemoveAllEntriesByField(SCCOLROW nField);
    void Resize(size_t nNew);
    void FillInExcelSyntax( svl::SharedStringPool& rPool, const OUString& aCellStr, SCSIZE nIndex,
                            SvNumberFormatter* pFormatter );

protected:
    typedef std::vector<std::unique_ptr<ScQueryEntry>> EntriesType;

public:
    typedef EntriesType::const_iterator const_iterator;

    const_iterator begin() const;
    const_iterator end() const;

protected:
    ScQueryParamBase();
    ScQueryParamBase(const ScQueryParamBase& r);
    ScQueryParamBase& operator=(const ScQueryParamBase& r);

    EntriesType m_Entries;
};

// For use in SAL_DEBUG etc. Output format not guaranteed to be stable.
template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(std::basic_ostream<charT, traits> & stream, const ScQueryParamBase& rParam)
{
    stream << "{" <<
        "searchType=" << rParam.eSearchType <<
        ",hasHeader=" << (rParam.bHasHeader?"YES":"NO") <<
        ",byRow=" << (rParam.bByRow?"YES":"NO") <<
        ",inplace=" << (rParam.bInplace?"YES":"NO") <<
        ",caseSens=" << (rParam.bCaseSens?"YES":"NO") <<
        ",duplicate=" << (rParam.bDuplicate?"YES":"NO") <<
        ",rangeLookup=" << (rParam.mbRangeLookup?"YES":"NO") <<
        "}";

    return stream;
}

struct ScQueryParamTable
{
    SCCOL           nCol1;
    SCROW           nRow1;
    SCCOL           nCol2;
    SCROW           nRow2;
    SCTAB           nTab;

    ScQueryParamTable();
    virtual ~ScQueryParamTable();

    ScQueryParamTable(ScQueryParamTable const &) = default;
    ScQueryParamTable(ScQueryParamTable &&) = default;
    ScQueryParamTable & operator =(ScQueryParamTable const &) = default;
    ScQueryParamTable & operator =(ScQueryParamTable &&) = default;
};

// For use in SAL_DEBUG etc. Output format not guaranteed to be stable.
template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(std::basic_ostream<charT, traits> & stream, const ScQueryParamTable& rParam)
{
    stream << "{" <<
        "col1=" << rParam.nCol1 <<
        ",row1=" << rParam.nRow1 <<
        ",col2=" << rParam.nCol2 <<
        ",row2=" << rParam.nRow2 <<
        ",tab=" << rParam.nTab <<
        "}";

    return stream;
}

struct SC_DLLPUBLIC ScQueryParam : public ScQueryParamBase, public ScQueryParamTable
{
    bool            bDestPers;          // not saved
    SCTAB           nDestTab;
    SCCOL           nDestCol;
    SCROW           nDestRow;

    ScQueryParam();
    ScQueryParam( const ScQueryParam& );
    ScQueryParam( const ScDBQueryParamInternal& r );
    virtual ~ScQueryParam() override;

    ScQueryParam&   operator=  ( const ScQueryParam& );
    bool            operator== ( const ScQueryParam& rOther ) const;
    void            Clear();
    void            ClearDestParams();
    void            MoveToDest();
};

// For use in SAL_DEBUG etc. Output format not guaranteed to be stable.
template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(std::basic_ostream<charT, traits> & stream, const ScQueryParam& rParam)
{
    stream << "{" <<
        "base=" << *static_cast<const ScQueryParamBase*>(&rParam) <<
        ",table=" << *static_cast<const ScQueryParamTable*>(&rParam) <<
        ",destPers=" << (rParam.bDestPers?"YES":"NO") <<
        ",destTab=" << rParam.nDestTab <<
        ",destCol=" << rParam.nDestCol <<
        ",destRow=" << rParam.nDestRow <<
        "}";

    return stream;
}

struct ScDBQueryParamBase : public ScQueryParamBase
{
    enum DataType { INTERNAL, MATRIX };

    SCCOL   mnField;    /// the field in which the values are processed during iteration.
    bool    mbSkipString;

    DataType        GetType() const { return meType;}

    ScDBQueryParamBase() = delete;
    virtual ~ScDBQueryParamBase() override;

protected:
    ScDBQueryParamBase(DataType eType);

private:
    DataType        meType;
};

struct ScDBQueryParamInternal : public ScDBQueryParamBase, public ScQueryParamTable
{
    ScDBQueryParamInternal();
    virtual ~ScDBQueryParamInternal() override;

    virtual bool IsValidFieldIndex() const override;
};

struct ScDBQueryParamMatrix : public ScDBQueryParamBase
{
    ScMatrixRef mpMatrix;

    ScDBQueryParamMatrix();
    virtual ~ScDBQueryParamMatrix() override;

    virtual bool IsValidFieldIndex() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
