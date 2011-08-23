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

#ifndef _SVX_ROTMODIT_HXX
#define _SVX_ROTMODIT_HXX

#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
namespace binfilter {

//----------------------------------------------------------------------------
//	Ausrichtung bei gedrehtem Text

enum SvxRotateMode
{
    SVX_ROTATE_MODE_STANDARD,
    SVX_ROTATE_MODE_TOP,
    SVX_ROTATE_MODE_CENTER,
    SVX_ROTATE_MODE_BOTTOM
};

class SvxRotateModeItem: public SfxEnumItem
{
public:
                TYPEINFO();

                SvxRotateModeItem( SvxRotateMode eMode=SVX_ROTATE_MODE_STANDARD, USHORT nWhich=0);
                SvxRotateModeItem( const SvxRotateModeItem& rItem );
                ~SvxRotateModeItem();

    virtual USHORT				GetValueCount() const;
    virtual SfxPoolItem*		Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*		Create(SvStream &, USHORT) const;
    virtual USHORT				GetVersion( USHORT nFileVersion ) const;
    virtual	sal_Bool        	 QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool			 PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
};

}//end of namespace binfilter
#endif

