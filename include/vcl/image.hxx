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

#ifndef INCLUDED_VCL_IMAGE_HXX
#define INCLUDED_VCL_IMAGE_HXX

#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <tools/solar.h>
#include <vcl/bitmapex.hxx>
#include <vcl/outdev.hxx>

#include <com/sun/star/uno/Reference.hxx>

#include <memory>
#include <vector>

struct ImplImage;
struct ImplImageList;
namespace com { namespace sun { namespace star { namespace graphic { class XGraphic;} } } }

namespace vcl
{
    enum class ImageType
    {
        Size16,
        Size26,
        Size32,
        Small = Size16,
        LAST = Size32,
    };
}

#define IMAGELIST_IMAGE_NOTFOUND    ((sal_uInt16)0xFFFF)

class SAL_WARN_UNUSED VCL_DLLPUBLIC Image
{
    friend class ::OutputDevice;

public:
                    Image();
                    explicit Image( const BitmapEx& rBitmapEx );
                    explicit Image( const css::uno::Reference< css::graphic::XGraphic >& rxGraphic );
                    explicit Image( const OUString &rPNGFileUrl );

    Size            GetSizePixel() const;

    BitmapEx        GetBitmapEx() const;

    bool            operator!() const { return !mpImplData; }
    bool            operator==( const Image& rImage ) const;
    bool            operator!=( const Image& rImage ) const { return !(Image::operator==( rImage )); }

    void Draw(OutputDevice* pOutDev, const Point& rPos, DrawImageFlags nStyle, const Size* pSize = nullptr);

private:

    std::shared_ptr<ImplImage> mpImplData;

    SAL_DLLPRIVATE void    ImplInit( const BitmapEx& rBmpEx );
};

#endif // INCLUDED_VCL_IMAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
