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
#ifndef INCLUDED_SW_INC_TBLAFMT_HXX
#define INCLUDED_SW_INC_TBLAFMT_HXX

#include <memory>

#include <editeng/keepitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/shaditem.hxx>
#include <svx/autoformathelper.hxx>
#include <unotools/weakref.hxx>
#include <rtl/ref.hxx>
#include <svx/TableAutoFmt.hxx>
#include "fmtornt.hxx"
#include "node.hxx"
#include "swdllapi.h"

using namespace Autoformat;

class SvNumberFormatter;
class SwTable;
class SwXTextCellStyle;
class SwXTextTableStyle;

class SW_DLLPUBLIC SwBoxAutoFormat : public SvxAutoFormatDataField
{
private:
    std::unique_ptr<SvxFrameDirectionItem> m_aTextOrientation;
    std::unique_ptr<SwFormatVertOrient> m_aVerticalAlignment;

    // associated UNO object, if such exists
    unotools::WeakReference<SwXTextCellStyle> m_xAutoFormatUnoObject;

public:
    SwBoxAutoFormat();
    SwBoxAutoFormat(const SwBoxAutoFormat& rNew);
    SwBoxAutoFormat(const SvxAutoFormatDataField& rNew);
    ~SwBoxAutoFormat();

    /// assignment-op (still used)
    SwBoxAutoFormat& operator=(const SwBoxAutoFormat& rRef);

    /// Comparing based of boxes backgrounds.
    bool operator==(const SwBoxAutoFormat& rRight) const;
    bool IsSameAs(const SwBoxAutoFormat& rBox) const;

    const SvxFrameDirectionItem& GetTextOrientation() const { return *m_aTextOrientation; }
    const SwFormatVertOrient& GetVerticalAlignment() const { return *m_aVerticalAlignment; }

    void SetTextOrientation(const SvxFrameDirectionItem& rNew)
    {
        m_aTextOrientation.reset(rNew.Clone());
    }
    void SetVerticalAlignment(const SwFormatVertOrient& rNew)
    {
        m_aVerticalAlignment.reset(rNew.Clone());
    }

    void UpdateAlignment(bool bSvxUpdate = false);

    unotools::WeakReference<SwXTextCellStyle> const& GetXObject() const
    {
        return m_xAutoFormatUnoObject;
    }
    void SetXObject(rtl::Reference<SwXTextCellStyle> const& xObject);
};

enum class SwTableAutoFormatUpdateFlags
{
    Char = 1,
    Box = 2
};
namespace o3tl
{
template <>
struct typed_flags<SwTableAutoFormatUpdateFlags>
    : is_typed_flags<SwTableAutoFormatUpdateFlags, 0x03>
{
};
};

class SW_DLLPUBLIC SwTableAutoFormat : public SvxAutoFormatData
{
    friend class SwDocTest;
    friend void FinitCore(); // To destroy default pointer.

    TableStyleName m_aName; // note that this could be a ProgName __or__ a UIName
    bool m_bHidden;
    bool m_bUserDefined;

    static SwBoxAutoFormat* s_pDefaultBoxAutoFormat;
    std::array<std::unique_ptr<SwBoxAutoFormat>, ELEMENT_COUNT> m_aBoxAutoFormat;

    unotools::WeakReference<SwXTextTableStyle> m_xUnoTextTableStyle;

public:
    SwTableAutoFormat(const TableStyleName& aName);
    SwTableAutoFormat(const SwTableAutoFormat& rNew);
    SwTableAutoFormat(const SvxAutoFormatData& rNew);

    SwTableAutoFormat& operator=(const SwTableAutoFormat& rNew);

    void SetHidden(bool bHidden) { m_bHidden = bHidden; }
    void SetUserDefined(bool bUserDefined) { m_bUserDefined = bUserDefined; }
    void SetName(const TableStyleName& rNew);
    void SetBoxFormat(const SwBoxAutoFormat& rNew, size_t nIndex);
    void DisableAll();

    bool IsHidden() const { return m_bHidden; }
    bool IsUserDefined() const { return m_bUserDefined; }

    const TableStyleName& GetName() const { return m_aName; }
    static const SwBoxAutoFormat& GetDefaultBoxFormat();
    size_t GetIndex(const SwBoxAutoFormat& rBoxFormat) const;

    void SetName(const OUString& rName) override;
    SwBoxAutoFormat* GetField(size_t nIndex) override;
    const SwBoxAutoFormat* GetField(size_t nIndex) const override;
    bool SetField(size_t nIndex, const SvxAutoFormatDataField& aField) override;
    SvxAutoFormatData* MakeCopy() const override { return new SwTableAutoFormat(*this); };

    SwBoxAutoFormat* GetDefaultField() const override { return new SwBoxAutoFormat; }

    void RestoreToOriginal(SwContentNode* rContentNode, SfxItemSet& aDummySet, size_t nRow,
                           size_t nCol, size_t nRows, size_t nCols) const;
    void UpdateToSet(SfxItemSet& aItemSet, size_t nRow, size_t nCol, size_t nRows, size_t nCols,
                     SvNumberFormatter* pNFormatr) const;
    void UpdateFromSet(size_t nPos, const SfxItemSet& rSet, SwTableAutoFormatUpdateFlags eFlags,
                       SvNumberFormatter const*);
    void FillToItemSet(size_t nIndex, SfxItemSet& rItemSet, SvNumberFormatter* pNFormatr) const;

    bool NeedsExport();

    /// These methods returns what style (row or column) is applied first on given Cell
    bool FirstRowEndColumnIsRow();
    bool FirstRowStartColumnIsRow();
    bool LastRowEndColumnIsRow();
    bool LastRowStartColumnIsRow();
    bool HasHeaderRow() const;

    unotools::WeakReference<SwXTextTableStyle> const& GetXObject() const
    {
        return m_xUnoTextTableStyle;
    }
    void SetXObject(rtl::Reference<SwXTextTableStyle> const& xObject);
};

class SW_DLLPUBLIC SwTableAutoFormatTable : public SvxAutoFormat
{
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;

public:
    SwTableAutoFormatTable();
    ~SwTableAutoFormatTable();

    size_t size() const override;
    SwTableAutoFormat const& operator[](size_t i) const;
    SwTableAutoFormat& operator[](size_t i);

    const SwTableAutoFormat* GetData(size_t nIndex) const override;
    SwTableAutoFormat* GetData(size_t nIndex) override;

    /// Append table style to the existing styles.
    void AddAutoFormat(const SwTableAutoFormat& rFormat);

    bool InsertAutoFormat(SvxAutoFormatData* pFormat) override;
    SwTableAutoFormat* FindAutoFormat(const OUString& rName) const override;
    const SwTableAutoFormat* GetResolvedStyle(const SvxAutoFormatData* pData) const override;
    SwTableAutoFormat* ReleaseAutoFormat(const OUString& rName) override;
    SwTableAutoFormat* GetDefaultData() override
    {
        return new SwTableAutoFormat(TableStyleName(""));
    }
    SwBoxAutoFormat* GetDefaultField() override { return new SwBoxAutoFormat; }
    SwTableAutoFormat* FindAutoFormat(const TableStyleName& rName) const;
    std::unique_ptr<SwTableAutoFormat> ReleaseAutoFormat(const TableStyleName& rName);
};

class SwCellStyleDescriptor
{
    const std::pair<UIName, std::unique_ptr<SwBoxAutoFormat>>& m_rCellStyleDesc;

public:
    SwCellStyleDescriptor(const std::pair<UIName, std::unique_ptr<SwBoxAutoFormat>>& rCellStyleDesc)
        : m_rCellStyleDesc(rCellStyleDesc)
    {
    }

    const UIName& GetName() const { return m_rCellStyleDesc.first; }
    const SwBoxAutoFormat& GetAutoFormat() const { return *m_rCellStyleDesc.second; }
};

class SwCellStyleTable
{
    std::vector<std::pair<UIName, std::unique_ptr<SwBoxAutoFormat>>> m_aCellStyles;

public:
    SwCellStyleTable();
    ~SwCellStyleTable();

    size_t size() const;
    SwCellStyleDescriptor operator[](size_t i) const;
    void clear();

    /// Add a copy of rBoxFormat
    void AddBoxFormat(const SwBoxAutoFormat& rBoxFormat, const UIName& sName);
    void RemoveBoxFormat(const OUString& sName);
    void ChangeBoxFormatName(std::u16string_view sFromName, const UIName& sToName);
    /// If found returns its name. If not found returns an empty UIName
    UIName GetBoxFormatName(const SwBoxAutoFormat& rBoxFormat) const;
    /// If found returns a ptr to a BoxFormat. If not found returns nullptr
    SwBoxAutoFormat* GetBoxFormat(const UIName& sName) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
