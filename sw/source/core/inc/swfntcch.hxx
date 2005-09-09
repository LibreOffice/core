/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swfntcch.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:59:05 $
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
#ifndef _SWFNTCCH_HXX
#define _SWFNTCCH_HXX

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif

#define NUM_DEFAULT_VALUES 35

#include "swcache.hxx"
#include "swfont.hxx"

class ViewShell;
class SfxPoolItem;

/*************************************************************************
 *                      class SwFontCache
 *************************************************************************/

class SwFontCache : public SwCache
{
public:

    inline SwFontCache() : SwCache(50,50
#ifndef PRODUCT
    , "Globaler AttributSet/Font-Cache pSwFontCache"
#endif
    ) {}

};

// AttributSet/Font-Cache, globale Variable, in FontCache.Cxx angelegt
extern SwFontCache *pSwFontCache;

/*************************************************************************
 *                      class SwFontObj
 *************************************************************************/

class SwFontObj : public SwCacheObj
{
    friend class SwFontAccess;

private:
    SwFont aSwFont;
    const SfxPoolItem* pDefaultArray[ NUM_DEFAULT_VALUES ];

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwFontObj)

    SwFontObj( const void* pOwner, ViewShell *pSh );

    virtual ~SwFontObj();

    inline       SwFont *GetFont()      { return &aSwFont; }
    inline const SwFont *GetFont() const  { return &aSwFont; }
    inline const SfxPoolItem** GetDefault() { return pDefaultArray; }
};

/*************************************************************************
 *                      class SwFontAccess
 *************************************************************************/


class SwFontAccess : public SwCacheAccess
{
    ViewShell *pShell;
protected:
    virtual SwCacheObj *NewObj( );

public:
    SwFontAccess( const void *pOwner, ViewShell *pSh );
    SwFontObj *Get();
};

#endif
