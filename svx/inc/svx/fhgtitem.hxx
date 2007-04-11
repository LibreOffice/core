/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fhgtitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:49:16 $
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
#ifndef _SVX_FHGTITEM_HXX
#define _SVX_FHGTITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxFontHeightItem -----------------------------------------------

// Achtung: Twips-Werte

/*  [Beschreibung]

    Dieses Item beschreibt die Font-Hoehe.
*/

#define FONTHEIGHT_16_VERSION   ((USHORT)0x0001)
#define FONTHEIGHT_UNIT_VERSION ((USHORT)0x0002)

class SVX_DLLPUBLIC SvxFontHeightItem : public SfxPoolItem
{
    UINT32  nHeight;
    USHORT  nProp;              // default 100%
    SfxMapUnit ePropUnit;       // Percent, Twip, ...
public:
    TYPEINFO();

    SvxFontHeightItem( const ULONG nSz = 240, const USHORT nPropHeight = 100,
                       const USHORT nId = ITEMID_FONTHEIGHT );

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
    virtual USHORT           GetVersion( USHORT nItemVersion) const;
    virtual int              ScaleMetrics( long nMult, long nDiv );
    virtual int              HasMetrics() const;

    inline SvxFontHeightItem& operator=(const SvxFontHeightItem& rSize)
        {
            SetHeightValue( rSize.GetHeight() );
            SetProp( rSize.GetProp(), ePropUnit );
            return *this;
        }

    void SetHeight( UINT32 nNewHeight, const USHORT nNewProp = 100,
                     SfxMapUnit eUnit = SFX_MAPUNIT_RELATIVE );

    void SetHeight( UINT32 nNewHeight, USHORT nNewProp,
                     SfxMapUnit eUnit, SfxMapUnit eCoreUnit );

    UINT32 GetHeight() const { return nHeight; }

    void SetHeightValue( UINT32 nNewHeight )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nHeight = nNewHeight;
        }

    void SetProp( const USHORT nNewProp,
                    SfxMapUnit eUnit = SFX_MAPUNIT_RELATIVE )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nProp = nNewProp;
            ePropUnit = eUnit;
        }

    USHORT GetProp() const { return nProp; }

    SfxMapUnit GetPropUnit() const { return ePropUnit;  }   // Percent, Twip, ...
};

#endif

