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

#include "global.hxx"
#include "types.hxx"

#include <boost/ptr_container/ptr_vector.hpp>

class SvNumberFormatter;

struct ScDBQueryParamInternal;
struct ScQueryEntry;

namespace svl {

class SharedStringPool;

}

struct ScQueryParamBase
{
    bool            bHasHeader;
    bool            bByRow;
    bool            bInplace;
    bool            bCaseSens;
    bool            bRegExp;
    bool            bDuplicate;
    bool            mbRangeLookup;      ///< for spreadsheet functions like MATCH, LOOKUP, HLOOKUP, VLOOKUP

    virtual ~ScQueryParamBase();

    virtual bool IsValidFieldIndex() const;

    SC_DLLPUBLIC SCSIZE GetEntryCount() const;
    SC_DLLPUBLIC const ScQueryEntry& GetEntry(SCSIZE n) const;
    SC_DLLPUBLIC ScQueryEntry& GetEntry(SCSIZE n);
    SC_DLLPUBLIC ScQueryEntry& AppendEntry();
    ScQueryEntry* FindEntryByField(SCCOLROW nField, bool bNew);
    SC_DLLPUBLIC void RemoveEntryByField(SCCOLROW nField);
    void Resize(size_t nNew);
    void FillInExcelSyntax( svl::SharedStringPool& rPool, const OUString& aCellStr, SCSIZE nIndex,
                            SvNumberFormatter* pFormatter );

protected:
    typedef boost::ptr_vector<ScQueryEntry> EntriesType;

public:
    typedef EntriesType::const_iterator const_iterator;

    const_iterator begin() const;
    const_iterator end() const;

protected:
    ScQueryParamBase();
    ScQueryParamBase(const ScQueryParamBase& r);

    EntriesType maEntries;
};

struct ScQueryParamTable
{
    SCCOL           nCol1;
    SCROW           nRow1;
    SCCOL           nCol2;
    SCROW           nRow2;
    SCTAB           nTab;

    ScQueryParamTable();
    ScQueryParamTable(const ScQueryParamTable& r);
    virtual ~ScQueryParamTable();
};

struct SC_DLLPUBLIC ScQueryParam : public ScQueryParamBase, public ScQueryParamTable
{
    bool            bDestPers;          // not saved
    SCTAB           nDestTab;
    SCCOL           nDestCol;
    SCROW           nDestRow;

    ScQueryParam();
    ScQueryParam( const ScQueryParam& r );
    ScQueryParam( const ScDBQueryParamInternal& r );
    virtual ~ScQueryParam();

    ScQueryParam&   operator=  ( const ScQueryParam& r );
    bool            operator== ( const ScQueryParam& rOther ) const;
    void            Clear();
    void            ClearDestParams();
    void            MoveToDest();
};

struct ScDBQueryParamBase : public ScQueryParamBase
{
    enum DataType { INTERNAL, MATRIX };

    SCCOL   mnField;    /// the field in which the values are processed during iteration.
    bool    mbSkipString;

    DataType        GetType() const { return meType;}

    virtual ~ScDBQueryParamBase();

protected:
    ScDBQueryParamBase(DataType eType);

private:
    ScDBQueryParamBase();

    DataType        meType;
};

struct ScDBQueryParamInternal : public ScDBQueryParamBase, public ScQueryParamTable
{
    ScDBQueryParamInternal();
    virtual ~ScDBQueryParamInternal();

    virtual bool IsValidFieldIndex() const override;
};

struct ScDBQueryParamMatrix : public ScDBQueryParamBase
{
    ScMatrixRef mpMatrix;

    ScDBQueryParamMatrix();
    virtual ~ScDBQueryParamMatrix();

    virtual bool IsValidFieldIndex() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
