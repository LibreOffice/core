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

#ifndef INCLUDED_VCL_INC_IMAGE_H
#define INCLUDED_VCL_INC_IMAGE_H

#include <vcl/bitmapex.hxx>

#include <unordered_map>

// - ImplImageList -

struct ImageAryData
{
    OUString maName;
    // Images identified by either name, or by id
    sal_uInt16          mnId;
    BitmapEx        maBitmapEx;

    ImageAryData( const OUString &aName,
                  sal_uInt16 nId, const BitmapEx &aBitmap );
    ImageAryData( const ImageAryData& rData );
    ~ImageAryData();

    bool IsLoadable() { return maBitmapEx.IsEmpty() && !maName.isEmpty(); }
    void Load(const OUString &rPrefix);

    ImageAryData&   operator=( const ImageAryData& rData );
};

struct ImplImageList
{
    typedef std::vector<ImageAryData *> ImageAryDataVec;
    typedef std::unordered_map< OUString, ImageAryData *, OUStringHash >
        ImageAryDataNameHash;

    ImageAryDataVec             maImages;
    ImageAryDataNameHash        maNameHash;
    OUString               maPrefix;
    Size                        maImageSize;
    sal_uIntPtr                       mnRefCount;

    ImplImageList();
    ImplImageList( const ImplImageList &aSrc );
    ~ImplImageList();

    void AddImage( const OUString &aName,
                   sal_uInt16 nId, const BitmapEx &aBitmapEx );
    void RemoveImage( sal_uInt16 nPos );
};

// - ImplImage -

struct ImplImage
{
    sal_uIntPtr mnRefCount;

    BitmapChecksum maBitmapChecksum;

    std::unique_ptr<BitmapEx> mpBitmapEx;
    BitmapEx maDisabledBitmapEx;

    ImplImage();
    ~ImplImage();

private:
    ImplImage(const ImplImage&) = delete;
    void operator=(const ImplImage&) = delete;
};

#endif // INCLUDED_VCL_INC_IMAGE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
