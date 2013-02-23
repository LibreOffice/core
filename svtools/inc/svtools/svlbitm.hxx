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


#ifndef _SVLBOXITM_HXX
#define _SVLBOXITM_HXX

#include "svtools/svtdllapi.h"

#include <tools/link.hxx>

#include <vcl/image.hxx>
#include <svtools/treelistbox.hxx>

class SvTreeListEntry;


#define SV_BMP_UNCHECKED        0
#define SV_BMP_CHECKED          1
#define SV_BMP_TRISTATE         2
#define SV_BMP_HIUNCHECKED      3
#define SV_BMP_HICHECKED        4
#define SV_BMP_HITRISTATE       5
#define SV_BMP_STATICIMAGE      6

struct SvLBoxButtonData_Impl;

class SVT_DLLPUBLIC SvLBoxButtonData
{
private:
    Link                    aLink;
    long                    nWidth;
    long                    nHeight;
    SvLBoxButtonData_Impl*  pImpl;
    sal_Bool                    bDataOk;
    SvButtonState           eState;

    SVT_DLLPRIVATE void                     SetWidthAndHeight();
    SVT_DLLPRIVATE void                 InitData( sal_Bool bImagesFromDefault,
                                      bool _bRadioBtn, const Control* pControlForSettings = NULL );
public:
                            // include creating default images (CheckBox or RadioButton)
                            SvLBoxButtonData( const Control* pControlForSettings );
                            SvLBoxButtonData( const Control* pControlForSettings, bool _bRadioBtn );

                            ~SvLBoxButtonData();

    sal_uInt16                  GetIndex( sal_uInt16 nItemState );
    long                    Width();
    long                    Height();
    void                    SetLink( const Link& rLink) { aLink=rLink; }
    const Link&             GetLink() const { return aLink; }
     sal_Bool                   IsRadio();
    // weil Buttons nicht von LinkHdl abgeleitet sind
    void                    CallLink();

    void                    StoreButtonState( SvTreeListEntry* pEntry, sal_uInt16 nItemFlags );
    SvButtonState           ConvertToButtonState( sal_uInt16 nItemFlags ) const;

    SvButtonState GetActButtonState() const
    {
        return eState;
    }

    SvTreeListEntry*            GetActEntry() const;

    Image aBmps[24];  // Indizes siehe Konstanten BMP_ ....

    void                    SetDefaultImages( const Control* pControlForSettings = NULL );
                                // set images acording to the color scheeme of the Control
                                // pControlForSettings == NULL: settings are taken from Application
    sal_Bool                    HasDefaultImages( void ) const;
};

// **********************************************************************

class SVT_DLLPUBLIC SvLBoxString : public SvLBoxItem
{
protected:
    rtl::OUString maText;
public:
                    SvLBoxString(SvTreeListEntry*, sal_uInt16 nFlags, const rtl::OUString& rStr);
                    SvLBoxString();
    virtual         ~SvLBoxString();
    virtual sal_uInt16 GetType() const;
    virtual void    InitViewData(SvTreeListBox*, SvTreeListEntry*, SvViewDataItem*);
    rtl::OUString   GetText() const { return maText; }
    void            SetText( const rtl::OUString& rText ) { maText = rText; }

    virtual void Paint(
        const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry);

    virtual SvLBoxItem* Create() const;
    virtual void    Clone( SvLBoxItem* pSource );
};

class SvLBoxBmp : public SvLBoxItem
{
    Image aBmp;
public:
                    SvLBoxBmp();
    virtual         ~SvLBoxBmp();
    virtual sal_uInt16 GetType() const;
    virtual void    InitViewData( SvTreeListBox*,SvTreeListEntry*,SvViewDataItem* );
    virtual void Paint(
        const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry);
    virtual SvLBoxItem* Create() const;
    virtual void    Clone( SvLBoxItem* pSource );
};


#define SV_ITEMSTATE_UNCHECKED          0x0001
#define SV_ITEMSTATE_CHECKED            0x0002
#define SV_ITEMSTATE_TRISTATE           0x0004
#define SV_ITEMSTATE_HILIGHTED          0x0008
#define SV_STATE_MASK 0xFFF8  // zum Loeschen von UNCHECKED,CHECKED,TRISTATE

class SVT_DLLPUBLIC SvLBoxButton : public SvLBoxItem
{
    SvLBoxButtonData*   pData;
    SvLBoxButtonKind eKind;
    sal_uInt16 nItemFlags;
    sal_uInt16 nBaseOffs;

    void ImplAdjustBoxSize( Size& io_rCtrlSize, ControlType i_eType, Window* pParent );
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
    virtual void    InitViewData( SvTreeListBox*,SvTreeListEntry*,SvViewDataItem* );
    virtual sal_uInt16 GetType() const;
    virtual sal_Bool    ClickHdl(SvTreeListBox* pView, SvTreeListEntry* );
    virtual void Paint(
        const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry);
    virtual SvLBoxItem* Create() const;
    virtual void    Clone( SvLBoxItem* pSource );
    sal_uInt16          GetButtonFlags() const { return nItemFlags; }
    sal_Bool            IsStateChecked() const { return (sal_Bool)(nItemFlags & SV_ITEMSTATE_CHECKED)!=0; }
    sal_Bool            IsStateUnchecked() const { return (sal_Bool)(nItemFlags & SV_ITEMSTATE_UNCHECKED)!=0; }
    sal_Bool            IsStateTristate() const { return (sal_Bool)(nItemFlags & SV_ITEMSTATE_TRISTATE)!=0; }
    sal_Bool            IsStateHilighted() const { return (sal_Bool)(nItemFlags & SV_ITEMSTATE_HILIGHTED)!=0; }
    void            SetStateChecked();
    void            SetStateUnchecked();
    void            SetStateTristate();
    void            SetStateHilighted( sal_Bool bHilight );

    SvLBoxButtonKind GetKind() const { return eKind; }

    void            SetBaseOffs( sal_uInt16 nOffs ) { nBaseOffs = nOffs; }
    sal_uInt16          GetBaseOffs() const { return nBaseOffs; }

    // Check whether this button can be modified via UI
    bool            CheckModification() const;
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
inline void SvLBoxButton::SetStateHilighted( sal_Bool bHilight )
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
    virtual sal_uInt16 GetType() const;
    virtual void    InitViewData( SvTreeListBox*,SvTreeListEntry*,SvViewDataItem* );
    virtual void Paint(
        const Point& rPos, SvTreeListBox& rOutDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry);
    virtual SvLBoxItem* Create() const;
    virtual void    Clone( SvLBoxItem* pSource );


    sal_Bool            SetModeImages( const Image& _rBitmap1, const Image& _rBitmap2 );
    void            GetModeImages(       Image& _rBitmap1,       Image& _rBitmap2 ) const;

    inline void         SetBitmap1( const Image& _rImage );
    inline void         SetBitmap2( const Image& _rImage );
    inline const Image& GetBitmap1( ) const;
    inline const Image& GetBitmap2( ) const;

private:
    Image& implGetImageStore( sal_Bool _bFirst );
};

inline void SvLBoxContextBmp::SetBitmap1( const Image& _rImage  )
{
    implGetImageStore( sal_True ) = _rImage;
}

inline void SvLBoxContextBmp::SetBitmap2( const Image& _rImage )
{
    implGetImageStore( sal_False ) = _rImage;
}

inline const Image& SvLBoxContextBmp::GetBitmap1( ) const
{
    Image& rImage = const_cast< SvLBoxContextBmp* >( this )->implGetImageStore( sal_True );
    return rImage;
}

inline const Image& SvLBoxContextBmp::GetBitmap2( ) const
{
    Image& rImage = const_cast< SvLBoxContextBmp* >( this )->implGetImageStore( sal_False );
    return rImage;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
