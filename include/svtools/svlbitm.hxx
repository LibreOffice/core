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

struct SvLBoxButtonData_Impl;

class SVT_DLLPUBLIC SvLBoxButtonData
{
private:
    Link                    aLink;
    long                    nWidth;
    long                    nHeight;
    SvLBoxButtonData_Impl*  pImpl;
    bool                    bDataOk;
    SvButtonState           eState;
    std::vector<Image>      aBmps;  // Indizes siehe Konstanten BMP_ ....

    SVT_DLLPRIVATE void     SetWidthAndHeight();
    SVT_DLLPRIVATE void     InitData( bool bImagesFromDefault,
                                      bool _bRadioBtn, const Control* pControlForSettings = NULL );
public:
                            // include creating default images (CheckBox or RadioButton)
                            SvLBoxButtonData( const Control* pControlForSettings );
                            SvLBoxButtonData( const Control* pControlForSettings, bool _bRadioBtn );

                            ~SvLBoxButtonData();

    SvBmp                   GetIndex( sal_uInt16 nItemState );
    long                    Width();
    long                    Height();
    void                    SetLink( const Link& rLink) { aLink=rLink; }
    const Link&             GetLink() const { return aLink; }
    bool                    IsRadio();
    // weil Buttons nicht von LinkHdl abgeleitet sind
    void                    CallLink();

    void                    StoreButtonState( SvTreeListEntry* pEntry, sal_uInt16 nItemFlags );
    SvButtonState           ConvertToButtonState( sal_uInt16 nItemFlags ) const;

    SvButtonState           GetActButtonState() const { return eState; }

    SvTreeListEntry*        GetActEntry() const;

    void                    SetImage(SvBmp nIndex, const Image& aImage) { aBmps[(int)nIndex] = aImage; }
    Image&                  GetImage(SvBmp nIndex) { return aBmps[(int)nIndex]; }

    void                    SetDefaultImages( const Control* pControlForSettings = NULL );
                                // set images according to the color scheeme of the Control
                                // pControlForSettings == NULL: settings are taken from Application
    bool                    HasDefaultImages( void ) const;
};

// **********************************************************************

class SVT_DLLPUBLIC SvLBoxString : public SvLBoxItem
{
protected:
    OUString maText;
public:
                    SvLBoxString(SvTreeListEntry*, sal_uInt16 nFlags, const OUString& rStr);
                    SvLBoxString();
    virtual         ~SvLBoxString();
    virtual sal_uInt16 GetType() const SAL_OVERRIDE;
    virtual void    InitViewData(SvTreeListBox*, SvTreeListEntry*, SvViewDataItem*) SAL_OVERRIDE;
    OUString   GetText() const { return maText; }
    virtual OUString GetExtendText() const { return OUString(); }
    void            SetText( const OUString& rText ) { maText = rText; }

    virtual void Paint(
        const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry) SAL_OVERRIDE;

    virtual SvLBoxItem* Create() const SAL_OVERRIDE;
    virtual void    Clone( SvLBoxItem* pSource ) SAL_OVERRIDE;
};

class SvLBoxBmp : public SvLBoxItem
{
    Image aBmp;
public:
                    SvLBoxBmp();
    virtual         ~SvLBoxBmp();
    virtual sal_uInt16 GetType() const SAL_OVERRIDE;
    virtual void    InitViewData( SvTreeListBox*,SvTreeListEntry*,SvViewDataItem* ) SAL_OVERRIDE;
    virtual void Paint(
        const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry) SAL_OVERRIDE;
    virtual SvLBoxItem* Create() const SAL_OVERRIDE;
    virtual void    Clone( SvLBoxItem* pSource ) SAL_OVERRIDE;
};


#define SV_ITEMSTATE_UNCHECKED          0x0001
#define SV_ITEMSTATE_CHECKED            0x0002
#define SV_ITEMSTATE_TRISTATE           0x0004
#define SV_ITEMSTATE_HILIGHTED          0x0008
#define SV_STATE_MASK 0xFFF8  // zum Loeschen von UNCHECKED,CHECKED,TRISTATE

class SVT_DLLPUBLIC SvLBoxButton : public SvLBoxItem
{
    bool    isVis;
    SvLBoxButtonData*   pData;
    SvLBoxButtonKind eKind;
    sal_uInt16 nItemFlags;

    void ImplAdjustBoxSize( Size& io_rCtrlSize, ControlType i_eType, vcl::Window* pParent );
public:
                    // An SvLBoxButton can be of three different kinds: an
                    // enabled checkbox (the normal kind), a disabled checkbox
                    // (which cannot be modified via UI), or a static image
                    // (see SV_BMP_STATICIMAGE; nFlags are effectively ignored
                    // for that kind).
                    SvLBoxButton( SvTreeListEntry* pEntry,
                                  SvLBoxButtonKind eTheKind, sal_uInt16 nFlags,
                                  SvLBoxButtonData* pBData );
                    SvLBoxButton();
    virtual         ~SvLBoxButton();
    virtual void    InitViewData( SvTreeListBox*,SvTreeListEntry*,SvViewDataItem* ) SAL_OVERRIDE;
    virtual sal_uInt16 GetType() const SAL_OVERRIDE;
    virtual bool    ClickHdl(SvTreeListBox* pView, SvTreeListEntry* );
    virtual void Paint(
        const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry) SAL_OVERRIDE;
    virtual SvLBoxItem* Create() const SAL_OVERRIDE;
    virtual void    Clone( SvLBoxItem* pSource ) SAL_OVERRIDE;
    sal_uInt16          GetButtonFlags() const { return nItemFlags; }
    bool            IsStateChecked() const { return (nItemFlags & SV_ITEMSTATE_CHECKED)!=0; }
    bool            IsStateUnchecked() const { return (nItemFlags & SV_ITEMSTATE_UNCHECKED)!=0; }
    bool            IsStateTristate() const { return (nItemFlags & SV_ITEMSTATE_TRISTATE)!=0; }
    bool            IsStateHilighted() const { return (nItemFlags & SV_ITEMSTATE_HILIGHTED)!=0; }
    void            SetStateChecked();
    void            SetStateUnchecked();
    void            SetStateTristate();
    void            SetStateHilighted( bool bHilight );
    void            SetStateInvisible();

    SvLBoxButtonKind GetKind() const { return eKind; }

    // Check whether this button can be modified via UI
    bool            CheckModification() const;
    SvLBoxButtonData* GetButtonData() const{ return pData;}
};

inline void SvLBoxButton::SetStateChecked()
{
    nItemFlags &= SV_STATE_MASK;
    nItemFlags |= SV_ITEMSTATE_CHECKED;
}
inline void SvLBoxButton::SetStateUnchecked()
{
    nItemFlags &= SV_STATE_MASK;
    nItemFlags |= SV_ITEMSTATE_UNCHECKED;
}
inline void SvLBoxButton::SetStateTristate()
{
    nItemFlags &= SV_STATE_MASK;
    nItemFlags |= SV_ITEMSTATE_TRISTATE;
}
inline void SvLBoxButton::SetStateHilighted( bool bHilight )
{
    if ( bHilight )
        nItemFlags |= SV_ITEMSTATE_HILIGHTED;
    else
        nItemFlags &= ~SV_ITEMSTATE_HILIGHTED;
}


struct SvLBoxContextBmp_Impl;
class SVT_DLLPUBLIC SvLBoxContextBmp : public SvLBoxItem
{
    SvLBoxContextBmp_Impl*  m_pImpl;
public:
    SvLBoxContextBmp(
        SvTreeListEntry* pEntry, sal_uInt16 nItemFlags, Image aBmp1, Image aBmp2, bool bExpanded);
    SvLBoxContextBmp();

    virtual         ~SvLBoxContextBmp();
    virtual sal_uInt16 GetType() const SAL_OVERRIDE;
    virtual void    InitViewData( SvTreeListBox*,SvTreeListEntry*,SvViewDataItem* ) SAL_OVERRIDE;
    virtual void Paint(
        const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry) SAL_OVERRIDE;
    virtual SvLBoxItem* Create() const SAL_OVERRIDE;
    virtual void    Clone( SvLBoxItem* pSource ) SAL_OVERRIDE;


    bool            SetModeImages( const Image& _rBitmap1, const Image& _rBitmap2 );
    void            GetModeImages(       Image& _rBitmap1,       Image& _rBitmap2 ) const;

    inline void         SetBitmap1( const Image& _rImage );
    inline void         SetBitmap2( const Image& _rImage );
    inline const Image& GetBitmap1( ) const;
    inline const Image& GetBitmap2( ) const;

private:
    Image& implGetImageStore( bool _bFirst );
};

inline void SvLBoxContextBmp::SetBitmap1( const Image& _rImage  )
{
    implGetImageStore( true ) = _rImage;
}

inline void SvLBoxContextBmp::SetBitmap2( const Image& _rImage )
{
    implGetImageStore( false ) = _rImage;
}

inline const Image& SvLBoxContextBmp::GetBitmap1( ) const
{
    Image& rImage = const_cast< SvLBoxContextBmp* >( this )->implGetImageStore( true );
    return rImage;
}

inline const Image& SvLBoxContextBmp::GetBitmap2( ) const
{
    Image& rImage = const_cast< SvLBoxContextBmp* >( this )->implGetImageStore( false );
    return rImage;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
