/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include "document.hxx"
#include "scdllapi.h"
#include "dbdata.hxx"

class ScPatternAttr;

enum class ScTableStyleElement
{
    WholeTable,
    FirstColumnStripe,
    SecondColumnStripe,
    FirstRowStripe,
    SecondRowStripe,
    LastColumn,
    FirstColumn,
    HeaderRow,
    TotalRow,
    FirstHeaderCell,
    LastHeaderCell,
};

template <class T> const T* GetItemFromPattern(ScPatternAttr* pPattern, TypedWhichId<T> nWhich)
{
    return pPattern->GetItemSet().GetItemIfSet(nWhich);
}

class SC_DLLPUBLIC ScTableStyle
{
private:
    ScTableStyle(ScTableStyle const&) = delete;
    ScTableStyle(ScTableStyle&&) = delete;
    void operator=(ScTableStyle const&) = delete;
    void operator=(ScTableStyle&&) = delete;

    std::unique_ptr<ScPatternAttr> mpTablePattern;
    std::unique_ptr<ScPatternAttr> mpFirstColumnStripePattern;
    std::unique_ptr<ScPatternAttr> mpSecondColumnStripePattern;
    std::unique_ptr<ScPatternAttr> mpFirstRowStripePattern;
    std::unique_ptr<ScPatternAttr> mpSecondRowStripePattern;
    std::unique_ptr<ScPatternAttr> mpLastColumnPattern;
    std::unique_ptr<ScPatternAttr> mpFirstColumnPattern;
    std::unique_ptr<ScPatternAttr> mpHeaderRowPattern;
    std::unique_ptr<ScPatternAttr> mpTotalRowPattern;
    std::unique_ptr<ScPatternAttr> mpFirstHeaderCellPattern;
    std::unique_ptr<ScPatternAttr> mpLastHeaderCellPattern;

    sal_Int32 mnFirstRowStripeSize;
    sal_Int32 mnSecondRowStripeSize;
    sal_Int32 mnFirstColStripeSize;
    sal_Int32 mnSecondColStripeSize;

    OUString maStyleName;
    std::optional<OUString> maUIName;
    bool mbIsOOXMLDefault;

public:
    ScTableStyle(const OUString& rName, const std::optional<OUString>& rUIName);

    static bool HasFontAttrSet(ScPatternAttr* pPattern);
    const SfxItemSet* GetFontItemSet(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                     SCROW nRowIndex) const;
    const SvxBrushItem* GetFillItem(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                    SCROW nRowIndex) const;
    std::unique_ptr<SvxBoxItem> GetBoxItem(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                           SCROW nRowIndex) const;

    void SetRowStripeSize(sal_Int32 nFirstRowStripeSize, sal_Int32 nSecondRowStripeSize);
    void SetColStripeSize(sal_Int32 nFirstColStripeSize, sal_Int32 nSecondColStripeSize);

    void SetPattern(ScTableStyleElement eTableStyleElement,
                    std::unique_ptr<ScPatternAttr> pPattern);
    void SetTablePattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetFirstColumnStripePattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetSecondColumnStripePattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetFirstRowStripePattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetSecondRowStripePattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetLastColumnPattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetFirstColumnPattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetHeaderRowPattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetTotalRowPattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetFirstHeaderCellPattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetLastHeaderCellPattern(std::unique_ptr<ScPatternAttr> pPattern);

    std::map<ScTableStyleElement, const ScPatternAttr*> GetSetPatterns() const;

    sal_Int32 GetFirstRowStripeSize() const;
    sal_Int32 GetFirstColumnStripeSize() const;
    sal_Int32 GetSecondRowStripeSize() const;
    sal_Int32 GetSecondColumnStripeSize() const;

    void SetOOXMLDefault(bool bDefault);
    bool IsOOXMLDefault() const;

    const OUString& GetName() const;
    const OUString& GetUIName() const;
};

class SC_DLLPUBLIC ScTableStyles
{
private:
    ScTableStyles(ScTableStyles const&) = delete;
    ScTableStyles(ScTableStyles&&) = delete;
    void operator=(ScTableStyles const&) = delete;
    void operator=(ScTableStyles&&) = delete;

    std::unordered_map<OUString, std::unique_ptr<ScTableStyle>> maTableStyles;

public:
    ScTableStyles();

    void AddTableStyle(std::unique_ptr<ScTableStyle> pTableStyle);
    void DeleteTableStyle(const OUString& rName);
    const ScTableStyle* GetTableStyle(const OUString& rName) const;
    bool HasTableStyle() const { return maTableStyles.size() > 0; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
