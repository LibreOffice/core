/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _SFX_STYFITEM_HXX
#define _SFX_STYFITEM_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <vcl/bitmap.hxx>
#include <vcl/image.hxx>
#include <tools/rc.hxx>
#include <tools/string.hxx>
#include <rsc/rscsfx.hxx>
#include <vector>

struct SfxFilterTupel {
    String aName;
    sal_uInt16 nFlags;
};
typedef ::std::vector< SfxFilterTupel* > SfxStyleFilter;

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

typedef ::std::vector< SfxStyleFamilyItem* > SfxStyleFamilyList;

class SFX2_DLLPUBLIC SfxStyleFamilies: public Resource
{
    SfxStyleFamilyList  aEntryList;

public:
                        SfxStyleFamilies( const ResId &);
                        SfxStyleFamilies( ) {};
                        ~SfxStyleFamilies();

    size_t              size() const
                        { return aEntryList.size(); }

    const SfxStyleFamilyItem* at(size_t nIdx) const
                        { return (SfxStyleFamilyItem*)(aEntryList.empty() ? NULL : aEntryList[nIdx]); }

    /** updates the images of all single SfxStyleFamilyItems with new images from the given resource

        <p>The resource must contain a local image lists, with the id beeing the integer equivalent for the
        requested bitmap mode, incremented by 1.</p>

        <p>Usually, you will use the same resource which originally constructed the object.</p>

        @return
            <TRUE/> if an image list for the requested mode could be found in the given resource.
    */
    sal_Bool    updateImages( const ResId& _rId );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
