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

#include <svl/itemprop.hxx>
#include <i18nlangtag/lang.h>
#include <svx/autoformathelper.hxx>
#include <bitset>

namespace Autoformat
{
enum PropertyIndex : sal_uInt8
{
    PROP_FONT = 0,
    PROP_HEIGHT,
    PROP_WEIGHT,
    PROP_POSTURE,
    PROP_CJK_FONT,
    PROP_CJK_HEIGHT,
    PROP_CJK_WEIGHT,
    PROP_CJK_POSTURE,
    PROP_CTL_FONT,
    PROP_CTL_HEIGHT,
    PROP_CTL_WEIGHT,
    PROP_CTL_POSTURE,
    PROP_UNDERLINE,
    PROP_COLOR,
    PROP_BOX,
    PROP_BACKGROUND,
    PROP_HOR_JUSTIFY,
    PROP_VER_JUSTIFY,
    PROP_PADDING,
    PROP_COUNT
};

enum Elements : sal_uInt8
{
    FIRST_ROW = 0,
    LAST_ROW,
    FIRST_COL,
    LAST_COL,
    BODY,
    EVEN_ROW,
    ODD_ROW,
    EVEN_COL,
    ODD_COL,
    BACKGROUND,
    FIRST_ROW_EVEN_COL,
    LAST_ROW_EVEN_COL,
    FIRST_ROW_END_COL,
    FIRST_ROW_START_COL,
    LAST_ROW_END_COL,
    LAST_ROW_START_COL,
    ELEMENT_COUNT
};
}

using namespace Autoformat;

class SVX_DLLPUBLIC SvxAutoFormatDataField : public AutoFormatBase
{
private:
    // number format
    rtl::OUString maNumFormatString;
    LanguageType meSysLanguage;
    LanguageType meNumFormatLanguage;

    std::bitset<Autoformat::PROP_COUNT> mSetProperties;

public:
    SvxAutoFormatDataField(){};
    SvxAutoFormatDataField(const SvxAutoFormatDataField& rCopy);

    virtual ~SvxAutoFormatDataField() = default;

    void GetValueFormat(OUString& rFormat, LanguageType& rLng, LanguageType& rSys) const;
    const OUString& GetNumFormatString() const noexcept { return maNumFormatString; }
    const LanguageType& GetSysLanguage() const noexcept { return meSysLanguage; }
    const LanguageType& GetNumFormatLanguage() const noexcept { return meNumFormatLanguage; }

    void SetValueFormat(const OUString& rFormat, LanguageType eLng, LanguageType eSys);
    void SetNumFormatString(const OUString& rNew) { maNumFormatString = rNew; }
    void SetSysLanguage(const LanguageType& rNew) noexcept { meSysLanguage = rNew; }
    void SetNumFormatLanguage(const LanguageType& rNew) noexcept { meNumFormatLanguage = rNew; }

    bool IsPropertySet(PropertyIndex prop) const noexcept
    {
        return prop < PROP_COUNT && mSetProperties[prop];
    }
    void SetPropertyFlag(PropertyIndex prop, bool set = true) noexcept
    {
        if (prop < PROP_COUNT)
            mSetProperties[prop] = set;
    }
    std::bitset<PROP_COUNT> GetPropertySet() const { return mSetProperties; }
    void SetPropertySet(std::bitset<PROP_COUNT> aSet) { mSetProperties = aSet; }
};

class SVX_DLLPUBLIC SvxAutoFormatData
{
private:
    OUString maName;

    // Common flags of Calc and Writer.
    bool mbInclFont : 1;
    bool mbInclJustify : 1;
    bool mbInclFrame : 1;
    bool mbInclBackground : 1;
    // Calc specific flags
    bool mbInclValueFormat : 1;
    bool mbInclWidthHeight : 1;

    bool mbUseFirstRowStyles : 1;
    bool mbUseLastRowStyles : 1;
    bool mbUseFirstColStyles : 1;
    bool mbUseLastColStyles : 1;
    bool mbUseBandedRowStyles : 1;
    bool mbUseBandedColStyles : 1;

    OUString maParent;

public:
    SvxAutoFormatData();
    SvxAutoFormatData(const SvxAutoFormatData& rData);
    virtual ~SvxAutoFormatData() = default;

    bool IsFont() const noexcept { return mbInclFont; }
    bool IsJustify() const noexcept { return mbInclJustify; }
    bool IsFrame() const noexcept { return mbInclFrame; }
    bool IsBackground() const noexcept { return mbInclBackground; }
    bool IsValueFormat() const noexcept { return mbInclValueFormat; }
    bool IsWidthHeight() const noexcept { return mbInclWidthHeight; }

    bool UseFirstRowStyles() const noexcept { return mbUseFirstRowStyles; }
    bool UseLastRowStyles() const noexcept { return mbUseLastRowStyles; }
    bool UseFirstColStyles() const noexcept { return mbUseFirstColStyles; }
    bool UseLastColStyles() const noexcept { return mbUseLastColStyles; }
    bool UseBandedRowStyles() const noexcept { return mbUseBandedRowStyles; }
    bool UseBandedColStyles() const noexcept { return mbUseBandedColStyles; }

    void SetFont(const bool bNew) noexcept { mbInclFont = bNew; }
    void SetJustify(const bool bNew) noexcept { mbInclJustify = bNew; }
    void SetFrame(const bool bNew) noexcept { mbInclFrame = bNew; }
    void SetBackground(const bool bNew) noexcept { mbInclBackground = bNew; }
    void SetValueFormat(const bool bNew) noexcept { mbInclValueFormat = bNew; }
    void SetWidthHeight(const bool bNew) noexcept { mbInclWidthHeight = bNew; }
    void SetParent(const OUString& rParentName) { maParent = rParentName; }

    void SetUseFirstRowStyles(const bool bNew) noexcept { mbUseFirstRowStyles = bNew; }
    void SetUseFirstColStyles(const bool bNew) noexcept { mbUseFirstColStyles = bNew; }
    void SetUseLastRowStyles(const bool bNew) noexcept { mbUseLastRowStyles = bNew; }
    void SetUseLastColStyles(const bool bNew) noexcept { mbUseLastColStyles = bNew; }
    void SetUseBandedRowStyles(const bool bNew) noexcept { mbUseBandedRowStyles = bNew; }
    void SetUseBandedColStyles(const bool bNew) noexcept { mbUseBandedColStyles = bNew; }

    const OUString& GetName() const noexcept { return maName; }
    const OUString& GetParent() const noexcept { return maParent; }

    virtual void SetName(const OUString& rName) { maName = rName; }
    virtual SvxAutoFormatDataField* GetField(size_t nIndex) = 0;
    virtual const SvxAutoFormatDataField* GetField(size_t nIndex) const = 0;
    virtual SvxAutoFormatDataField* GetDefaultField() const = 0;
    virtual bool SetField(size_t nIndex, const SvxAutoFormatDataField& aField) = 0;
    virtual SvxAutoFormatData* MakeCopy() const = 0;

    void FillToItemSet(size_t nIndex, SfxItemSet& rItemSet) const;
    void MergeStyle(const SvxAutoFormatData& pData);
    void ResetAutoFormat(SvxAutoFormatData& pOld);

    SvxAutoFormatData& operator=(const SvxAutoFormatData& rCopy);
};

class SVX_DLLPUBLIC SvxAutoFormat
{
public:
    virtual ~SvxAutoFormat();

    virtual const SvxAutoFormatData* GetData(size_t nIndex) const = 0;
    virtual SvxAutoFormatData* GetData(size_t nIndex) = 0;
    virtual bool InsertAutoFormat(SvxAutoFormatData* pFormat) = 0;
    virtual SvxAutoFormatData* ReleaseAutoFormat(const OUString& rName) = 0;
    virtual SvxAutoFormatData* FindAutoFormat(const OUString& rName) const = 0;
    virtual SvxAutoFormatData* GetDefaultData() = 0;
    virtual SvxAutoFormatDataField* GetDefaultField() = 0;
    virtual const SvxAutoFormatData* GetResolvedStyle(const SvxAutoFormatData* pData) const = 0;
    virtual size_t size() const = 0;
    virtual void ResetParent(const OUString& rName);

    static const SfxItemPropertySet& GetTablePropertySet();

    bool Load(bool bWriter);
    bool Save();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
