/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SFX_STYFITEM_HXX
#define _SFX_STYFITEM_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#ifndef _BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif
#ifndef _IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#include <tools/list.hxx>
#ifndef _RC_HXX //autogen
#include <tools/rc.hxx>
#endif
#include <rsc/rscsfx.hxx>

#ifndef _SFX_STYFITEM_HXX_NOLIST
struct SfxFilterTupel {
    String aName;
    sal_uInt16 nFlags;
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
    sal_uInt16          nFamily;
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

    sal_uInt16              Count() const
                        { return (sal_uInt16)aEntryList.Count(); }

    const SfxStyleFamilyItem* GetObject(sal_uIntPtr nIdx) const
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

