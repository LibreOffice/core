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
#ifndef _FMTSRND_HXX
#define _FMTSRND_HXX

#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
class IntlWrapper; 
namespace binfilter {



//SwFmtSurround, wie soll sich der ---------------
//	Dokumentinhalt unter dem Rahmen verhalten ---

enum SwSurround {
    SURROUND_BEGIN,
    SURROUND_NONE = SURROUND_BEGIN,
    SURROUND_THROUGHT,
    SURROUND_PARALLEL,
    SURROUND_IDEAL,
    SURROUND_LEFT,
    SURROUND_RIGHT,
    SURROUND_END
};

class SwFmtSurround: public SfxEnumItem
{
    BOOL	bAnchorOnly :1;
    BOOL	bContour 	:1;
    BOOL	bOutside 	:1;
public:
    SwFmtSurround( SwSurround eNew = SURROUND_PARALLEL );
    SwFmtSurround( const SwFmtSurround & );
    inline SwFmtSurround &operator=( const SwFmtSurround &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;
    virtual USHORT			GetValueCount() const{DBG_BF_ASSERT(0, "STRIP"); return 0;} //STRIP001 virtual USHORT			GetValueCount() const;
    virtual USHORT			GetVersion( USHORT nFFVer ) const;
    virtual	BOOL        	 QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	BOOL			 PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


    SwSurround GetSurround()const { return SwSurround( GetValue() ); }
    BOOL    IsAnchorOnly()	const { return bAnchorOnly; }
    BOOL    IsContour()		const { return bContour; }
    BOOL    IsOutside()		const { return bOutside; }
    void	SetSurround  ( SwSurround eNew ){ SfxEnumItem::SetValue( USHORT( eNew ) ); }
    void	SetAnchorOnly( BOOL bNew )		{ bAnchorOnly = bNew; }
    void	SetContour( BOOL bNew )			{ bContour = bNew; }
    void	SetOutside( BOOL bNew )			{ bOutside = bNew; }
};

inline SwFmtSurround &SwFmtSurround::operator=( const SwFmtSurround &rCpy )
{
    bAnchorOnly = rCpy.IsAnchorOnly();
    bContour = rCpy.IsContour();
    bOutside = rCpy.IsOutside();
    SfxEnumItem::SetValue( rCpy.GetValue() );
    return *this;
}

#if !(defined(MACOSX) && ( __GNUC__ < 3 ))
// GrP moved to gcc_outl.hxx; revisit with gcc3
inline const SwFmtSurround &SwAttrSet::GetSurround(BOOL bInP) const
    { return (const SwFmtSurround&)Get( RES_SURROUND,bInP); }

inline const SwFmtSurround &SwFmt::GetSurround(BOOL bInP) const
    { return aSet.GetSurround(bInP); }
#endif

} //namespace binfilter
#endif

