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

#ifndef INCLUDED_SC_SOURCE_CORE_INC_DOUBLEREF_HXX
#define INCLUDED_SC_SOURCE_CORE_INC_DOUBLEREF_HXX

#include <sal/config.h>

#include <memory>

#include <address.hxx>
#include <types.hxx>

class ScDocument;
struct ScDBQueryParamBase;
struct ScQueryParamBase;
enum class FormulaError : sal_uInt16;

/**
 * Base class for abstracting range data backends for database functions.
 */
class ScDBRangeBase
{
public:
    ScDBRangeBase() = delete;

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
    virtual OUString getString(SCCOL nCol, SCROW nRow) const = 0;

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
    virtual SCCOL findFieldColumn(const OUString& rStr, FormulaError* pErr = nullptr) const = 0;
    virtual std::unique_ptr<ScDBQueryParamBase>
    createQueryParam(const ScDBRangeBase* pQueryRef) const = 0;
    virtual bool isRangeEqual(const ScRange& rRange) const = 0;

protected:
    ScDBRangeBase(ScDocument* pDoc);
    ScDocument* getDoc() const { return mpDoc; }

    /**
     * Populate query options that are always the same for all database
     * queries.
     */
    static void fillQueryOptions(ScQueryParamBase* pParam);

private:
    ScDocument* mpDoc;
};

class ScDBInternalRange final : public ScDBRangeBase
{
public:
    explicit ScDBInternalRange(ScDocument* pDoc, const ScRange& rRange);
    virtual ~ScDBInternalRange() override;

    const ScRange& getRange() const { return maRange; }

    virtual SCCOL getColSize() const override;
    virtual SCROW getRowSize() const override;
    virtual SCSIZE getVisibleDataCellCount() const override;

    /**
     * Get a string value of a specified cell position.  Note that the
     * position of the upper left cell of the range is always (0, 0) even if
     * the reference type is of internal range.
     *
     * @param nCol column position (0 to column size-1)
     * @param nRow row position (0 to row size-1)
     */
    virtual OUString getString(SCCOL nCol, SCROW nRow) const override;

    virtual SCCOL getFirstFieldColumn() const override;
    /**
     * Get a <i>0-based</i> column index that corresponds with the passed field
     * index.  Note that the field index passed as the 1st parameter is
     * <i>1-based.</i>
     *
     * @param nIndex 1-based field index.
     *
     * @return 0-based column index
     */
    virtual SCCOL findFieldColumn(SCCOL nIndex) const override;
    virtual SCCOL findFieldColumn(const OUString& rStr,
                                  FormulaError* pErr = nullptr) const override;
    virtual std::unique_ptr<ScDBQueryParamBase>
    createQueryParam(const ScDBRangeBase* pQueryRef) const override;
    virtual bool isRangeEqual(const ScRange& rRange) const override;

private:
    ScRange maRange;
};

class ScDBExternalRange final : public ScDBRangeBase
{
public:
    explicit ScDBExternalRange(ScDocument* pDoc, const ScMatrixRef& pMat);
    virtual ~ScDBExternalRange() override;

    virtual SCCOL getColSize() const override;
    virtual SCROW getRowSize() const override;
    virtual SCSIZE getVisibleDataCellCount() const override;

    /**
     * Get a string value of a specified cell position.  Note that the
     * position of the upper left cell of the range is always (0, 0) even if
     * the reference type is of internal range.
     *
     * @param nCol column position (0 to column size-1)
     * @param nRow row position (0 to row size-1)
     */
    virtual OUString getString(SCCOL nCol, SCROW nRow) const override;

    virtual SCCOL getFirstFieldColumn() const override;

    /**
     * Get a <i>0-based</i> column index that corresponds with the passed field
     * index.  Note that the field index passed as the 1st parameter is
     * <i>1-based.</i>
     *
     * @param nIndex 1-based field index.
     *
     * @return 0-based column index
     */
    virtual SCCOL findFieldColumn(SCCOL nIndex) const override;
    virtual SCCOL findFieldColumn(const OUString& rStr,
                                  FormulaError* pErr = nullptr) const override;
    virtual std::unique_ptr<ScDBQueryParamBase>
    createQueryParam(const ScDBRangeBase* pQueryRef) const override;
    virtual bool isRangeEqual(const ScRange& rRange) const override;

private:
    const ScMatrixRef mpMatrix;
    SCCOL mnCols;
    SCROW mnRows;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
