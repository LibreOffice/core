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

#include <rangelst.hxx>
#include <rtl/ustring.hxx>

#include <memory>
#include <set>
#include <map>
#include <vector>
#include <optional>

class ScXMLImport;

struct ScMyStyleNumberFormat
{
    OUString       sStyleName;
    sal_Int32      nNumberFormat;

    explicit ScMyStyleNumberFormat(const OUString& rStyleName) :
        sStyleName(rStyleName), nNumberFormat(-1) {}
    ScMyStyleNumberFormat(const OUString& rStyleName, const sal_Int32 nFormat) :
        sStyleName(rStyleName), nNumberFormat(nFormat) {}
};

struct LessStyleNumberFormat
{
    bool operator() (const ScMyStyleNumberFormat& rValue1, const ScMyStyleNumberFormat& rValue2) const
    {
        return rValue1.sStyleName < rValue2.sStyleName;
    }
};

typedef std::set< ScMyStyleNumberFormat, LessStyleNumberFormat >    ScMyStyleNumberFormatSet;

class ScMyStyleNumberFormats
{
    ScMyStyleNumberFormatSet    aSet;

public:
    void AddStyleNumberFormat(const OUString& rStyleName, const sal_Int32 nNumberFormat);
    sal_Int32 GetStyleNumberFormat(const OUString& rStyleName);
};

struct ScMyCurrencyStyle
{
    OUString       sCurrency;
    std::shared_ptr<ScRangeList> mpRanges;

    ScMyCurrencyStyle() :
        mpRanges(std::make_shared<ScRangeList>())
    {}
};

struct LessCurrencyStyle
{
    bool operator() (const ScMyCurrencyStyle& rValue1, const ScMyCurrencyStyle& rValue2) const
    {
        return rValue1.sCurrency < rValue2.sCurrency;
    }
};

typedef std::set<ScMyCurrencyStyle, LessCurrencyStyle>  ScMyCurrencyStylesSet;

class ScMyStyleRanges
{
    std::shared_ptr<ScRangeList>     mpTextList;
    std::shared_ptr<ScRangeList>     mpNumberList;
    std::shared_ptr<ScRangeList>     mpTimeList;
    std::shared_ptr<ScRangeList>     mpDateTimeList;
    std::shared_ptr<ScRangeList>     mpPercentList;
    std::shared_ptr<ScRangeList>     mpLogicalList;
    std::shared_ptr<ScRangeList>     mpUndefinedList;
    std::unique_ptr<ScMyCurrencyStylesSet> pCurrencyList;

    static void SetStylesToRanges(const ScRangeList& rList,
        const OUString* pStyleName, const sal_Int16 nCellType,
        const OUString* pCurrency, ScXMLImport& rImport);
public:
    ScMyStyleRanges();
    ~ScMyStyleRanges();
    void AddRange(const ScRange& rRange, const sal_Int16 nType);
    void AddCurrencyRange(const ScRange& rRange, const std::optional<OUString> & pCurrency);
    void InsertCol(const sal_Int32 nCol, const sal_Int32 nTab);
    void SetStylesToRanges(const OUString* pStyleName, ScXMLImport& rImport);
};

/** map from style name to ScMyStyleRanges */
typedef std::map<OUString, std::unique_ptr<ScMyStyleRanges>>  ScMyStylesMap;

class ScMyStylesImportHelper
{
    ScMyStylesMap       aCellStyles;
    std::vector<ScMyStylesMap::iterator>  aColDefaultStyles;
    ScMyStylesMap::iterator aRowDefaultStyle;
    ScXMLImport&        rImport;
    std::optional<OUString>
                        pStyleName;
    std::optional<OUString>
                        pPrevStyleName;
    std::optional<OUString>
                        pCurrency;
    std::optional<OUString>
                        pPrevCurrency;
    ScRange             aPrevRange;
    sal_Int16           nCellType;
    sal_Int16           nPrevCellType;
    bool                bPrevRangeAdded;

    void ResetAttributes();
    ScMyStylesMap::iterator GetIterator(const OUString & rStyleName);
    void AddDefaultRange(const ScRange& rRange);
    void AddSingleRange(const ScRange& rRange);
    void AddRange();
public:
    explicit ScMyStylesImportHelper(ScXMLImport& rImport);
    ~ScMyStylesImportHelper();
    void AddColumnStyle(const OUString& rStyleName, const sal_Int32 nColumn, const sal_Int32 nRepeat);
    void SetRowStyle(const OUString& rStyleName);
    void SetAttributes(std::optional<OUString> pStyleName,
        std::optional<OUString> pCurrency, const sal_Int16 nCellType);
    void AddRange(const ScRange& rRange);
    void AddCell(const ScAddress& rAddress);
    void InsertCol(const sal_Int32 nCol, const sal_Int32 nTab); // a col is inserted before nCol
    void EndTable();
    void SetStylesToRanges();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
