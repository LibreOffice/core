/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_DATAPROVIDER_DATATRANSFORMATION_HXX
#define INCLUDED_SC_SOURCE_UI_DATAPROVIDER_DATATRANSFORMATION_HXX

#include <types.hxx>
#include <scdllapi.h>

#include <sortparam.hxx>

#include <set>

class ScDocument;

namespace sc {

enum class TransformationType
{
    MERGE_TRANSFORMATION,
    SPLIT_TRANSFORMATION,
    DELETE_TRANSFORMATION,
    SORT_TRANSFORMATION,
    TEXT_TRANSFORMATION,
    AGGREGATE_FUNCTION,
    NUMBER_TRANSFORMATION,
    REMOVE_NULL_TRANSFORMATION,
    DATETIME_TRANSFORMATION
};

enum class TEXT_TRANSFORM_TYPE { TO_LOWER, TO_UPPER, CAPITALIZE, TRIM };

enum class AGGREGATE_FUNCTION { SUM, AVERAGE, MIN, MAX };

enum class NUMBER_TRANSFORM_TYPE { ROUND, ROUND_UP, ROUND_DOWN, ABSOLUTE, LOG_E, LOG_10, CUBE,
    SQUARE, SQUARE_ROOT, EXPONENT, IS_EVEN, IS_ODD, SIGN };

enum class DATETIME_TRANSFORMATION_TYPE { DATE_STRING, YEAR, START_OF_YEAR, END_OF_YEAR, MONTH,
    MONTH_NAME, START_OF_MONTH, END_OF_MONTH, DAY, DAY_OF_WEEK, DAY_OF_YEAR, QUARTER, START_OF_QUARTER,
    END_OF_QUARTER, TIME, HOUR, MINUTE, SECOND };

class SC_DLLPUBLIC DataTransformation
{
protected:

    static SCROW getLastRow(const ScDocument& rDoc, SCCOL nCol);

public:
    virtual ~DataTransformation();

    virtual void Transform(ScDocument& rDoc) const = 0;

    virtual TransformationType getTransformationType() const = 0;

};

class SC_DLLPUBLIC ColumnRemoveTransformation : public DataTransformation
{
    std::set<SCCOL> const maColumns;

public:

    ColumnRemoveTransformation(const std::set<SCCOL>& rColumns);
    virtual ~ColumnRemoveTransformation() override;
    virtual void Transform(ScDocument& rDoc) const override;
    virtual TransformationType getTransformationType() const override;
    const std::set<SCCOL> & getColumns() const;
};

class SC_DLLPUBLIC SplitColumnTransformation : public DataTransformation
{
    SCCOL const mnCol;
    sal_Unicode const mcSeparator;

public:

    SplitColumnTransformation(SCCOL nCol, sal_Unicode cSeparator);
    virtual void Transform(ScDocument& rDoc) const override;
    virtual TransformationType getTransformationType() const override;
    SCCOL getColumn() const;
    sal_Unicode getSeparator() const;
};

class SC_DLLPUBLIC MergeColumnTransformation : public DataTransformation
{
    std::set<SCCOL> const maColumns;
    OUString maMergeString;

public:

    MergeColumnTransformation(const std::set<SCCOL>& rColumns, const OUString& rMergeString);
    virtual void Transform(ScDocument& rDoc) const  override;
    virtual TransformationType getTransformationType() const override;
    const OUString & getMergeString() const;
    const std::set<SCCOL> & getColumns() const;
};

class SC_DLLPUBLIC SortTransformation : public DataTransformation
{
    ScSortParam const maSortParam;
public:

    SortTransformation(const ScSortParam& rParam);
    virtual void Transform(ScDocument& rDoc) const override;
    virtual TransformationType getTransformationType() const override;
    const ScSortParam & getSortParam() const;
};

class SC_DLLPUBLIC TextTransformation : public DataTransformation
{
    std::set<SCCOL> const mnCol;
    TEXT_TRANSFORM_TYPE const maType;

    public:
    TextTransformation(const std::set<SCCOL>& nCol, const TEXT_TRANSFORM_TYPE rType);
    virtual void Transform(ScDocument& rDoc) const override;
    virtual TransformationType getTransformationType() const override;
    TEXT_TRANSFORM_TYPE getTextTransformationType() const;
    const std::set<SCCOL>& getColumns() const;
};

class SC_DLLPUBLIC AggregateFunction : public DataTransformation
{
    std::set<SCCOL> const maColumns;
    AGGREGATE_FUNCTION const maType;

    public:
    AggregateFunction(const std::set<SCCOL>& rColumns, const AGGREGATE_FUNCTION rType);
    virtual void Transform(ScDocument& rDoc) const override;
    virtual TransformationType getTransformationType() const override;
    AGGREGATE_FUNCTION getAggregateType() const;
    const std::set<SCCOL>& getColumns() const;
};

class SC_DLLPUBLIC NumberTransformation : public DataTransformation
{
    std::set<SCCOL> const mnCol;
    NUMBER_TRANSFORM_TYPE const maType;
    int const maPrecision;

    public:
        NumberTransformation(const std::set<SCCOL>& nCol, const NUMBER_TRANSFORM_TYPE rType);
        NumberTransformation(const std::set<SCCOL>& nCol, const NUMBER_TRANSFORM_TYPE rType,
                             int nPrecision);
        virtual void Transform(ScDocument& rDoc) const override;
        virtual TransformationType getTransformationType() const override;
        NUMBER_TRANSFORM_TYPE getNumberTransfromationType() const;
        int getPrecision() const;
        const std::set<SCCOL>& getColumn() const;
};

class SC_DLLPUBLIC ReplaceNullTransformation : public DataTransformation
{
    std::set<SCCOL> const mnCol;
    OUString const msReplaceWith;

    public:
        ReplaceNullTransformation(const std::set<SCCOL>& nCol, const OUString& sReplaceWith);
        virtual void Transform(ScDocument& rDoc) const override;
        virtual TransformationType getTransformationType() const override;
        const std::set<SCCOL>& getColumn() const;
        const OUString& getReplaceString() const;
};

class SC_DLLPUBLIC DateTimeTransformation : public DataTransformation
{
    std::set<SCCOL> const mnCol;
    DATETIME_TRANSFORMATION_TYPE const maType;

    public:
        DateTimeTransformation(const std::set<SCCOL>& nCol,
                               const DATETIME_TRANSFORMATION_TYPE rType);
        virtual void Transform(ScDocument& rDoc) const override;
        virtual TransformationType getTransformationType() const override;
        DATETIME_TRANSFORMATION_TYPE getDateTimeTransfromationType() const;
        const std::set<SCCOL>& getColumn() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
