/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxekitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:33:38 $
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
#ifndef _SXEKITM_HXX
#define _SXEKITM_HXX

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

enum SdrEdgeKind {SDREDGE_ORTHOLINES,
                  SDREDGE_THREELINES,
                  SDREDGE_ONELINE,
                  SDREDGE_BEZIER,
                  SDREDGE_ARC};

//------------------------------
// class SdrEdgeKindItem
//------------------------------
class SVX_DLLPUBLIC SdrEdgeKindItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrEdgeKindItem(SdrEdgeKind eStyle=SDREDGE_ORTHOLINES): SfxEnumItem(SDRATTR_EDGEKIND,sal::static_int_cast< USHORT >(eStyle)) {}
    SdrEdgeKindItem(SvStream& rIn)                        : SfxEnumItem(SDRATTR_EDGEKIND,rIn)    {}
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem* Create(SvStream& rIn, USHORT nVer) const;
    virtual USHORT       GetValueCount() const; // { return 5; }
            SdrEdgeKind  GetValue() const      { return (SdrEdgeKind)SfxEnumItem::GetValue(); }
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
    virtual String  GetValueTextByPos(USHORT nPos) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};

#endif
