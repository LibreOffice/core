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

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <memory>
#include <vcl/dllapi.h>
#include <tools/link.hxx>
#include <vcl/image.hxx>
#include <vcl/toolkit/treelistbox.hxx>
#include <o3tl/typed_flags_set.hxx>

class SvTreeListEntry;

enum class SvBmp
{
    UNCHECKED,
    CHECKED,
    TRISTATE,
    HIUNCHECKED,
    HICHECKED,
    HITRISTATE,
};

enum class SvItemStateFlags
{
    NONE               = 0x00,
    UNCHECKED          = 0x01,
    CHECKED            = 0x02,
    TRISTATE           = 0x04,
    HIGHLIGHTED          = 0x08
};
namespace o3tl
{
    template<> struct typed_flags<SvItemStateFlags> : is_typed_flags<SvItemStateFlags, 0x0f> {};
}

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) SvLBoxString : public SvLBoxItem
{
private:
    bool mbEmphasized;
    bool mbCustom;
    TxtAlign meAlign;

protected:
    OUString maText;

public:
    SvLBoxString(OUString aText);
    SvLBoxString();
    virtual ~SvLBoxString() override;

    virtual SvLBoxItemType GetType() const override;
    virtual void InitViewData(SvTreeListBox& rView, SvTreeListEntry& rEntry,
                              SvViewDataItem* pViewData = nullptr) override;

    virtual int CalcWidth(const SvTreeListBox& rView) const override;

    void Align(TxtAlign eAlign) { meAlign = eAlign; }

    void Emphasize(bool bEmphasize) { mbEmphasized = bEmphasize; }
    bool IsEmphasized() const { return mbEmphasized; }

    void SetCustomRender() { mbCustom = true; }
    bool IsCustomRender() const { return mbCustom; }

    const OUString& GetText() const
    {
        return maText;
    }
    void SetText(const OUString& rText)
    {
        maText = rText;
    }

    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev,
                       vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView,
                       const SvTreeListEntry& rEntry) override;

    virtual std::unique_ptr<SvLBoxItem> Clone(SvLBoxItem const * pSource) const override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) SvLBoxContextBmp : public SvLBoxItem
{
    Image m_aImage1;
    Image m_aImage2;

public:
    SvLBoxContextBmp(const Image& aBmp1, const Image& aBmp2);
    virtual ~SvLBoxContextBmp() override;

    virtual SvLBoxItemType GetType() const override;
    virtual void InitViewData(SvTreeListBox& rView, SvTreeListEntry& rEntry,
                              SvViewDataItem* pViewData = nullptr) override;
    virtual void Paint(const Point& rPos,
                       SvTreeListBox& rOutDev,
                       vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView,
                       const SvTreeListEntry& rEntry) override;

    virtual std::unique_ptr<SvLBoxItem> Clone(SvLBoxItem const * pSource) const override;

    void SetBitmap1(const Image& rImage) { m_aImage1 = rImage; };
    void SetBitmap2(const Image& rImage) { m_aImage2 = rImage; };
    const Image& GetBitmap1() const { return m_aImage1; };
    const Image& GetBitmap2() const { return m_aImage2; };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
