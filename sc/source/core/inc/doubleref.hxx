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

#ifndef SC_DOUBLEREF_HXX
#define SC_DOUBLEREF_HXX

#include "address.hxx"
#include "scmatrix.hxx"

class ScDocument;
class ScBaseCell;
struct ScDBQueryParamBase;
struct ScQueryParamBase;

// ============================================================================

/**
 * Base class for abstracting range data backends for database functions.
 */
class ScDBRangeBase
{
public:
    enum RefType { INTERNAL, EXTERNAL }; // TODO: We may not need this after all... (kohei)

    virtual ~ScDBRangeBase() = 0;

    bool fillQueryEntries(ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef) const;

    virtual SCCOL getColSize() const = 0;
    virtual SCROW getRowSize() const = 0;
    virtual SCSIZE getVisibleDataCellCount() const = 0;

    /**
     * Get a string value of a specified cell position.  Note that the
     * position of the upper left cell of the range is always (0, 0) even if
     * the reference type is of internal range.
     *
     * @param nCol column position (0 to column size-1)
     * @param nRow row position (0 to row size-1)
     */
    virtual ::rtl::OUString getString(SCCOL nCol, SCROW nRow) const = 0;

    virtual SCCOL getFirstFieldColumn() const = 0;

    /**
     * Get a <i>0-based</i> column index that corresponds with the passed field
     * index.  Note that the field index passed as the 1st parameter is
     * <i>1-based.</i>
     *
     * @param nIndex 1-based field index.
     *
     * @return 0-based column index
     */
    virtual SCCOL findFieldColumn(SCCOL nIndex) const = 0;
    virtual SCCOL findFieldColumn(const ::rtl::OUString& rStr, sal_uInt16* pErr = NULL) const = 0;
    virtual ScDBQueryParamBase* createQueryParam(const ScDBRangeBase* pQueryRef) const = 0;
    virtual bool isRangeEqual(const ScRange& rRange) const = 0;

protected:
    ScDBRangeBase(ScDocument* pDoc, RefType eType);
    ScDocument* getDoc() const;

    /**
     * Populate query options that are always the same for all database
     * queries.
     */
    static void fillQueryOptions(ScQueryParamBase* pParam);

private:
    ScDBRangeBase(); // disabled

    ScDocument* mpDoc;
    RefType meType;
};

// ============================================================================

class ScDBInternalRange : public ScDBRangeBase
{
public:
    explicit ScDBInternalRange(ScDocument* pDoc, const ScRange& rRange);
    virtual ~ScDBInternalRange();

    const ScRange& getRange() const;

    virtual SCCOL getColSize() const;
    virtual SCROW getRowSize() const;
    virtual SCSIZE getVisibleDataCellCount() const;

    /**
     * Get a string value of a specified cell position.  Note that the
     * position of the upper left cell of the range is always (0, 0) even if
     * the reference type is of internal range.
     *
     * @param nCol column position (0 to column size-1)
     * @param nRow row position (0 to row size-1)
     */
    virtual ::rtl::OUString getString(SCCOL nCol, SCROW nRow) const;

    virtual SCCOL getFirstFieldColumn() const;
    /**
     * Get a <i>0-based</i> column index that corresponds with the passed field
     * index.  Note that the field index passed as the 1st parameter is
     * <i>1-based.</i>
     *
     * @param nIndex 1-based field index.
     *
     * @return 0-based column index
     */
    virtual SCCOL findFieldColumn(SCCOL nIndex) const;
    virtual SCCOL findFieldColumn(const ::rtl::OUString& rStr, sal_uInt16* pErr = NULL) const;
    virtual ScDBQueryParamBase* createQueryParam(const ScDBRangeBase* pQueryRef) const;
    virtual bool isRangeEqual(const ScRange& rRange) const;

private:
    ScRange maRange;
};

// ============================================================================

class ScDBExternalRange : public ScDBRangeBase
{
public:
    explicit ScDBExternalRange(ScDocument* pDoc, const ScMatrixRef& pMat);
    virtual ~ScDBExternalRange();

    virtual SCCOL getColSize() const;
    virtual SCROW getRowSize() const;
    virtual SCSIZE getVisibleDataCellCount() const;

    /**
     * Get a string value of a specified cell position.  Note that the
     * position of the upper left cell of the range is always (0, 0) even if
     * the reference type is of internal range.
     *
     * @param nCol column position (0 to column size-1)
     * @param nRow row position (0 to row size-1)
     */
    virtual ::rtl::OUString getString(SCCOL nCol, SCROW nRow) const;

    virtual SCCOL getFirstFieldColumn() const;

    /**
     * Get a <i>0-based</i> column index that corresponds with the passed field
     * index.  Note that the field index passed as the 1st parameter is
     * <i>1-based.</i>
     *
     * @param nIndex 1-based field index.
     *
     * @return 0-based column index
     */
    virtual SCCOL findFieldColumn(SCCOL nIndex) const;
    virtual SCCOL findFieldColumn(const ::rtl::OUString& rStr, sal_uInt16* pErr = NULL) const;
    virtual ScDBQueryParamBase* createQueryParam(const ScDBRangeBase* pQueryRef) const;
    virtual bool isRangeEqual(const ScRange& rRange) const;

private:
    const ScMatrixRef mpMatrix;
    SCCOL mnCols;
    SCROW mnRows;
};

#endif
