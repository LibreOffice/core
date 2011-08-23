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
#ifndef _SVX_GRFCROP_HXX
#define _SVX_GRFCROP_HXX

#include <bf_svtools/poolitem.hxx>
namespace binfilter {

#define GRFCROP_VERSION_SWDEFAULT		0
#define GRFCROP_VERSION_MOVETOSVX 		1

class SvxGrfCrop : public SfxPoolItem
{
    sal_Int32	nLeft, nRight, nTop, nBottom;
public:
//	TYPEINFO();

    SvxGrfCrop( USHORT = ITEMID_GRF_CROP );
    SvxGrfCrop( sal_Int32 nLeft,	sal_Int32 nRight,
                sal_Int32 nTop,		sal_Int32 nBottom,
                USHORT = ITEMID_GRF_CROP );
    virtual ~SvxGrfCrop();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int            		operator==( const SfxPoolItem& ) const;
// must be implement by the specific items
//	virtual SfxPoolItem*		Clone( SfxItemPool *pPool = 0 ) const;
//	virtual USHORT			 	GetVersion( USHORT nFileVersion ) const;
    virtual SfxPoolItem*		Create(SvStream &, USHORT nVer) const;
    virtual SvStream&			Store(SvStream &, USHORT nIVer) const;
    virtual	bool                QueryValue( ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 ) const;
    virtual	bool                PutValue( const ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 );

    void SetLeft( sal_Int32 nVal )		{ nLeft = nVal; }
    void SetRight( sal_Int32 nVal )		{ nRight = nVal; }
    void SetTop( sal_Int32 nVal )		{ nTop = nVal; }
    void SetBottom( sal_Int32 nVal )	{ nBottom = nVal; }

    sal_Int32 GetLeft() const			{ return nLeft; }
    sal_Int32 GetRight() const 			{ return nRight; }
    sal_Int32 GetTop() const			{ return nTop; }
    sal_Int32 GetBottom() const			{ return nBottom; }

    inline SvxGrfCrop& operator=( const SvxGrfCrop& rCrop )
        {
            nLeft = rCrop.GetLeft(); 		nTop = rCrop.GetTop();
            nRight = rCrop.GetRight();		nBottom = rCrop.GetBottom();
            return *this;
        }
};

}//end of namespace binfilter
#endif  // _GRFATR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
