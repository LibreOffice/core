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

#ifndef _SDASAITM_HXX
#define _SDASAITM_HXX

#ifndef _SFXPOOLITEM_HXX //autogen
#include <bf_svtools/poolitem.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
namespace binfilter {

class SdrAutoShapeAdjustmentValue
{
    sal_uInt32	nValue;

    friend class SdrAutoShapeAdjustmentItem;

    public :

        void		SetValue( sal_Int32 nVal ) { nValue = nVal; };
        sal_Int32	GetValue() const { return nValue; };

};

class SdrAutoShapeAdjustmentItem : public SfxPoolItem
{
            List	aAdjustmentValueList;

    public :

            TYPEINFO();
            SdrAutoShapeAdjustmentItem();
            SdrAutoShapeAdjustmentItem( SvStream& rIn, sal_uInt16 nVersion );
            ~SdrAutoShapeAdjustmentItem();

            virtual int					operator==( const SfxPoolItem& ) const;
            virtual SfxPoolItem*		Create( SvStream&, sal_uInt16 nItem ) const;
            virtual SvStream&			Store( SvStream&, sal_uInt16 nVersion ) const;
            virtual SfxPoolItem*		Clone( SfxItemPool* pPool = NULL ) const;
            virtual	sal_uInt16			GetVersion( sal_uInt16 nFileFormatVersion ) const;


#ifdef SDR_ISPOOLABLE
            virtual int IsPoolable() const;
#endif

            sal_uInt32							GetCount() const { return aAdjustmentValueList.Count(); };
};

}//end of namespace binfilter
#endif

