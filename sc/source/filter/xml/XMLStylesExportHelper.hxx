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

#pragma once

#include <vector>
#include <memory>
#include <list>

#include <address.hxx>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>
#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>

#include <mdds/flat_segment_tree.hpp>

class ScDocument;
class ScXMLExport;

struct ScMyValidation
{
    OUString               sName;
    OUString               sErrorMessage;
    OUString               sErrorTitle;
    OUString               sInputMessage;
    OUString               sInputTitle;
    OUString               sFormula1;
    OUString               sFormula2;
    ScAddress              aBaseCell;
    css::sheet::ValidationAlertStyle aAlertStyle;
    css::sheet::ValidationType       aValidationType;
    css::sheet::ConditionOperator    aOperator;
    sal_Int16                   nShowList;
    bool                        bShowErrorMessage;
    bool                        bShowInputMessage;
    bool                        bIgnoreBlanks;
    bool                        bCaseSensitive;

                                ScMyValidation();

    bool                        IsEqual(const ScMyValidation& aVal) const;
};

class ScMyValidationsContainer
{
private:
    std::vector<ScMyValidation> aValidationVec;
public:
                           ScMyValidationsContainer();
                           ~ScMyValidationsContainer();
    void                   AddValidation(const css::uno::Any& aAny,
                                    sal_Int32& nValidationIndex);
    static OUString        GetCondition(ScXMLExport& rExport, const ScMyValidation& aValidation);
    static OUString        GetBaseCellAddress(const ScDocument* pDoc, const ScAddress& aCell);
    static void            WriteMessage(ScXMLExport& rExport,
                                    const OUString& sTitle, const OUString& sMessage,
                                    const bool bShowMessage, const bool bIsHelpMessage);
    void                   WriteValidations(ScXMLExport& rExport);
    const OUString&        GetValidationName(const sal_Int32 nIndex);
};

struct ScMyDefaultStyle
{
    sal_Int32   nIndex;
    sal_Int32   nRepeat;
    bool        bIsAutoStyle;

    ScMyDefaultStyle() : nIndex(-1), nRepeat(1),
        bIsAutoStyle(true) {}
};

typedef std::vector<ScMyDefaultStyle> ScMyDefaultStyleList;

class ScFormatRangeStyles;

class ScMyDefaultStyles
{
    ScMyDefaultStyleList maColDefaults;

    static sal_Int32 GetStyleNameIndex(const ScFormatRangeStyles* pCellStyles,
        const sal_Int32 nTable, const sal_Int32 nPos,
        const sal_Int32 i, bool& bIsAutoStyle);
public:

    void FillDefaultStyles(const sal_Int32 nTable,
        const sal_Int32 nLastRow, const sal_Int32 nLastCol,
        const ScFormatRangeStyles* pCellStyles, ScDocument* pDoc);

    const ScMyDefaultStyleList& GetColDefaults() const { return maColDefaults; }
};

struct ScMyRowFormatRange
{
    sal_Int32   nStartColumn;
    sal_Int32   nRepeatColumns;
    sal_Int32   nRepeatRows;
    sal_Int32   nIndex;
    sal_Int32   nValidationIndex;
    bool        bIsAutoStyle;

    ScMyRowFormatRange();
    bool operator<(const ScMyRowFormatRange& rRange) const;
};

class ScRowFormatRanges
{
    typedef std::list<ScMyRowFormatRange> ScMyRowFormatRangesList;
    ScMyRowFormatRangesList     aRowFormatRanges;
    const ScMyDefaultStyleList* pColDefaults;
    sal_uInt32                  nSize;

    void AddRange(const sal_Int32 nPrevStartCol, const sal_Int32 nRepeat, const sal_Int32 nPrevIndex,
        const bool bPrevAutoStyle, const ScMyRowFormatRange& rFormatRange);

public:
    ScRowFormatRanges();
    explicit ScRowFormatRanges(const ScRowFormatRanges* pRanges);
    ~ScRowFormatRanges();

    void SetColDefaults(const ScMyDefaultStyleList* pDefaults) { pColDefaults = pDefaults; }
    void Clear();
    void AddRange(const ScMyRowFormatRange& rFormatRange);
    bool GetNext(ScMyRowFormatRange& rFormatRange);
    sal_Int32 GetMaxRows() const;
    sal_Int32 GetSize() const { return nSize;}
    void Sort();
};


struct ScMyFormatRange
{
    css::table::CellRangeAddress aRangeAddress;
    sal_Int32                    nStyleNameIndex;
    sal_Int32                    nValidationIndex;
    sal_Int32                    nNumberFormat;
    bool                         bIsAutoStyle;

    ScMyFormatRange();
    bool operator< (const ScMyFormatRange& rRange) const;
};

class ScFormatRangeStyles
{
    typedef std::list<ScMyFormatRange>          ScMyFormatRangeAddresses;
    typedef std::vector<ScMyFormatRangeAddresses> ScMyFormatRangeListVec;

    ScMyFormatRangeListVec      aTables;
    std::vector<OUString>       aStyleNames;
    std::vector<OUString>       aAutoStyleNames;
    const ScMyDefaultStyleList* pColDefaults;

public:
    ScFormatRangeStyles();
    ~ScFormatRangeStyles();

    void SetColDefaults(const ScMyDefaultStyleList* pDefaults) { pColDefaults = pDefaults; }
    void AddNewTable(const sal_Int32 nTable);
    bool AddStyleName(const OUString& rString, sal_Int32& rIndex, const bool bIsAutoStyle = true);
    sal_Int32 GetIndexOfStyleName(std::u16string_view rString, std::u16string_view rPrefix, bool& bIsAutoStyle);
    // does not delete ranges
    sal_Int32 GetStyleNameIndex(const sal_Int32 nTable, const sal_Int32 nColumn, const sal_Int32 nRow,
        bool& bIsAutoStyle) const;
    // deletes not necessary ranges if wanted
    sal_Int32 GetStyleNameIndex(const sal_Int32 nTable, const sal_Int32 nColumn, const sal_Int32 nRow,
        bool& bIsAutoStyle, sal_Int32& nValidationIndex, sal_Int32& nNumberFormat, const sal_Int32 nRemoveBeforeRow);
    void GetFormatRanges(const sal_Int32 nStartColumn, const sal_Int32 nEndColumn, const sal_Int32 nRow,
                    const sal_Int32 nTable, ScRowFormatRanges* pFormatRanges);
    void AddRangeStyleName(const css::table::CellRangeAddress& rCellRangeAddress, const sal_Int32 nStringIndex,
                    const bool bIsAutoStyle, const sal_Int32 nValidationIndex, const sal_Int32 nNumberFormat);
    OUString& GetStyleNameByIndex(const sal_Int32 nIndex, const bool bIsAutoStyle);
    void Sort();
};

class ScColumnRowStylesBase
{
    std::vector<OUString>   aStyleNames;

public:
    ScColumnRowStylesBase();
    virtual ~ScColumnRowStylesBase();

    virtual void AddNewTable(const sal_Int32 nTable, const sal_Int32 nFields) = 0;
    sal_Int32 AddStyleName(const OUString & rString);
    sal_Int32 GetIndexOfStyleName(std::u16string_view rString, std::u16string_view rPrefix);
    OUString& GetStyleNameByIndex(const sal_Int32 nIndex);
};

struct ScColumnStyle
{
    sal_Int32   nIndex;
    bool        bIsVisible;

    ScColumnStyle() : nIndex(-1), bIsVisible(true) {}
};

class ScColumnStyles : public ScColumnRowStylesBase
{
    typedef std::vector<ScColumnStyle>  ScMyColumnStyleVec;
    typedef std::vector<ScMyColumnStyleVec> ScMyColumnVectorVec;
    ScMyColumnVectorVec             aTables;

public:
    ScColumnStyles();
    virtual ~ScColumnStyles() override;

    virtual void AddNewTable(const sal_Int32 nTable, const sal_Int32 nFields) override;
    sal_Int32 GetStyleNameIndex(const sal_Int32 nTable, const sal_Int32 nField,
        bool& bIsVisible);
    void AddFieldStyleName(const sal_Int32 nTable, const sal_Int32 nField, const sal_Int32 nStringIndex, const bool bIsVisible);
};

class ScRowStyles : public ScColumnRowStylesBase
{
    typedef ::mdds::flat_segment_tree<sal_Int32, sal_Int32> StylesType;
    std::vector<std::unique_ptr<StylesType> > aTables;
    struct Cache
    {
        sal_Int32 mnTable;
        sal_Int32 mnStart;
        sal_Int32 mnEnd;
        sal_Int32 mnStyle;
        Cache();

        bool hasCache(sal_Int32 nTable, sal_Int32 nField) const;
    };
    Cache maCache;

public:
    ScRowStyles();
    virtual ~ScRowStyles() override;

    virtual void AddNewTable(const sal_Int32 nTable, const sal_Int32 nFields) override;
    sal_Int32 GetStyleNameIndex(const sal_Int32 nTable, const sal_Int32 nField);
    void AddFieldStyleName(const sal_Int32 nTable, const sal_Int32 nField, const sal_Int32 nStringIndex);
    void AddFieldStyleName(const sal_Int32 nTable, const sal_Int32 nStartField, const sal_Int32 nStringIndex, const sal_Int32 nEndField);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
