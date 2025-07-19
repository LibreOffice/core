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

#include "LoggedResources.hxx"
#include <com/sun/star/text/XTextDocument.hpp>
#include <rtl/ref.hxx>
#include <memory>
#include <optional>

class SwXTextDocument;
namespace com::sun::star::lang
{
class XMultiServiceFactory;
struct Locale;
}

namespace writerfilter::dmapper
{
class DomainMapper;
class DocumentProtection;
class WriteProtection;

class SettingsTable : public LoggedProperties, public LoggedTable
{
public:
    SettingsTable(const DomainMapper& rDomainMapper);
    virtual ~SettingsTable() override;

    //returns default TabStop in 1/100th mm
    int GetDefaultTabStop() const;

    /// Automatically update styles from document template?
    bool GetLinkStyles() const;

    /// What's the zoom factor set in percents?
    sal_Int16 GetZoomFactor() const;

    /// Gets the type of the zoom.
    sal_Int16 GetZoomType() const;

    /// What's the requested view? E.g. "web".
    Id GetView() const;

    bool GetEvenAndOddHeaders() const;

    bool GetUsePrinterMetrics() const;

    bool GetEmbedTrueTypeFonts() const;
    bool GetEmbedSystemFonts() const;

    bool GetDoNotUseHTMLParagraphAutoSpacing() const;
    bool GetSplitPgBreakAndParaMark() const;
    bool GetMirrorMarginSettings() const;
    bool GetDisplayBackgroundShape() const;
    bool GetDoNotExpandShiftReturn() const;
    bool GetBalanceSingleByteDoubleByteWidth() const;
    bool GetNoColumnBalance() const;
    bool GetProtectForm() const;
    bool GetReadOnly() const;
    bool GetLongerSpaceSequence() const;
    bool GetNoLeading() const;
    bool GetNoHyphenateCaps() const;
    bool GetMsWordUlTrailSpace() const;
    sal_Int16 GetHyphenationZone() const;
    sal_Int16 GetConsecutiveHyphenLimit() const;
    bool GetHyphenationKeep() const;
    bool GetHyphenationKeepLine() const;

    const OUString& GetDecimalSymbol() const;
    const OUString& GetListSeparator() const;

    css::uno::Sequence<css::beans::PropertyValue> const& GetThemeFontLangProperties() const;

    css::uno::Sequence<css::beans::PropertyValue> GetCompatSettings();

    css::uno::Sequence<css::beans::PropertyValue> GetDocumentProtectionSettings() const;

    css::uno::Sequence<css::beans::PropertyValue> GetWriteProtectionSettings() const;

    void ApplyProperties(rtl::Reference<SwXTextDocument> const& xDoc);

    std::pair<bool, bool> GetCompatSettingHasAndValue(std::u16string_view sCompatName) const;
    sal_Int32 GetWordCompatibilityMode();

    const OUString& GetCurrentDatabaseDataSource() const;
    bool GetGutterAtTop() const;

    bool GetRecordChanges() const;

    bool GetEndnoteIsCollectAtSectionEnd() const;

    bool GetDoNotVertAlignCellWithSp() const;

    bool GetAdjustLineHeightInTable() const;

private:
    // Properties
    virtual void lcl_attribute(Id Name, const Value& val) override;
    virtual void lcl_sprm(Sprm& sprm) override;

    // Table
    virtual void lcl_entry(const writerfilter::Reference<Properties>::Pointer_t& ref) override;

    int m_nDefaultTabStop;

    bool m_bRecordChanges;
    bool m_bShowInsDelChanges;
    bool m_bShowFormattingChanges;
    bool m_bShowMarkupChanges;
    std::optional<bool> m_oLinkStyles;
    sal_Int16 m_nZoomFactor;
    sal_Int16 m_nZoomType = 0;
    sal_Int32 m_nWordCompatibilityMode;
    Id m_nView;
    bool m_bEvenAndOddHeaders;
    bool m_bUsePrinterMetrics;
    bool embedTrueTypeFonts;
    bool embedSystemFonts;
    bool m_bDoNotUseHTMLParagraphAutoSpacing;
    bool m_bNoColumnBalance;
    bool m_bAutoHyphenation;
    bool m_bNoHyphenateCaps;
    bool m_bMsWordUlTrailSpace = false;
    sal_Int16 m_nHyphenationZone;
    sal_Int16 m_nConsecutiveHyphenLimit;
    sal_Int16 m_nUseWord2013TrackBottomHyphenation;
    sal_Int16 m_nAllowHyphenationAtTrackBottom;
    bool m_bWidowControl;
    bool m_bLongerSpaceSequence;
    bool m_bSplitPgBreakAndParaMark;
    bool m_bMirrorMargin;
    bool m_bDoNotExpandShiftReturn;
    bool m_bBalanceSingleByteDoubleByteWidth = false;
    bool m_bDisplayBackgroundShape;
    bool m_bNoLeading = false;
    OUString m_sDecimalSymbol;
    OUString m_sListSeparator;
    std::vector<std::pair<OUString, OUString>> m_aDocVars;

    css::uno::Sequence<css::beans::PropertyValue> m_pThemeFontLangProps;

    std::vector<css::beans::PropertyValue> m_aCompatSettings;
    css::uno::Sequence<css::beans::PropertyValue> m_pCurrentCompatSetting;
    OUString m_aCurrentCompatSettingName;
    OUString m_aCurrentCompatSettingUri;
    OUString m_aCurrentCompatSettingValue;
    OUString m_sCurrentDatabaseDataSource;

    std::shared_ptr<DocumentProtection> m_pDocumentProtection;
    std::shared_ptr<WriteProtection> m_pWriteProtection;
    bool m_bGutterAtTop = false;
    bool m_bDoNotBreakWrappedTables = false;
    bool m_bAllowTextAfterFloatingTableBreak = false;
    /// Endnotes at section end, not at document end.
    bool m_bEndnoteIsCollectAtSectionEnd = false;
    /// Don't vertically align table cells containing shapes
    bool m_bDoNotVertAlignCellWithSp = false; // tdf#37153
    bool m_bAdjustLineHeightInTable = false; // tdf#167583
};
typedef tools::SvRef<SettingsTable> SettingsTablePtr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
