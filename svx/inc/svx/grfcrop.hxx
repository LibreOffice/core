/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: grfcrop.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:56:04 $
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
#ifndef _SVX_GRFCROP_HXX
#define _SVX_GRFCROP_HXX

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif


#define GRFCROP_VERSION_SWDEFAULT       0
#define GRFCROP_VERSION_MOVETOSVX       1

class SVX_DLLPUBLIC SvxGrfCrop : public SfxPoolItem
{
    sal_Int32   nLeft, nRight, nTop, nBottom;
public:
    SvxGrfCrop( USHORT = ITEMID_GRF_CROP );
    SvxGrfCrop( sal_Int32 nLeft,    sal_Int32 nRight,
                sal_Int32 nTop,     sal_Int32 nBottom,
                USHORT = ITEMID_GRF_CROP );
    virtual ~SvxGrfCrop();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*        Create(SvStream &, USHORT nVer) const;
    virtual SvStream&           Store(SvStream &, USHORT nIVer) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;
    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 );

    void SetLeft( sal_Int32 nVal )      { nLeft = nVal; }
    void SetRight( sal_Int32 nVal )     { nRight = nVal; }
    void SetTop( sal_Int32 nVal )       { nTop = nVal; }
    void SetBottom( sal_Int32 nVal )    { nBottom = nVal; }

    sal_Int32 GetLeft() const           { return nLeft; }
    sal_Int32 GetRight() const          { return nRight; }
    sal_Int32 GetTop() const            { return nTop; }
    sal_Int32 GetBottom() const         { return nBottom; }

    inline SvxGrfCrop& operator=( const SvxGrfCrop& rCrop )
        {
            nLeft = rCrop.GetLeft();        nTop = rCrop.GetTop();
            nRight = rCrop.GetRight();      nBottom = rCrop.GetBottom();
            return *this;
        }
};


#endif  // _GRFATR_HXX
