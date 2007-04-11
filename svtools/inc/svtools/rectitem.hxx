/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rectitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:32:43 $
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
#ifndef _SFXRECTITEM_HXX
#define _SFXRECTITEM_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

class SvStream;

DBG_NAMEEX_VISIBILITY(SfxRectangleItem, SVT_DLLPUBLIC)

// -----------------------------------------------------------------------

class SVT_DLLPUBLIC SfxRectangleItem: public SfxPoolItem
{
    Rectangle                aVal;

public:
                             TYPEINFO();
                             SfxRectangleItem();
                             SfxRectangleItem( USHORT nWhich, const Rectangle& rVal );
                             SfxRectangleItem( USHORT nWhich, SvStream & );
                             SfxRectangleItem( const SfxRectangleItem& );
                             ~SfxRectangleItem() {
                                 DBG_DTOR(SfxRectangleItem, 0); }

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT nItemVersion) const;
    virtual SvStream&        Store(SvStream &, USHORT nItemVersion) const;

    const Rectangle&         GetValue() const { return aVal; }
            void             SetValue( const Rectangle& rNewVal ) {
                                 DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
                                 aVal = rNewVal;
                             }
    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal,
                                          BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal,
                                          BYTE nMemberId = 0 );
};

#endif

