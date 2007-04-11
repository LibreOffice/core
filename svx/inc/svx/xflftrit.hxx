/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xflftrit.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:52:35 $
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

#ifndef _SVX_XFLFTRIT_HXX
#define _SVX_XFLFTRIT_HXX

#ifndef _SVX_XFLGRIT_HXX //autogen
#include <svx/xflgrit.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//-----------------------------------
// class XFillFloatTransparenceItem -
//-----------------------------------

class SVX_DLLPUBLIC XFillFloatTransparenceItem : public XFillGradientItem
{
private:

    long                    nDummy1;
    long                    nDummy2;
    BOOL                    bEnabled;

public:
                            TYPEINFO();

                            XFillFloatTransparenceItem();
                            XFillFloatTransparenceItem( long nIndex, const XGradient& rGradient, BOOL bEnable = TRUE );
                            XFillFloatTransparenceItem(const String& rName, const XGradient& rGradient, BOOL bEnable = TRUE );
                            XFillFloatTransparenceItem(SfxItemPool* pPool, const XGradient& rTheGradient, BOOL bEnable = TRUE );
                            XFillFloatTransparenceItem(SfxItemPool* pPool );
                            XFillFloatTransparenceItem( const XFillFloatTransparenceItem& rItem );

    virtual int             operator==( const SfxPoolItem& rItem ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual USHORT          GetVersion( USHORT nFileFormatVersion ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres, SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric, String &rText, const IntlWrapper * pIntlWrapper = 0 ) const;

    BOOL                    IsEnabled() const { return bEnabled; }
    void                    SetEnabled( BOOL bEnable ) { bEnabled = bEnable; }

    static BOOL CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 );
    XFillFloatTransparenceItem* checkForUniqueItem( SdrModel* pModel ) const;
};

#endif
