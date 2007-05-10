/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fwdtitem.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-10 14:19:57 $
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
#ifndef _SVX_FWDTITEM_HXX
#define _SVX_FWDTITEM_HXX

// include ---------------------------------------------------------------


#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif


// class SvxFontWidthItem -----------------------------------------------


/*  [Beschreibung]

    Dieses Item beschreibt die Font-Breite.
*/

class SvxFontWidthItem : public SfxPoolItem
{
    UINT16  nWidth;         // 0 = default
    USHORT  nProp;          // default 100%
public:
    TYPEINFO();

    SvxFontWidthItem(   const USHORT nSz /*= 0*/,
                        const USHORT nPropWidth /*= 100*/,
                        const USHORT nId  );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT) const;
    virtual SvStream&        Store(SvStream &, USHORT nItemVersion) const;
    virtual int              ScaleMetrics( long nMult, long nDiv );
    virtual int              HasMetrics() const;

    inline SvxFontWidthItem& operator=(const SvxFontWidthItem& rItem )
        {
            SetWidthValue( rItem.GetWidth() );
            SetProp( rItem.GetProp() );
            return *this;
        }

    void SetWidth( UINT16 nNewWidth, const USHORT nNewProp = 100 )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nWidth = UINT16(( (UINT32)nNewWidth * nNewProp ) / 100 );
            nProp = nNewProp;
        }

    UINT16 GetWidth() const { return nWidth; }

    void SetWidthValue( UINT16 nNewWidth )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nWidth = nNewWidth;
        }

    void SetProp( const USHORT nNewProp )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nProp = nNewProp;
        }

    USHORT GetProp() const { return nProp; }
};





#endif


