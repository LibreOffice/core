/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdmetitm.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:36:27 $
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
#ifndef _SDMETITM_HXX
#define _SDMETITM_HXX

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif


//------------------------------------------------------------
// class SdrAngleItem
// Fuer alle Metriken. GetPresentation liefert dann bei
// Value=2500 z.B. ein "25mm".
//------------------------------------------------------------
class SVX_DLLPUBLIC SdrMetricItem: public SfxInt32Item {
public:
    TYPEINFO();
    SdrMetricItem(): SfxInt32Item() {}
    SdrMetricItem(USHORT nId, INT32 nVal=0):  SfxInt32Item(nId,nVal) {}
    SdrMetricItem(USHORT nId, SvStream& rIn): SfxInt32Item(nId,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem* Create(SvStream& rIn, USHORT nVer) const;
    virtual FASTBOOL HasMetrics() const;
    virtual FASTBOOL ScaleMetrics(long nMul, long nDiv);

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres,
                                                SfxMapUnit eCoreMetric,
                                                SfxMapUnit ePresMetric,
                                                String& rText, const IntlWrapper * = 0) const;

#ifdef SDR_ISPOOLABLE
    virtual int IsPoolable() const;
#endif
};


#endif
