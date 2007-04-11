/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spltitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:16:47 $
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
#ifndef _SVX_SPLTITEM_HXX
#define _SVX_SPLTITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
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

// class SvxFmtSplitItem -------------------------------------------------

/*
[Beschreibung]
Dieses Item beschreibt eine logische Variable "Absatz trennen ja oder nein".
*/

class SVX_DLLPUBLIC SvxFmtSplitItem : public SfxBoolItem
{
public:
    TYPEINFO();
    ~SvxFmtSplitItem();

    inline SvxFmtSplitItem( const BOOL bSplit = TRUE,
                            const USHORT nWh = ITEMID_FMTSPLIT );
    inline SvxFmtSplitItem& operator=( const SvxFmtSplitItem& rSplit );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream&, USHORT ) const;
    virtual SvStream&       Store( SvStream& , USHORT nItemVersion ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
};

inline SvxFmtSplitItem::SvxFmtSplitItem( const BOOL bSplit, const USHORT nWh ) :
    SfxBoolItem( nWh, bSplit )
{}

inline SvxFmtSplitItem& SvxFmtSplitItem::operator=(
    const SvxFmtSplitItem& rSplit )
{
    SetValue( rSplit.GetValue() );
    return *this;
}

#endif

