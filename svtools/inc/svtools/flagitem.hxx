/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: flagitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:18:29 $
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
#ifndef _SFXFLAGITEM_HXX
#define _SFXFLAGITEM_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

class SvStream;

extern USHORT nSfxFlagVal[16];

// -----------------------------------------------------------------------

DBG_NAMEEX_VISIBILITY(SfxFlagItem, SVT_DLLPUBLIC)

class SVT_DLLPUBLIC SfxFlagItem: public SfxPoolItem
{
    USHORT                   nVal;

public:
                             TYPEINFO();

                             SfxFlagItem( USHORT nWhich = 0, USHORT nValue = 0 );
                             SfxFlagItem( USHORT nWhich, SvStream & );
                             SfxFlagItem( const SfxFlagItem& );

                             ~SfxFlagItem() {
                                DBG_DTOR(SfxFlagItem, 0); }

    virtual BYTE             GetFlagCount() const;
    virtual XubString        GetFlagText( BYTE nFlag ) const;

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT nVersion) const;
    virtual SvStream&        Store(SvStream &, USHORT nItemVersion) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;
            USHORT           GetValue() const { return nVal; }
            void             SetValue( USHORT nNewVal ) {
                                 DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
                                 nVal = nNewVal;
                             }
            int              GetFlag( BYTE nFlag ) const {
                                 return ( (nVal & nSfxFlagVal[nFlag]) != 0 ); }
            void             SetFlag( BYTE nFlag, int bVal );
};

#endif
