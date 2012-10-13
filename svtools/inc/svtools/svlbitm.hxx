/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifndef _SVLBOXITM_HXX
#define _SVLBOXITM_HXX

#include "svtools/svtdllapi.h"

#include <tools/link.hxx>

#include <vcl/image.hxx>
#include <svtools/treelistbox.hxx>

class SvLBoxEntry;


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
    inline long             Width();
    inline long             Height();
    void                    SetLink( const Link& rLink) { aLink=rLink; }
    const Link&             GetLink() const { return aLink; }
     sal_Bool                   IsRadio();
    // weil Buttons nicht von LinkHdl abgeleitet sind
    void                    CallLink();

    void                    StoreButtonState( SvLBoxEntry* pEntry, sal_uInt16 nItemFlags );
    SvButtonState           ConvertToButtonState( sal_uInt16 nItemFlags ) const;

    inline SvButtonState    GetActButtonState() const;
    SvLBoxEntry*            GetActEntry() const;

    Image aBmps[24];  // Indizes siehe Konstanten BMP_ ....

    void                    SetDefaultImages( const Control* pControlForSettings = NULL );
                                // set images acording to the color scheeme of the Control
                                // pControlForSettings == NULL: settings are taken from Application
    sal_Bool                    HasDefaultImages( void ) const;
};

inline long SvLBoxButtonData::Width()
{
    if ( !bDataOk )
        SetWidthAndHeight();
    return nWidth;
}

inline long SvLBoxButtonData::Height()
{
    if ( !bDataOk )
        SetWidthAndHeight();
    return nHeight;
}

inline SvButtonState SvLBoxButtonData::GetActButtonState() const
{
    return eState;
}

// **********************************************************************

class SVT_DLLPUBLIC SvLBoxString : public SvLBoxItem
{
protected:
    rtl::OUString maText;
public:
                    SvLBoxString(SvLBoxEntry*, sal_uInt16 nFlags, const rtl::OUString& rStr);
                    SvLBoxString();
    virtual         ~SvLBoxString();
    virtual sal_uInt16  IsA();
    virtual void    InitViewData(SvTreeListBox*, SvLBoxEntry*, SvViewDataItem*);
    rtl::OUString   GetText() const { return maText; }
    void            SetText( const rtl::OUString& rText ) { maText = rText; }
    virtual void    Paint( const Point&, SvTreeListBox& rDev, sal_uInt16 nFlags,SvLBoxEntry* );
    virtual SvLBoxItem* Create() const;
    virtual void    Clone( SvLBoxItem* pSource );
};

class SvLBoxBmp : public SvLBoxItem
{
    Image aBmp;
public:
                    SvLBoxBmp();
    virtual         ~SvLBoxBmp();
    virtual sal_uInt16  IsA();
    virtual void    InitViewData( SvTreeListBox*,SvLBoxEntry*,SvViewDataItem* );
    virtual void    Paint( const Point&, SvTreeListBox& rView, sal_uInt16 nFlags,SvLBoxEntry* );
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
                    SvLBoxButton( SvLBoxEntry* pEntry,
                                  SvLBoxButtonKind eTheKind, sal_uInt16 nFlags,
                                  SvLBoxButtonData* pBData );
                    SvLBoxButton();
    virtual         ~SvLBoxButton();
    virtual void    InitViewData( SvTreeListBox*,SvLBoxEntry*,SvViewDataItem* );
    virtual sal_uInt16  IsA();
    virtual sal_Bool    ClickHdl(SvTreeListBox* pView, SvLBoxEntry* );
    virtual void    Paint( const Point&, SvTreeListBox& rView, sal_uInt16 nFlags,SvLBoxEntry* );
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

    // Check whether this button can be modified via UI, sounding a beep if it
    // cannot be modified:
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
                    SvLBoxContextBmp( SvLBoxEntry*,sal_uInt16 nFlags,Image,Image,
                                    sal_uInt16 nEntryFlagsBmp1);
                    SvLBoxContextBmp();
    virtual         ~SvLBoxContextBmp();
    virtual sal_uInt16  IsA();
    virtual void    InitViewData( SvTreeListBox*,SvLBoxEntry*,SvViewDataItem* );
    virtual void    Paint( const Point&, SvTreeListBox& rView, sal_uInt16 nFlags,SvLBoxEntry* );
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
