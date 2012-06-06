/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_QUERYPARAM_HXX
#define SC_QUERYPARAM_HXX

#include "global.hxx"
#include "scmatrix.hxx"

#include <boost/ptr_container/ptr_vector.hpp>

struct ScDBQueryParamInternal;
struct ScQueryEntry;

struct ScQueryParamBase
{
    bool            bHasHeader;
    bool            bByRow;
    bool            bInplace;
    bool            bCaseSens;
    bool            bRegExp;
    bool            bDuplicate;

    virtual ~ScQueryParamBase();

    virtual bool IsValidFieldIndex() const;

    SC_DLLPUBLIC SCSIZE GetEntryCount() const;
    SC_DLLPUBLIC const ScQueryEntry& GetEntry(SCSIZE n) const;
    SC_DLLPUBLIC ScQueryEntry& GetEntry(SCSIZE n);
    SC_DLLPUBLIC ScQueryEntry& AppendEntry();
    ScQueryEntry* FindEntryByField(SCCOLROW nField, bool bNew);
    SC_DLLPUBLIC void RemoveEntryByField(SCCOLROW nField);
    void Resize(size_t nNew);
    void FillInExcelSyntax(const rtl::OUString& aCellStr, SCSIZE nIndex);

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

// ============================================================================

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

// ============================================================================

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

// ============================================================================

struct ScDBQueryParamBase : public ScQueryParamBase
{
    enum DataType { INTERNAL, MATRIX };

    SCCOL   mnField;    /// the field in which the values are processed during iteration.
    bool    mbSkipString;

    DataType        GetType() const;

    virtual ~ScDBQueryParamBase();

protected:
    ScDBQueryParamBase(DataType eType);

private:
    ScDBQueryParamBase();

    DataType        meType;
};

// ============================================================================

struct ScDBQueryParamInternal : public ScDBQueryParamBase, public ScQueryParamTable
{
    ScDBQueryParamInternal();
    virtual ~ScDBQueryParamInternal();

    virtual bool IsValidFieldIndex() const;
};

// ============================================================================

struct ScDBQueryParamMatrix : public ScDBQueryParamBase
{
    ScMatrixRef mpMatrix;

    ScDBQueryParamMatrix();
    virtual ~ScDBQueryParamMatrix();

    virtual bool IsValidFieldIndex() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
