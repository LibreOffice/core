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
#ifndef _SVX_TWOLINESITEM_HXX
#define _SVX_TWOLINESITEM_HXX

#include <sal/types.h>
#include <bf_svtools/poolitem.hxx>
#include <bf_svx/svxids.hrc>

namespace rtl
{
    class OUString;
}
namespace binfilter {
class SvXMLUnitConverter;
class SvxTwoLinesItem : public SfxPoolItem
{
    sal_Unicode	cStartBracket, cEndBracket;
    sal_Bool bOn;
public:
    TYPEINFO();
    SvxTwoLinesItem( sal_Bool bOn = TRUE,
                     sal_Unicode nStartBracket = 0,
                     sal_Unicode nEndBracket = 0,
                     sal_uInt16 nId = ITEMID_TWOLINES );
/*NBFF*/ 	SvxTwoLinesItem( const SvxTwoLinesItem& rAttr );
    virtual ~SvxTwoLinesItem();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
/*NBFF*/ 	virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
/*NBFF*/ 	virtual SvStream&		Store(SvStream &, USHORT nIVer) const;
/*NBFF*/ 	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
/*NBFF*/ 									SfxMapUnit eCoreMetric,
/*NBFF*/ 									SfxMapUnit ePresMetric,
/*NBFF*/ 									String &rText,
/*NBFF*/                                     const ::IntlWrapper* pIntl = 0 ) const;

/*NBFF*/ 	virtual	bool          QueryValue( ::com::sun::star::uno::Any& rVal,
/*NBFF*/ 										BYTE nMemberId = 0 ) const;
/*NBFF*/ 	virtual	bool          PutValue( const ::com::sun::star::uno::Any& rVal,
/*NBFF*/ 										BYTE nMemberId = 0 );

    virtual USHORT			GetVersion( USHORT nFFVer ) const;

    SvxTwoLinesItem& 		operator=( const SvxTwoLinesItem& rCpy )
    {
        SetValue( rCpy.GetValue() );
        SetStartBracket( rCpy.GetStartBracket() );
        SetEndBracket( rCpy.GetEndBracket() );
        return *this;
    }

    sal_Bool GetValue() const 					{ return bOn; }
    void SetValue( sal_Bool bFlag ) 			{ bOn = bFlag; }

    sal_Unicode GetStartBracket() const 		{ return cStartBracket; }
    void SetStartBracket( sal_Unicode c ) 		{ cStartBracket = c; }

    sal_Unicode GetEndBracket() const 			{ return cEndBracket; }
    void SetEndBracket( sal_Unicode c ) 		{ cEndBracket = c; }
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
