/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xflhtit.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:52:56 $
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

#ifndef _SVX_XFLHTIT_HXX
#define _SVX_XFLHTIT_HXX

#ifndef _SVX_XIT_HXX //autogen
#include <svx/xit.hxx>
#endif

#ifndef _SVX_XHATCH_HXX //autogen
#include <svx/xhatch.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SdrModel;

//---------------------
// class XFillHatchItem
//---------------------
class SVX_DLLPUBLIC XFillHatchItem : public NameOrIndex
{
    XHatch  aHatch;

public:
                            TYPEINFO();
                            XFillHatchItem() : NameOrIndex(XATTR_FILLHATCH, -1) {}
                            XFillHatchItem(long nIndex, const XHatch& rTheHatch);
                            XFillHatchItem(const String& rName, const XHatch& rTheHatch);
                            XFillHatchItem(SfxItemPool* pPool, const XHatch& rTheHatch);
                            XFillHatchItem(SfxItemPool* pPool );
                            XFillHatchItem(const XFillHatchItem& rItem);
                            XFillHatchItem(SvStream& rIn);

    virtual int             operator==(const SfxPoolItem& rItem) const;
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, USHORT nVer) const;
    virtual SvStream&       Store(SvStream& rOut, USHORT nItemVersion ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
    virtual FASTBOOL        HasMetrics() const;
    virtual FASTBOOL        ScaleMetrics(long nMul, long nDiv);

    const XHatch&           GetHatchValue(const XHatchTable* pTable = 0) const; // GetValue -> GetHatchValue
    void                    SetHatchValue(const XHatch& rNew)  { aHatch = rNew; Detach(); } // SetValue -> SetHatchValue

    static BOOL CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 );
    XFillHatchItem* checkForUniqueItem( SdrModel* pModel ) const;
};

#endif

