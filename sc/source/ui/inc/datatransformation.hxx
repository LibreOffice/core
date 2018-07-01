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

#include <address.hxx>
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
    TEXT_TRANSFORMATION
};

enum class TEXT_TRANSFORM_TYPE { TO_LOWER, TO_UPPER, CAPITALIZE, TRIM };

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
    std::set<SCCOL> maColumns;

public:

    ColumnRemoveTransformation(const std::set<SCCOL>& rColumns);
    virtual ~ColumnRemoveTransformation() override;
    virtual void Transform(ScDocument& rDoc) const override;
    virtual TransformationType getTransformationType() const override;
    std::set<SCCOL> getColumns() const;
};

class SC_DLLPUBLIC SplitColumnTransformation : public DataTransformation
{
    SCCOL mnCol;
    sal_Unicode mcSeparator;

public:

    SplitColumnTransformation(SCCOL nCol, sal_Unicode cSeparator);
    virtual void Transform(ScDocument& rDoc) const override;
    virtual TransformationType getTransformationType() const override;
    SCCOL getColumn() const;
    sal_Unicode getSeparator() const;
};

class SC_DLLPUBLIC MergeColumnTransformation : public DataTransformation
{
    std::set<SCCOL> maColumns;
    OUString maMergeString;

public:

    MergeColumnTransformation(const std::set<SCCOL>& rColumns, const OUString& rMergeString);
    virtual void Transform(ScDocument& rDoc) const  override;
    virtual TransformationType getTransformationType() const override;
    OUString getMergeString() const;
    std::set<SCCOL> getColumns() const;
};

class SC_DLLPUBLIC SortTransformation : public DataTransformation
{
    ScSortParam maSortParam;
public:

    SortTransformation(const ScSortParam& rParam);
    virtual void Transform(ScDocument& rDoc) const override;
    virtual TransformationType getTransformationType() const override;
    ScSortParam getSortParam() const;
};

class SC_DLLPUBLIC TextTransformation : public DataTransformation
{
    SCCOL mnCol;
    TEXT_TRANSFORM_TYPE maType;

    public:
    TextTransformation(SCCOL nCol, const TEXT_TRANSFORM_TYPE rType);
    virtual void Transform(ScDocument& rDoc) const override;
    virtual TransformationType getTransformationType() const override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
