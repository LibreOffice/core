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


#ifndef _GRFATR_HXX
#define _GRFATR_HXX

#include <hintids.hxx>      // fuer die WhichIds @@@ must be included first @@@
#include <tools/gen.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svx/grfcrop.hxx>
#include "swdllapi.h"
#include <swatrset.hxx>     // fuer inlines
#include <format.hxx>       // fuer inlines

/******************************************************************************
 *  class SwMirrorGrf
 ******************************************************************************/

enum MirrorGraph
{
RES_MIRROR_GRAPH_BEGIN,
    RES_MIRROR_GRAPH_DONT = RES_MIRROR_GRAPH_BEGIN,
    RES_MIRROR_GRAPH_VERT,
    RES_MIRROR_GRAPH_HOR,
    RES_MIRROR_GRAPH_BOTH,
RES_MIRROR_GRAPH_END
};

class SW_DLLPUBLIC SwMirrorGrf : public SfxEnumItem
{
    sal_Bool bGrfToggle; // auf geraden Seiten Grafiken spiegeln

public:
    SwMirrorGrf( MirrorGraph eMiro = RES_MIRROR_GRAPH_DONT )
        : SfxEnumItem( RES_GRFATR_MIRRORGRF, static_cast< sal_uInt16 >(eMiro) ), bGrfToggle( sal_False )
    {}
    SwMirrorGrf( const SwMirrorGrf &rMirrorGrf )
        : SfxEnumItem( RES_GRFATR_MIRRORGRF, rMirrorGrf.GetValue()),
        bGrfToggle( rMirrorGrf.IsGrfToggle() )
    {}

    // pure virtual-Methoden von SfxPoolItem
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;

    // pure virtual-Methiden von SfxEnumItem
    virtual sal_uInt16          GetValueCount() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 );

    inline SwMirrorGrf& operator=( const SwMirrorGrf& rMirrorGrf )
        {
            SfxEnumItem::SetValue( rMirrorGrf.GetValue() );
            bGrfToggle = rMirrorGrf.IsGrfToggle();
            return *this;
        }

    inline sal_Bool IsGrfToggle() const         { return bGrfToggle; }
    inline void SetGrfToggle( sal_Bool bNew )   { bGrfToggle = bNew; }
};


/******************************************************************************
 *  class SwAttrCropGrf
 ******************************************************************************/

class SW_DLLPUBLIC SwCropGrf : public SvxGrfCrop
{
public:
    SwCropGrf();
    SwCropGrf(  sal_Int32 nLeft,    sal_Int32 nRight,
                sal_Int32 nTop,     sal_Int32 nBottom );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
};

class SwRotationGrf : public SfxUInt16Item
{
    Size aUnrotatedSize;
public:
    SwRotationGrf( sal_Int16 nVal = 0 )
        : SfxUInt16Item( RES_GRFATR_ROTATION, nVal )
    {}
    SwRotationGrf( sal_Int16 nVal, const Size& rSz )
        : SfxUInt16Item( RES_GRFATR_ROTATION, nVal ), aUnrotatedSize( rSz )
    {}

    // pure virtual-Methiden from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal,
                                            sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal,
                                            sal_uInt8 nMemberId = 0 );

    void SetUnrotatedSize( const Size& rSz )        { aUnrotatedSize = rSz; }
    const Size& GetUnrotatedSize() const            { return aUnrotatedSize; }
};

class SW_DLLPUBLIC SwLuminanceGrf : public SfxInt16Item
{
public:
    SwLuminanceGrf( sal_Int16 nVal = 0 )
        : SfxInt16Item( RES_GRFATR_LUMINANCE, nVal )
    {}

    // pure virtual-Methiden from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;
};

class SW_DLLPUBLIC SwContrastGrf : public SfxInt16Item
{
public:
    SwContrastGrf( sal_Int16 nVal = 0 )
        : SfxInt16Item( RES_GRFATR_CONTRAST, nVal )
    {}

    // pure virtual-Methiden from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;
};

class SwChannelGrf : public SfxInt16Item
{
protected:
    SwChannelGrf( sal_Int16 nVal, sal_uInt16 nWhichL )
        : SfxInt16Item( nWhichL, nVal )
    {}

public:
    // pure virtual-Methiden from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;
};

class SwChannelRGrf : public SwChannelGrf
{
public:
    SwChannelRGrf( sal_Int16 nVal = 0 )
        : SwChannelGrf( nVal, RES_GRFATR_CHANNELR )
    {}
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
};
class SwChannelGGrf : public SwChannelGrf
{
public:
    SwChannelGGrf( sal_Int16 nVal = 0 )
        : SwChannelGrf( nVal, RES_GRFATR_CHANNELG )
    {}
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
};
class SwChannelBGrf : public SwChannelGrf
{
public:
    SwChannelBGrf( sal_Int16 nVal = 0 )
        : SwChannelGrf( nVal, RES_GRFATR_CHANNELB )
    {}
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
};

class SW_DLLPUBLIC SwGammaGrf : public SfxPoolItem
{
    double nValue;
public:
    SwGammaGrf() : SfxPoolItem( RES_GRFATR_GAMMA ), nValue( 1.0 )
    {}

    SwGammaGrf( const double& rVal )
        : SfxPoolItem( RES_GRFATR_GAMMA ), nValue( rVal )
    {}

    inline SwGammaGrf& operator=( const SwGammaGrf& rCopy )
        {
            SetValue( rCopy.GetValue() );
            return *this;
        }

    // pure virtual-Methiden von SfxEnumItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal,
                                            sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal,
                                            sal_uInt8 nMemberId = 0 );


    const double& GetValue() const              { return nValue; }
    void SetValue( const double& rVal )         { nValue = rVal; }
};

class SwInvertGrf: public SfxBoolItem
{
public:
    SwInvertGrf( sal_Bool bVal = sal_False )
        : SfxBoolItem( RES_GRFATR_INVERT, bVal )
    {}

    // pure virtual-Methiden from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;
};

class SwTransparencyGrf : public SfxByteItem
{
public:
    SwTransparencyGrf( sal_Int8 nVal = 0 )
        : SfxByteItem( RES_GRFATR_TRANSPARENCY, nVal )
    {}

    // pure virtual-Methiden from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;
    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 );
};

class SW_DLLPUBLIC SwDrawModeGrf : public SfxEnumItem
{
public:
    SwDrawModeGrf( sal_uInt16 nMode = 0 )
        : SfxEnumItem( RES_GRFATR_DRAWMODE, nMode )
    {}

    // pure virtual-Methoden von SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    // pure virtual-Methiden von SfxEnumItem
    virtual sal_uInt16          GetValueCount() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 );
};



/******************************************************************************
 *  Implementierung der GrafikAttribut Methoden vom SwAttrSet
 ******************************************************************************/

inline const SwMirrorGrf &SwAttrSet::GetMirrorGrf(sal_Bool bInP) const
    { return (const SwMirrorGrf&)Get( RES_GRFATR_MIRRORGRF,bInP); }
inline const SwCropGrf   &SwAttrSet::GetCropGrf(sal_Bool bInP) const
    { return (const SwCropGrf&)Get( RES_GRFATR_CROPGRF,bInP); }
inline const SwRotationGrf &SwAttrSet::GetRotationGrf(sal_Bool bInP) const
    { return (const SwRotationGrf&)Get( RES_GRFATR_ROTATION,bInP); }
inline const SwLuminanceGrf &SwAttrSet::GetLuminanceGrf(sal_Bool bInP) const
    { return (const SwLuminanceGrf&)Get( RES_GRFATR_LUMINANCE,bInP); }
inline const SwContrastGrf &SwAttrSet::GetContrastGrf(sal_Bool bInP) const
    { return (const SwContrastGrf&)Get( RES_GRFATR_CONTRAST,bInP); }
inline const SwChannelRGrf &SwAttrSet::GetChannelRGrf(sal_Bool bInP) const
    { return (const SwChannelRGrf&)Get( RES_GRFATR_CHANNELR,bInP); }
inline const SwChannelGGrf &SwAttrSet::GetChannelGGrf(sal_Bool bInP) const
    { return (const SwChannelGGrf&)Get( RES_GRFATR_CHANNELG,bInP); }
inline const SwChannelBGrf &SwAttrSet::GetChannelBGrf(sal_Bool bInP) const
    { return (const SwChannelBGrf&)Get( RES_GRFATR_CHANNELB,bInP); }
inline const SwGammaGrf &SwAttrSet::GetGammaGrf(sal_Bool bInP) const
    { return (const SwGammaGrf&)Get( RES_GRFATR_GAMMA,bInP); }
inline const SwInvertGrf &SwAttrSet::GetInvertGrf(sal_Bool bInP) const
    { return (const SwInvertGrf&)Get( RES_GRFATR_INVERT,bInP); }
inline const SwTransparencyGrf &SwAttrSet::GetTransparencyGrf(sal_Bool bInP) const
    { return (const SwTransparencyGrf&)Get( RES_GRFATR_TRANSPARENCY,bInP); }
inline const SwDrawModeGrf      &SwAttrSet::GetDrawModeGrf(sal_Bool bInP) const
    { return (const SwDrawModeGrf&)Get( RES_GRFATR_DRAWMODE,bInP); }

/******************************************************************************
 *  Implementierung der GrafikAttribut Methoden vom SwFmt
 ******************************************************************************/

inline const SwMirrorGrf &SwFmt::GetMirrorGrf(sal_Bool bInP) const
    { return aSet.GetMirrorGrf(bInP); }
inline const SwCropGrf   &SwFmt::GetCropGrf(sal_Bool bInP) const
    { return aSet.GetCropGrf(bInP); }
inline const SwRotationGrf &SwFmt::GetRotationGrf(sal_Bool bInP) const
    { return aSet.GetRotationGrf(bInP); }
inline const SwLuminanceGrf &SwFmt::GetLuminanceGrf(sal_Bool bInP) const
    { return aSet.GetLuminanceGrf( bInP); }
inline const SwContrastGrf &SwFmt::GetContrastGrf(sal_Bool bInP) const
    { return aSet.GetContrastGrf( bInP); }
inline const SwChannelRGrf &SwFmt::GetChannelRGrf(sal_Bool bInP) const
    { return aSet.GetChannelRGrf( bInP); }
inline const SwChannelGGrf &SwFmt::GetChannelGGrf(sal_Bool bInP) const
    { return aSet.GetChannelGGrf( bInP); }
inline const SwChannelBGrf &SwFmt::GetChannelBGrf(sal_Bool bInP) const
    { return aSet.GetChannelBGrf( bInP); }
inline const SwGammaGrf &SwFmt::GetGammaGrf(sal_Bool bInP) const
    { return aSet.GetGammaGrf( bInP); }
inline const SwInvertGrf &SwFmt::GetInvertGrf(sal_Bool bInP) const
    { return aSet.GetInvertGrf( bInP); }
inline const SwTransparencyGrf &SwFmt::GetTransparencyGrf(sal_Bool bInP) const
    { return aSet.GetTransparencyGrf( bInP); }
inline const SwDrawModeGrf &SwFmt::GetDrawModeGrf(sal_Bool bInP) const
    { return aSet.GetDrawModeGrf(bInP); }


#endif  // _GRFATR_HXX
