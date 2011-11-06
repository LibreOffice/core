/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
