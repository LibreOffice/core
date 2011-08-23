/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
#include <bf_svtools/poolitem.hxx>
#else
#include <sfxipool.hxx>
#endif

namespace binfilter
{

// -----------------------------------------------------------------------

class SfxRequestItem: public SfxSetItem

/**	[Description]

    Represents a function call with optional arguments.
*/

{
public:
                                TYPEINFO();
                                SfxRequestItem();
                                SfxRequestItem( USHORT nWhich, SvStream & );
                                SfxRequestItem( const SfxRequestItem& );
                                ~SfxRequestItem();

    virtual int 			 	operator==( const SfxPoolItem& ) const;
    virtual
    virtual SfxPoolItem*     	Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	 	Create(SvStream &, USHORT nItemVersion) const;
    virtual SvStream&		 	Store(SvStream &, USHORT nItemVersion ) const;
};

}

#endif

