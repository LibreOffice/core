/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxfiitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:34:14 $
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
#ifndef _SFXIITM_HXX
#define _SFXIITM_HXX

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef _FRACT_HXX //autogen
#include <tools/fract.hxx>
#endif

/*************************************************************************/
/* FractionItem                                                          */
/*************************************************************************/

class SdrFractionItem: public SfxPoolItem {
    Fraction nValue;
public:
    TYPEINFO();
    SdrFractionItem(USHORT nId=0): SfxPoolItem(nId) {}
    SdrFractionItem(USHORT nId, const Fraction& rVal): SfxPoolItem(nId), nValue(rVal) {}
    SdrFractionItem(USHORT nId, SvStream& rIn);
    virtual int              operator==(const SfxPoolItem&) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePresentation, SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric, String &rText, const IntlWrapper * = 0) const;
    virtual SfxPoolItem*     Create(SvStream&, USHORT nVer) const;
#if SUPD <= 345
    virtual SvStream&        Store(SvStream&) const;
#else
    virtual SvStream&        Store(SvStream&, USHORT nItemVers) const;
#endif
    virtual SfxPoolItem*     Clone(SfxItemPool *pPool=NULL) const;
            const Fraction&  GetValue() const { return nValue; }
            void             SetValue(const Fraction& rVal) { nValue = rVal; }
#ifdef SDR_ISPOOLABLE
    virtual int IsPoolable() const;
#endif
};


#endif
