/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: aeitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:11:41 $
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
#ifndef _AEITEM_HXX
#define _AEITEM_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif

class SfxAllEnumValueArr;
class SvUShorts;

class SVT_DLLPUBLIC SfxAllEnumItem: public SfxEnumItem
{
    SfxAllEnumValueArr*     pValues;
    SvUShorts*              pDisabledValues;

protected:
    USHORT                  _GetPosByValue( USHORT nValue ) const;

public:
    TYPEINFO();
                            SfxAllEnumItem();
                            SfxAllEnumItem( USHORT nWhich);
                            SfxAllEnumItem( USHORT nWhich, USHORT nVal );
                            SfxAllEnumItem( USHORT nWhich, USHORT nVal, const XubString &rText );
                            SfxAllEnumItem( USHORT nWhich, SvStream &rStream );
                            SfxAllEnumItem( const SfxAllEnumItem & );
                            ~SfxAllEnumItem();

    void                    InsertValue( USHORT nValue );
    void                    InsertValue( USHORT nValue, const XubString &rText );
    void                    RemoveValue( USHORT nValue );
    void                    RemoveAllValues();

    USHORT                  GetPosByValue( USHORT nValue ) const;

    virtual USHORT          GetValueCount() const;
    virtual USHORT          GetValueByPos( USHORT nPos ) const;
    virtual XubString       GetValueTextByPos( USHORT nPos ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVersion) const;
    virtual BOOL            IsEnabled( USHORT ) const;
    void                    DisableValue( USHORT );
};

#endif
