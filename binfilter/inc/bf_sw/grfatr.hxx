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
#ifndef _GRFATR_HXX
#define _GRFATR_HXX

#include <hintids.hxx>		// fuer die WhichIds

#include <tools/gen.hxx>
#include <bf_svtools/eitem.hxx>
#include <bf_svtools/intitem.hxx>
#include <bf_svx/grfcrop.hxx>

#include <swatrset.hxx>		// fuer inlines
#include <format.hxx>		// fuer inlines
namespace binfilter {

/******************************************************************************
 *	class SwMirrorGrf
 ******************************************************************************/

enum GRFMIRROR
{
RES_GRFMIRROR_BEGIN,
    RES_DONT_MIRROR_GRF = RES_GRFMIRROR_BEGIN,
    RES_MIRROR_GRF_VERT,
    RES_MIRROR_GRF_HOR,
    RES_MIRROR_GRF_BOTH,
RES_GRFMIRROR_END
};

class SwMirrorGrf : public SfxEnumItem
{
    BOOL bGrfToggle; // auf geraden Seiten Grafiken spiegeln

public:
    SwMirrorGrf( USHORT nMiro = RES_DONT_MIRROR_GRF )
        : SfxEnumItem( RES_GRFATR_MIRRORGRF, nMiro ), bGrfToggle( sal_False )
    {}
    SwMirrorGrf( const SwMirrorGrf &rMirrorGrf )
        : SfxEnumItem( RES_GRFATR_MIRRORGRF, rMirrorGrf.GetValue()),
        bGrfToggle( rMirrorGrf.IsGrfToggle() )
    {}

    // pure virtual-Methoden von SfxPoolItem
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;

    // pure virtual-Methiden von SfxEnumItem
    virtual USHORT          GetValueCount() const;
    virtual USHORT			GetVersion( USHORT nFFVer ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer ) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;
    virtual int				operator==( const SfxPoolItem& ) const;

    virtual	bool            QueryValue( ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 ) const;
    virtual	bool            PutValue( const ::com::sun::star::uno::Any& rVal,
                                      BYTE nMemberId = 0 );

    inline SwMirrorGrf& operator=( const SwMirrorGrf& rMirrorGrf )
        {
            SfxEnumItem::SetValue( rMirrorGrf.GetValue() );
            bGrfToggle = rMirrorGrf.IsGrfToggle();
            return *this;
        }

    inline BOOL IsGrfToggle() const			{ return bGrfToggle; }
    inline void SetGrfToggle( BOOL bNew )	{ bGrfToggle = bNew; }
};


/******************************************************************************
 *	class SwAttrCropGrf
 ******************************************************************************/

class SwCropGrf : public SvxGrfCrop
{
public:
    TYPEINFO();
    SwCropGrf();
    SwCropGrf(  sal_Int32 nLeft,    sal_Int32 nRight,
                sal_Int32 nTop,     sal_Int32 nBottom );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*		Clone( SfxItemPool *pPool = 0 ) const;
    virtual USHORT			 	GetVersion( USHORT nFileVersion ) const;
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
    virtual USHORT		 GetVersion( USHORT nFFVer ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVer ) const;
    virtual SvStream&       Store(SvStream &, USHORT nIVer) const;
    virtual int				operator==( const SfxPoolItem& ) const;
    virtual	bool            QueryValue( ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 ) const;
    virtual	bool            PutValue( const ::com::sun::star::uno::Any& rVal,
                                      BYTE nMemberId = 0 );

    void SetUnrotatedSize( const Size& rSz ) 		{ aUnrotatedSize = rSz; }
    const Size& GetUnrotatedSize() const 			{ return aUnrotatedSize; }
};

class SwLuminanceGrf : public SfxInt16Item
{
public:
    SwLuminanceGrf( sal_Int16 nVal = 0 )
        : SfxInt16Item( RES_GRFATR_LUMINANCE, nVal )
    {}

    // pure virtual-Methiden from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual USHORT		 GetVersion( USHORT nFFVer ) const;
};

class SwContrastGrf : public SfxInt16Item
{
public:
    SwContrastGrf( sal_Int16 nVal = 0 )
        : SfxInt16Item( RES_GRFATR_CONTRAST, nVal )
    {}

    // pure virtual-Methiden from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual USHORT		 GetVersion( USHORT nFFVer ) const;
};

class SwChannelGrf : public SfxInt16Item
{
protected:
    SwChannelGrf( sal_Int16 nVal, USHORT nWhich )
        : SfxInt16Item( nWhich, nVal )
    {}

public:
    // pure virtual-Methiden from SfxInt16Item
    virtual USHORT			GetVersion( USHORT nFFVer ) const;
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

class SwGammaGrf : public SfxPoolItem
{
    double nValue;
public:
    TYPEINFO();
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
    virtual SfxPoolItem* 	Clone( SfxItemPool *pPool = 0 ) const;
    virtual USHORT			GetVersion( USHORT nFFVer ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVer ) const;
    virtual SvStream&       Store(SvStream &, USHORT nIVer) const;
    virtual int				operator==( const SfxPoolItem& ) const;

    virtual	bool            QueryValue( ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 ) const;
    virtual	bool            PutValue( const ::com::sun::star::uno::Any& rVal,
                                      BYTE nMemberId = 0 );


    const double& GetValue() const				{ return nValue; }
    void SetValue( const double& rVal )			{ nValue = rVal; }
};

class SwInvertGrf: public SfxBoolItem
{
public:
    SwInvertGrf( sal_Bool bVal = sal_False )
        : SfxBoolItem( RES_GRFATR_INVERT, bVal )
    {}

    // pure virtual-Methiden from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual USHORT		 GetVersion( USHORT nFFVer ) const;
};

class SwTransparencyGrf : public SfxByteItem
{
public:
    SwTransparencyGrf( sal_Int8 nVal = 0 )
        : SfxByteItem( RES_GRFATR_TRANSPARENCY, nVal )
    {}

    // pure virtual-Methiden from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual USHORT		 GetVersion( USHORT nFFVer ) const;
    virtual	bool         QueryValue( ::com::sun::star::uno::Any& rVal,
                                     BYTE nMemberId = 0 ) const;
    virtual	bool         PutValue( const ::com::sun::star::uno::Any& rVal,
                                   BYTE nMemberId = 0 );
};

class SwDrawModeGrf : public SfxEnumItem
{
public:
    SwDrawModeGrf( USHORT nMode = 0 )
        : SfxEnumItem( RES_GRFATR_DRAWMODE, nMode )
    {}

    // pure virtual-Methoden von SfxPoolItem
    virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;

    // pure virtual-Methiden von SfxEnumItem
    virtual USHORT			GetValueCount() const{DBG_BF_ASSERT(0, "STRIP"); return 0;} //STRIP001 	virtual USHORT			GetValueCount() const;
    virtual USHORT			GetVersion( USHORT nFFVer ) const;

    virtual	bool            QueryValue( ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 ) const;
    virtual	bool            PutValue( const ::com::sun::star::uno::Any& rVal,
                                      BYTE nMemberId = 0 );
};



/******************************************************************************
 *	Implementierung der GrafikAttribut Methoden vom SwAttrSet
 ******************************************************************************/

#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SwMirrorGrf &SwAttrSet::GetMirrorGrf(BOOL bInP) const
    { return (const SwMirrorGrf&)Get( RES_GRFATR_MIRRORGRF,bInP); }
#endif
inline const SwCropGrf   &SwAttrSet::GetCropGrf(BOOL bInP) const
    { return (const SwCropGrf&)Get( RES_GRFATR_CROPGRF,bInP); }
inline const SwRotationGrf &SwAttrSet::GetRotationGrf(BOOL bInP) const
    { return (const SwRotationGrf&)Get( RES_GRFATR_ROTATION,bInP); }
inline const SwLuminanceGrf &SwAttrSet::GetLuminanceGrf(BOOL bInP) const
    { return (const SwLuminanceGrf&)Get( RES_GRFATR_LUMINANCE,bInP); }
inline const SwContrastGrf &SwAttrSet::GetContrastGrf(BOOL bInP) const
    { return (const SwContrastGrf&)Get( RES_GRFATR_CONTRAST,bInP); }
inline const SwChannelRGrf &SwAttrSet::GetChannelRGrf(BOOL bInP) const
    { return (const SwChannelRGrf&)Get( RES_GRFATR_CHANNELR,bInP); }
inline const SwChannelGGrf &SwAttrSet::GetChannelGGrf(BOOL bInP) const
    { return (const SwChannelGGrf&)Get( RES_GRFATR_CHANNELG,bInP); }
inline const SwChannelBGrf &SwAttrSet::GetChannelBGrf(BOOL bInP) const
    { return (const SwChannelBGrf&)Get( RES_GRFATR_CHANNELB,bInP); }
inline const SwGammaGrf &SwAttrSet::GetGammaGrf(BOOL bInP) const
    { return (const SwGammaGrf&)Get( RES_GRFATR_GAMMA,bInP); }
inline const SwInvertGrf &SwAttrSet::GetInvertGrf(BOOL bInP) const
    { return (const SwInvertGrf&)Get( RES_GRFATR_INVERT,bInP); }
inline const SwTransparencyGrf &SwAttrSet::GetTransparencyGrf(BOOL bInP) const
    { return (const SwTransparencyGrf&)Get( RES_GRFATR_TRANSPARENCY,bInP); }
inline const SwDrawModeGrf		&SwAttrSet::GetDrawModeGrf(BOOL bInP) const
    { return (const SwDrawModeGrf&)Get( RES_GRFATR_DRAWMODE,bInP); }

/******************************************************************************
 *	Implementierung der GrafikAttribut Methoden vom SwFmt
 ******************************************************************************/

#if ! (defined(MACOSX) && ( __GNUC__ < 3 ) )
// GrP moved to gcc_outl.cxx; revisit with gcc3
inline const SwMirrorGrf &SwFmt::GetMirrorGrf(BOOL bInP) const
    { return aSet.GetMirrorGrf(bInP); }
#endif
inline const SwCropGrf   &SwFmt::GetCropGrf(BOOL bInP) const
    { return aSet.GetCropGrf(bInP); }
inline const SwRotationGrf &SwFmt::GetRotationGrf(BOOL bInP) const
    { return aSet.GetRotationGrf(bInP); }
inline const SwLuminanceGrf &SwFmt::GetLuminanceGrf(BOOL bInP) const
    { return aSet.GetLuminanceGrf( bInP); }
inline const SwContrastGrf &SwFmt::GetContrastGrf(BOOL bInP) const
    { return aSet.GetContrastGrf( bInP); }
inline const SwChannelRGrf &SwFmt::GetChannelRGrf(BOOL bInP) const
    { return aSet.GetChannelRGrf( bInP); }
inline const SwChannelGGrf &SwFmt::GetChannelGGrf(BOOL bInP) const
    { return aSet.GetChannelGGrf( bInP); }
inline const SwChannelBGrf &SwFmt::GetChannelBGrf(BOOL bInP) const
    { return aSet.GetChannelBGrf( bInP); }
inline const SwGammaGrf	&SwFmt::GetGammaGrf(BOOL bInP) const
    { return aSet.GetGammaGrf( bInP); }
inline const SwInvertGrf &SwFmt::GetInvertGrf(BOOL bInP) const
    { return aSet.GetInvertGrf( bInP); }
inline const SwTransparencyGrf &SwFmt::GetTransparencyGrf(BOOL bInP) const
    { return aSet.GetTransparencyGrf( bInP); }
inline const SwDrawModeGrf &SwFmt::GetDrawModeGrf(BOOL bInP) const
    { return aSet.GetDrawModeGrf(bInP); }


} //namespace binfilter
#endif  // _GRFATR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
