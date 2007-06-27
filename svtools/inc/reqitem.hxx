/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: reqitem.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 21:05:43 $
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
#ifndef _REQITEM_HXX
#define _REQITEM_HXX

#ifndef _SOLAR_H
#include <solar.h>
#endif

#ifndef _RTTI_HXX
#include <rtti.hxx>
#endif

#if _SOLAR__PRIVATE
#include <svtools/poolitem.hxx>
#else
#include <sfxipool.hxx>
#endif

// -----------------------------------------------------------------------

class SfxRequestItem: public SfxSetItem

/** [Description]

    Represents a function call with optional arguments.
*/

{
public:
                                TYPEINFO();
                                SfxRequestItem();
                                SfxRequestItem( USHORT nWhich, SvStream & );
                                SfxRequestItem( const SfxRequestItem& );
                                ~SfxRequestItem();

    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*        Create(SvStream &, USHORT nItemVersion) const;
    virtual SvStream&           Store(SvStream &, USHORT nItemVersion ) const;
};


#endif

