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


#ifndef INCLUDED_SVTOOLS_SVLBITM_HXX
#define INCLUDED_SVTOOLS_SVLBITM_HXX

#include <svtools/svtdllapi.h>
#include <tools/link.hxx>
#include <vcl/image.hxx>
#include <svtools/treelistbox.hxx>
#include <o3tl/typed_flags_set.hxx>

class SvTreeListEntry;


enum class SvBmp
{
    UNCHECKED        = 0,
    CHECKED          = 1,
    TRISTATE         = 2,
    HIUNCHECKED      = 3,
    HICHECKED        = 4,
    HITRISTATE       = 5,
    STATICIMAGE      = 6
};

enum class SvItemStateFlags
{
    NONE               = 0x00,
    UNCHECKED          = 0x01,
    CHECKED            = 0x02,
    TRISTATE           = 0x04,
    HILIGHTED          = 0x08
};
namespace o3tl
{
    template<> struct typed_flags<SvItemStateFlags> : is_typed_flags<SvItemStateFlags, 0x0f> {};
}

struct SvLBoxButtonData_Impl;

class SVT_DLLPUBLIC SvLBoxButtonData
{
private:
    Link<SvLBoxButtonData*,void> aLink;
    long                    nWidth;
    long                    nHeight;
    std::unique_ptr<SvLBoxButtonData_Impl> pImpl;
    bool                    bDataOk;
    SvButtonState           eState;
    std::vector<Image>      aBmps;  // indices s. constants BMP_ ....

    SVT_DLLPRIVATE void     SetWidthAndHeight();
    SVT_DLLPRIVATE void     InitData( bool _bRadioBtn, const Control* pControlForSettings = nullptr );
public:
                            // include creating default images (CheckBox or RadioButton)
                            SvLBoxButtonData( const Control* pControlForSettings );
                            SvLBoxButtonData( const Control* pControlForSettings, bool _bRadioBtn );

                            ~SvLBoxButtonData();

    static SvBmp            GetIndex( SvItemStateFlags nItemState );
    long                    Width();
    long                    Height();
    void                    SetLink( const Link<SvLBoxButtonData*,void>& rLink) { aLink=rLink; }
    bool                    IsRadio();
    // as buttons are not derived from LinkHdl
    void                    CallLink();

    void                    StoreButtonState( SvTreeListEntry* pEntry, SvItemStateFlags nItemFlags );
    static SvButtonState    ConvertToButtonState( SvItemStateFlags nItemFlags );

    SvTreeListEntry*        GetActEntry() const;

    void                    SetImage(SvBmp nIndex, const Image& aImage) { aBmps[(int)nIndex] = aImage; }
    Image&                  GetImage(SvBmp nIndex) { return aBmps[(int)nIndex]; }

    void                    SetDefaultImages( const Control* pControlForSettings = nullptr );
                                // set images according to the color scheme of the Control
                                // pControlForSettings == NULL: settings are taken from Application
    bool                    HasDefaultImages() const;
};

// **********************************************************************

class SVT_DLLPUBLIC SvLBoxString : public SvLBoxItem
{
protected:
    OUString maText;

public:
    SvLBoxString(const OUString& rText);
    SvLBoxString();
    virtual ~SvLBoxString();

    virtual sal_uInt16 GetType() const override;
    virtual void InitViewData(SvTreeListBox* pView,
                              SvTreeListEntry* pEntry,
                              SvViewDataItem* pViewData = nullptr) override;

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

    virtual SvLBoxItem* Create() const override;
    virtual void Clone(SvLBoxItem* pSource) override;
};

class SvLBoxBmp : public SvLBoxItem
{
    Image aBmp;
public:
    SvLBoxBmp();
    virtual ~SvLBoxBmp();
    virtual sal_uInt16 GetType() const override;
    virtual void InitViewData( SvTreeListBox* ,SvTreeListEntry*, SvViewDataItem* = nullptr ) override;
    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev,  vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
    virtual SvLBoxItem* Create() const override;
    virtual void Clone( SvLBoxItem* pSource ) override;
};


class SVT_DLLPUBLIC SvLBoxButton : public SvLBoxItem
{
    bool    isVis;
    SvLBoxButtonData*   pData;
    SvLBoxButtonKind eKind;
    SvItemStateFlags nItemFlags;

    static void ImplAdjustBoxSize( Size& io_rCtrlSize, ControlType i_eType, vcl::RenderContext& pRenderContext);
public:
    // An SvLBoxButton can be of three different kinds: an
    // enabled checkbox (the normal kind), a disabled checkbox
    // (which cannot be modified via UI), or a static image
    // (see SV_BMP_STATICIMAGE; nFlags are effectively ignored
    // for that kind).
    SvLBoxButton( SvLBoxButtonKind eTheKind, SvLBoxButtonData* pBData );
    SvLBoxButton();
    virtual ~SvLBoxButton();
    virtual void InitViewData(SvTreeListBox* pView,
                              SvTreeListEntry* pEntry,
                              SvViewDataItem* pViewData = nullptr) override;

    virtual sal_uInt16 GetType() const override;
    bool ClickHdl(SvTreeListBox* pView, SvTreeListEntry* );

    virtual void Paint(const Point& rPos,
                       SvTreeListBox& rOutDev,
                       vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView,
                       const SvTreeListEntry& rEntry) override;

    virtual SvLBoxItem* Create() const override;

    virtual void Clone(SvLBoxItem* pSource) override;
    SvItemStateFlags GetButtonFlags() const
    {
        return nItemFlags;
    }
    bool IsStateChecked() const
    {
        return bool(nItemFlags & SvItemStateFlags::CHECKED);
    }
    bool IsStateUnchecked() const
    {
        return bool(nItemFlags & SvItemStateFlags::UNCHECKED);
    }
    bool IsStateTristate() const
    {
        return bool(nItemFlags & SvItemStateFlags::TRISTATE);
    }
    bool IsStateHilighted() const
    {
        return bool(nItemFlags & SvItemStateFlags::HILIGHTED);
    }
    void SetStateChecked();
    void SetStateUnchecked();
    void SetStateTristate();
    void SetStateHilighted(bool bHilight);
    void SetStateInvisible();

    SvLBoxButtonKind GetKind() const { return eKind; }

    // Check whether this button can be modified via UI
    bool CheckModification() const;
};

inline void SvLBoxButton::SetStateChecked()
{
    nItemFlags &= SvItemStateFlags::HILIGHTED;
    nItemFlags |= SvItemStateFlags::CHECKED;
}

inline void SvLBoxButton::SetStateUnchecked()
{
    nItemFlags &= SvItemStateFlags::HILIGHTED;
    nItemFlags |= SvItemStateFlags::UNCHECKED;
}
inline void SvLBoxButton::SetStateTristate()
{
    nItemFlags &= SvItemStateFlags::HILIGHTED;
    nItemFlags |= SvItemStateFlags::TRISTATE;
}
inline void SvLBoxButton::SetStateHilighted( bool bHilight )
{
    if ( bHilight )
        nItemFlags |= SvItemStateFlags::HILIGHTED;
    else
        nItemFlags &= ~SvItemStateFlags::HILIGHTED;
}

struct SvLBoxContextBmp_Impl;

class SVT_DLLPUBLIC SvLBoxContextBmp : public SvLBoxItem
{
    std::unique_ptr<SvLBoxContextBmp_Impl>  m_pImpl;
public:
    SvLBoxContextBmp(Image aBmp1,
                     Image aBmp2,
                     bool bExpanded);
    SvLBoxContextBmp();
    virtual ~SvLBoxContextBmp();

    virtual sal_uInt16 GetType() const override;
    virtual void InitViewData(SvTreeListBox* pView,
                              SvTreeListEntry* pEntry,
                              SvViewDataItem* pViewData = nullptr) override;
    virtual void Paint(const Point& rPos,
                       SvTreeListBox& rOutDev,
                       vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView,
                       const SvTreeListEntry& rEntry) override;

    virtual SvLBoxItem* Create() const override;
    virtual void Clone(SvLBoxItem* pSource) override;


    void SetModeImages(const Image& rBitmap1, const Image& rBitmap2);

    inline void SetBitmap1(const Image& rImage);
    inline void SetBitmap2(const Image& rImage);
    inline const Image& GetBitmap1() const;
    inline const Image& GetBitmap2() const;

private:
    Image& implGetImageStore(bool bFirst);
};

inline void SvLBoxContextBmp::SetBitmap1(const Image& _rImage)
{
    implGetImageStore(true) = _rImage;
}

inline void SvLBoxContextBmp::SetBitmap2(const Image& _rImage)
{
    implGetImageStore(false) = _rImage;
}

inline const Image& SvLBoxContextBmp::GetBitmap1() const
{
    Image& rImage = const_cast<SvLBoxContextBmp*>(this)->implGetImageStore(true);
    return rImage;
}

inline const Image& SvLBoxContextBmp::GetBitmap2() const
{
    Image& rImage = const_cast<SvLBoxContextBmp*>(this)->implGetImageStore(false);
    return rImage;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
