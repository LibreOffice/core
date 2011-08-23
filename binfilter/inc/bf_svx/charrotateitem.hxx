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
#ifndef _SVX_CHARROTATEITEM_HXX
#define _SVX_CHARROTATEITEM_HXX

// include ---------------------------------------------------------------

#include <bf_svtools/intitem.hxx>
#include <bf_svx/svxids.hrc>
namespace binfilter {

// class SvxCharRotateItem ----------------------------------------------

/* [Description]

    This item defines a character rotation value (0,1 degree). Currently
    character can only be rotated 90,0 and 270,0 degrees.
    The flag FitToLine defines only a UI-Information -
    if true it must also create a SvxCharScaleItem.

*/

class SvxCharRotateItem : public SfxUInt16Item
{
    sal_Bool bFitToLine;
public:
    TYPEINFO();

    SvxCharRotateItem( sal_uInt16 nValue = 0,
                       sal_Bool bFitIntoLine = sal_False,
                       const sal_uInt16 nId = ITEMID_CHARROTATE );

/*NBFF*/ 	virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;
/*NBFF*/ 	virtual SfxPoolItem*	Create(SvStream &, USHORT) const;
/*NBFF*/ 	virtual SvStream& 		Store(SvStream & rStrm, USHORT nIVer) const;
    virtual USHORT			GetVersion( USHORT nFileVersion ) const;

/*NBFF*/ 	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
/*NBFF*/ 									SfxMapUnit eCoreMetric,
/*NBFF*/ 									SfxMapUnit ePresMetric,
/*NBFF*/ 									String &rText,
/*NBFF*/                                     const ::IntlWrapper * = 0 ) const;

/*NBFF*/ 	virtual bool PutValue( const ::com::sun::star::uno::Any& rVal,
/*NBFF*/ 									BYTE nMemberId );
/*NBFF*/ 	virtual bool QueryValue( ::com::sun::star::uno::Any& rVal,
/*NBFF*/ 								BYTE nMemberId ) const;

/*NBFF*/ 	inline SvxCharRotateItem& operator=( const SvxCharRotateItem& rItem )
/*NBFF*/ 	{
/*NBFF*/ 		SetValue( rItem.GetValue() );
/*NBFF*/ 		SetFitToLine( rItem.IsFitToLine() );
/*NBFF*/ 		return *this;
/*NBFF*/ 	}

/*NBFF*/ 	virtual int 			 operator==( const SfxPoolItem& ) const;

    // our currently only degree values
/*NBFF*/ 	void SetTopToBotton() 					{ SetValue( 2700 ); }
/*NBFF*/ 	void SetBottomToTop() 					{ SetValue(  900 ); }
/*NBFF*/ 	sal_Bool IsTopToBotton() const			{ return 2700 == GetValue(); }
/*NBFF*/ 	sal_Bool IsBottomToTop() const			{ return  900 == GetValue(); }
/*NBFF*/ 
/*NBFF*/ 	sal_Bool IsFitToLine() const 			{ return bFitToLine; }
/*NBFF*/ 	void SetFitToLine( sal_Bool b )			{ bFitToLine = b; }
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
