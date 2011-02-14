/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: interpre.hxx,v $
 * $Revision: 1.35.44.2 $
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

#include <vector>

struct ScDBQueryParamInternal;

struct ScQueryParamBase
{
    bool            bHasHeader;
    bool            bByRow;
    bool            bInplace;
    bool            bCaseSens;
    bool            bRegExp;
    bool            bDuplicate;
    bool            bMixedComparison;   // whether numbers are smaller than strings

    virtual ~ScQueryParamBase();

    SC_DLLPUBLIC SCSIZE GetEntryCount() const;
    SC_DLLPUBLIC ScQueryEntry& GetEntry(SCSIZE n) const;
    void Resize(SCSIZE nNew);
    SC_DLLPUBLIC void DeleteQuery( SCSIZE nPos );
    void FillInExcelSyntax(String& aCellStr, SCSIZE nIndex);

protected:
    ScQueryParamBase();
    ScQueryParamBase(const ScQueryParamBase& r);

    mutable ::std::vector<ScQueryEntry>  maEntries;
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
    sal_Bool            bDestPers;          // nicht gespeichert
    SCTAB           nDestTab;
    SCCOL           nDestCol;
    SCROW           nDestRow;

    ScQueryParam();
    ScQueryParam( const ScQueryParam& r );
    ScQueryParam( const ScDBQueryParamInternal& r );
    virtual ~ScQueryParam();

    ScQueryParam&   operator=   ( const ScQueryParam& r );
    sal_Bool            operator==  ( const ScQueryParam& rOther ) const;
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
};

// ============================================================================

struct ScDBQueryParamMatrix : public ScDBQueryParamBase
{
    ScMatrixRef mpMatrix;

    ScDBQueryParamMatrix();
    virtual ~ScDBQueryParamMatrix();
};

#endif
