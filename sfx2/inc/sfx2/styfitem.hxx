/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: styfitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:30:16 $
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
#ifndef _SFX_STYFITEM_HXX
#define _SFX_STYFITEM_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif
#ifndef _IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _RC_HXX //autogen
#include <tools/rc.hxx>
#endif
#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif

#ifndef _SFX_STYFITEM_HXX_NOLIST
struct SfxFilterTupel {
    String aName;
    USHORT nFlags;
};

DECLARE_LIST(SfxStyleFilter, SfxFilterTupel*)
#else
typedef List SfxStyleFilter;
#endif

// CLASS -----------------------------------------------------------------

class SfxStyleFamilyItem: public Resource
{
    Image           aImage;
    Bitmap          aBitmap;
    String          aText;
    String          aHelpText;
    USHORT          nFamily;
    SfxStyleFilter  aFilterList;

public:
                    SfxStyleFamilyItem( const ResId &rId );
                    ~SfxStyleFamilyItem();

    const Bitmap&   GetBitmap() const { return aBitmap; }
    const String&   GetText() const { return aText; }
    const String&   GetHelpText() const { return aHelpText; }
    SfxStyleFamily  GetFamily() const { return (SfxStyleFamily)nFamily; }
    const SfxStyleFilter& GetFilterList() const { return aFilterList; }
    const Image&    GetImage() const { return aImage; }

    // --------------------------------------------------------------------
    class GrantAccess { friend class SfxStyleFamilies; };
    void            SetImage( const Image& _rImg ) { aImage = _rImg; }
};

//#if 0 // _SOLAR__PRIVATE
DECLARE_LIST(SfxStyleFamilyList, SfxStyleFamilyItem*)
//#else
//typedef List SfxStyleFamilyList;
//#endif

class SFX2_DLLPUBLIC SfxStyleFamilies: public Resource
{
    SfxStyleFamilyList  aEntryList;

public:
    /** ctor
        <p>Will automatically call updateImages with BMP_COLOR_NORMAL.</p>
    */
                        SfxStyleFamilies( const ResId &);
                        SfxStyleFamilies( ) {};
                        ~SfxStyleFamilies();

    USHORT              Count() const
                        { return (USHORT)aEntryList.Count(); }

    const SfxStyleFamilyItem* GetObject(ULONG nIdx) const
                        { return (SfxStyleFamilyItem*)aEntryList.GetObject(nIdx); }

    /** updates the images of all single SfxStyleFamilyItems with new images from the given resource

        <p>The resource must contain a local image lists, with the id beeing the integer equivalent for the
        requested bitmap mode, incremented by 1.</p>

        <p>Usually, you will use the same resource which originally constructed the object.</p>

        @return
            <TRUE/> if an image list for the requested mode could be found in the given resource.
    */
    sal_Bool    updateImages( const ResId& _rId, const BmpColorMode _eMode );
};

#endif

