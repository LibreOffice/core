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

#ifndef INCLUDED_STARMATH_SOURCE_CFGITEM_HXX
#define INCLUDED_STARMATH_SOURCE_CFGITEM_HXX

#include "utility.hxx"

#include <string_view>
#include <vector>

#include <rtl/ustring.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <unotools/configitem.hxx>

#include "types.hxx"
#include <memory>

namespace com::sun::star::uno
{
template <class E> class Sequence;
}

class SmSym;
class SmSymbolManager;
class SmFormat;
namespace vcl
{
class Font;
}
struct SmCfgOther;
class SfxItemSet;

struct SmFontFormat
{
    OUString aName;
    sal_Int16 nCharSet;
    sal_Int16 nFamily;
    sal_Int16 nPitch;
    sal_Int16 nWeight;
    sal_Int16 nItalic;

    SmFontFormat();
    explicit SmFontFormat(const vcl::Font& rFont);

    vcl::Font GetFont() const;
    bool operator==(const SmFontFormat& rFntFmt) const;
};

struct SmFntFmtListEntry
{
    OUString aId;
    SmFontFormat aFntFmt;

    SmFntFmtListEntry(const OUString& rId, const SmFontFormat& rFntFmt);
};

class SmFontFormatList
{
    std::vector<SmFntFmtListEntry> aEntries;
    bool bModified;

    SmFontFormatList(const SmFontFormatList&) = delete;
    SmFontFormatList& operator=(const SmFontFormatList&) = delete;

public:
    SmFontFormatList();

    void Clear();
    void AddFontFormat(const OUString& rFntFmtId, const SmFontFormat& rFntFmt);
    void RemoveFontFormat(std::u16string_view rFntFmtId);

    const SmFontFormat* GetFontFormat(std::u16string_view rFntFmtId) const;
    const SmFontFormat* GetFontFormat(size_t nPos) const;
    OUString GetFontFormatId(const SmFontFormat& rFntFmt) const;
    OUString GetFontFormatId(const SmFontFormat& rFntFmt, bool bAdd);
    OUString GetFontFormatId(size_t nPos) const;
    OUString GetNewFontFormatId() const;
    size_t GetCount() const { return aEntries.size(); }

    bool IsModified() const { return bModified; }
    void SetModified(bool bVal) { bModified = bVal; }
};

class SmMathConfig final : public utl::ConfigItem, public SfxBroadcaster
{
    std::unique_ptr<SmFormat> pFormat;
    std::unique_ptr<SmCfgOther> pOther;
    std::unique_ptr<SmFontFormatList> pFontFormatList;
    std::unique_ptr<SmSymbolManager> pSymbolMgr;
    bool bIsOtherModified;
    bool bIsFormatModified;
    SmFontPickList vFontPickList[7];

    SmMathConfig(const SmMathConfig&) = delete;
    SmMathConfig& operator=(const SmMathConfig&) = delete;

    void StripFontFormatList(const std::vector<SmSym>& rSymbols);

    void Save();

    void ReadSymbol(SmSym& rSymbol, const OUString& rSymbolName,
                    std::u16string_view rBaseNode) const;
    void ReadFontFormat(SmFontFormat& rFontFormat, std::u16string_view rSymbolName,
                        std::u16string_view rBaseNode) const;

    void SetOtherIfNotEqual(bool& rbItem, bool bNewVal);

    void LoadOther();
    void SaveOther();
    void LoadFormat();
    void SaveFormat();
    void LoadFontFormatList();
    void SaveFontFormatList();

    void SetOtherModified(bool bVal);
    bool IsOtherModified() const { return bIsOtherModified; }
    void SetFormatModified(bool bVal);
    bool IsFormatModified() const { return bIsFormatModified; }

    SmFontFormatList& GetFontFormatList();
    const SmFontFormatList& GetFontFormatList() const
    {
        return const_cast<SmMathConfig*>(this)->GetFontFormatList();
    }

    virtual void ImplCommit() override;

public:
    SmMathConfig();
    virtual ~SmMathConfig() override;

    // utl::ConfigItem
    virtual void Notify(const css::uno::Sequence<OUString>& rPropertyNames) override;

    SmSymbolManager& GetSymbolManager();
    void GetSymbols(std::vector<SmSym>& rSymbols) const;
    void SetSymbols(const std::vector<SmSym>& rNewSymbols);

    const SmFormat& GetStandardFormat() const;
    void SetStandardFormat(const SmFormat& rFormat, bool bSaveFontFormatList = false);

    bool IsPrintTitle() const;
    void SetPrintTitle(bool bVal);
    bool IsPrintFormulaText() const;
    void SetPrintFormulaText(bool bVal);
    bool IsPrintFrame() const;
    void SetPrintFrame(bool bVal);
    SmPrintSize GetPrintSize() const;
    void SetPrintSize(SmPrintSize eSize);
    sal_uInt16 GetPrintZoomFactor() const;
    void SetPrintZoomFactor(sal_uInt16 nVal);

    bool IsSaveOnlyUsedSymbols() const;
    void SetSaveOnlyUsedSymbols(bool bVal);
    bool IsAutoCloseBrackets() const;
    void SetAutoCloseBrackets(bool bVal);
    bool IsIgnoreSpacesRight() const;
    void SetIgnoreSpacesRight(bool bVal);
    bool IsAutoRedraw() const;
    void SetAutoRedraw(bool bVal);
    bool IsShowFormulaCursor() const;
    void SetShowFormulaCursor(bool bVal);

    sal_uInt16 GetDefaultSmSyntaxVersion() const;
    void SetDefaultSmSyntaxVersion(sal_uInt16 nVal);

    SmFontPickList& GetFontPickList(sal_uInt16 nIdent) { return vFontPickList[nIdent]; }

    void ItemSetToConfig(const SfxItemSet& rSet);
    void ConfigToItemSet(SfxItemSet& rSet) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
