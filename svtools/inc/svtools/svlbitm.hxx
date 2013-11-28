/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#ifndef _SVLBOXITM_HXX
#define _SVLBOXITM_HXX

#include "svtools/svtdllapi.h"

#ifndef LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _IMAGE_HXX
#include <vcl/image.hxx>
#endif
#include <svtools/svlbox.hxx>

class SvLBoxEntry;

#define SV_ITEM_ID_LBOXSTRING       1
#define SV_ITEM_ID_LBOXBMP          2
#define SV_ITEM_ID_LBOXBUTTON       3
#define SV_ITEM_ID_LBOXCONTEXTBMP   4
#define SV_ITEM_ID_EXTENDRLBOXSTRING    5

enum SvButtonState { SV_BUTTON_UNCHECKED, SV_BUTTON_CHECKED, SV_BUTTON_TRISTATE };

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

                            SvLBoxButtonData();
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
    XubString aStr;
public:
                    SvLBoxString( SvLBoxEntry*,sal_uInt16 nFlags,const XubString& rStr);
                    SvLBoxString();
    virtual         ~SvLBoxString();
    virtual sal_uInt16  IsA();
    void            InitViewData( SvLBox*,SvLBoxEntry*,SvViewDataItem* );
    XubString       GetText() const { return aStr; }
    virtual XubString       GetExtendText() const {return XubString();}
    void            SetText( SvLBoxEntry*, const XubString& rStr );
    void            Paint( const Point&, SvLBox& rDev, sal_uInt16 nFlags,SvLBoxEntry* );
    SvLBoxItem*     Create() const;
    void            Clone( SvLBoxItem* pSource );
};

class SvLBoxBmp : public SvLBoxItem
{
    Image aBmp;
public:
                    SvLBoxBmp( SvLBoxEntry*, sal_uInt16 nFlags, Image );
                    SvLBoxBmp();
    virtual         ~SvLBoxBmp();
    virtual sal_uInt16  IsA();
    void            InitViewData( SvLBox*,SvLBoxEntry*,SvViewDataItem* );
    void            SetBitmap( SvLBoxEntry*, Image );
    void            Paint( const Point&, SvLBox& rView, sal_uInt16 nFlags,SvLBoxEntry* );
    SvLBoxItem*     Create() const;
    void            Clone( SvLBoxItem* pSource );
};


#define SV_ITEMSTATE_UNCHECKED          0x0001
#define SV_ITEMSTATE_CHECKED            0x0002
#define SV_ITEMSTATE_TRISTATE           0x0004
#define SV_ITEMSTATE_HILIGHTED          0x0008
#define SV_STATE_MASK 0xFFF8  // zum Loeschen von UNCHECKED,CHECKED,TRISTATE

enum SvLBoxButtonKind
{
    SvLBoxButtonKind_enabledCheckbox,
    SvLBoxButtonKind_disabledCheckbox,
    SvLBoxButtonKind_staticImage
};

class SVT_DLLPUBLIC SvLBoxButton : public SvLBoxItem
{
    SvLBoxButtonData*   pData;
    SvLBoxButtonKind eKind;
    sal_uInt16 nItemFlags;
    sal_uInt16 nImgArrOffs;
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
    void            InitViewData( SvLBox*,SvLBoxEntry*,SvViewDataItem* );
    virtual sal_uInt16  IsA();
    void            Check( SvLBox* pView, SvLBoxEntry*, sal_Bool bCheck );
    virtual sal_Bool    ClickHdl(SvLBox* pView, SvLBoxEntry* );
    void            Paint( const Point&, SvLBox& rView, sal_uInt16 nFlags,SvLBoxEntry* );
    SvLBoxItem*     Create() const;
    void            Clone( SvLBoxItem* pSource );
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
    void            InitViewData( SvLBox*,SvLBoxEntry*,SvViewDataItem* );
    void            Paint( const Point&, SvLBox& rView, sal_uInt16 nFlags,SvLBoxEntry* );
    SvLBoxItem*     Create() const;
    void            Clone( SvLBoxItem* pSource );


    sal_Bool            SetModeImages( const Image& _rBitmap1, const Image& _rBitmap2, BmpColorMode _eMode = BMP_COLOR_NORMAL );
    void            GetModeImages(       Image& _rBitmap1,       Image& _rBitmap2, BmpColorMode _eMode = BMP_COLOR_NORMAL ) const;

    inline void         SetBitmap1( const Image& _rImage, BmpColorMode _eMode = BMP_COLOR_NORMAL );
    inline void         SetBitmap2( const Image& _rImage, BmpColorMode _eMode = BMP_COLOR_NORMAL );
    inline const Image& GetBitmap1( BmpColorMode _eMode = BMP_COLOR_NORMAL ) const;
    inline const Image& GetBitmap2( BmpColorMode _eMode = BMP_COLOR_NORMAL ) const;

private:
    Image& implGetImageStore( sal_Bool _bFirst, BmpColorMode _eMode );
};

inline void SvLBoxContextBmp::SetBitmap1( const Image& _rImage, BmpColorMode _eMode  )
{
    implGetImageStore( sal_True, _eMode ) = _rImage;
}

inline void SvLBoxContextBmp::SetBitmap2( const Image& _rImage, BmpColorMode _eMode  )
{
    implGetImageStore( sal_False, _eMode ) = _rImage;
}

inline const Image& SvLBoxContextBmp::GetBitmap1( BmpColorMode _eMode ) const
{
    Image& rImage = const_cast< SvLBoxContextBmp* >( this )->implGetImageStore( sal_True, _eMode );
    if ( !rImage )
        // fallback to the "normal" image
        rImage = const_cast< SvLBoxContextBmp* >( this )->implGetImageStore( sal_True, BMP_COLOR_NORMAL );
    return rImage;
}

inline const Image& SvLBoxContextBmp::GetBitmap2( BmpColorMode _eMode ) const
{
    Image& rImage = const_cast< SvLBoxContextBmp* >( this )->implGetImageStore( sal_False, _eMode );
    if ( !rImage )
        // fallback to the "normal" image
        rImage = const_cast< SvLBoxContextBmp* >( this )->implGetImageStore( sal_True, BMP_COLOR_NORMAL );
    return rImage;
}

#endif
