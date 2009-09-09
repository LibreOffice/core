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

struct ScQueryParamBase
{
    enum DataType { INTERNAL, MATRIX };

    bool            bHasHeader;
    bool            bByRow;
    bool            bInplace;
    bool            bCaseSens;
    bool            bRegExp;
    bool            bDuplicate;

    virtual ~ScQueryParamBase();

    SCSIZE          GetEntryCount() const           { return nEntryCount; }
    ScQueryEntry&   GetEntry(SCSIZE n) const        { return pEntries[n]; }
    void            Resize(SCSIZE nNew);
    SC_DLLPUBLIC void DeleteQuery( SCSIZE nPos );
    void            FillInExcelSyntax(String& aCellStr, SCSIZE nIndex);

    DataType        GetType() const;

protected:
    ScQueryParamBase(DataType eType);
    ScQueryParamBase(const ScQueryParamBase& r);

    SCSIZE          nEntryCount;
    ScQueryEntry*   pEntries;

private:
    ScQueryParamBase();
    DataType        meType;
};

// ============================================================================

struct SC_DLLPUBLIC ScQueryParam : public ScQueryParamBase
{
    SCCOL           nCol1;
    SCROW           nRow1;
    SCCOL           nCol2;
    SCROW           nRow2;
    SCTAB           nTab;
    BOOL            bMixedComparison;   // whether numbers are smaller than strings
    BOOL            bDestPers;          // nicht gespeichert
    SCTAB           nDestTab;
    SCCOL           nDestCol;
    SCROW           nDestRow;

    ScQueryParam();
    ScQueryParam( const ScQueryParam& r );
    virtual ~ScQueryParam();

    ScQueryParam&   operator=   ( const ScQueryParam& r );
    BOOL            operator==  ( const ScQueryParam& rOther ) const;
    void            Clear       ();
    void            MoveToDest();
};

// ============================================================================

struct ScQueryParamMatrix : public ScQueryParamBase
{
    ScMatrixRef mpMatrix;

    ScQueryParamMatrix();
    virtual ~ScQueryParamMatrix();
};

#endif
