/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdasaitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:08:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SDASAITM_HXX
#define _SDASAITM_HXX

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

class SdrCustomShapeAdjustmentValue
{
    sal_uInt32  nValue;

    friend class SdrCustomShapeAdjustmentItem;

    public :

        void        SetValue( sal_Int32 nVal ) { nValue = nVal; };
        sal_Int32   GetValue() const { return nValue; };

};

class SdrCustomShapeAdjustmentItem : public SfxPoolItem
{
            List    aAdjustmentValueList;

    public :

            TYPEINFO();
            SdrCustomShapeAdjustmentItem();
            SdrCustomShapeAdjustmentItem( SvStream& rIn, sal_uInt16 nVersion );
            ~SdrCustomShapeAdjustmentItem();

            virtual int                 operator==( const SfxPoolItem& ) const;
            virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePresentation,
                                            SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric,
                                                String &rText, const IntlWrapper * = 0) const;
            virtual SfxPoolItem*        Create( SvStream&, sal_uInt16 nItem ) const;
            virtual SvStream&           Store( SvStream&, sal_uInt16 nVersion ) const;
            virtual SfxPoolItem*        Clone( SfxItemPool* pPool = NULL ) const;
            virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;

            virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
            virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


#ifdef SDR_ISPOOLABLE
            virtual int IsPoolable() const;
#endif

            sal_uInt32                          GetCount() const { return aAdjustmentValueList.Count(); };
            const SdrCustomShapeAdjustmentValue&    GetValue( sal_uInt32 nIndex ) const;
            void                                SetValue( sal_uInt32 nIndex,
                                                        const SdrCustomShapeAdjustmentValue& rVal );
};

#endif

