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

#ifndef FRAMEWORK_SOURCE_UICONFIGURATION_IMAGELIST_HXX
#define FRAMEWORK_SOURCE_UICONFIGURATION_IMAGELIST_HXX

class ImageList
{
public:
                    explicit ImageList();
                    ImageList( const std::vector<OUString>& rNameVector,
                               const OUString& rPrefix);

    void            InsertFromHorizontalStrip( const BitmapEx &rBitmapEx,
                                   const std::vector< OUString > &rNameVector );
    BitmapEx        GetAsHorizontalStrip() const;
    sal_uInt16      GetImageCount() const;

    void            AddImage( const OUString& rImageName, const Image& rImage );

    void            ReplaceImage( const OUString& rImageName, const Image& rImage );

    void            RemoveImage( sal_uInt16 nId );

    Image           GetImage( sal_uInt16 nId ) const;
    Image           GetImage( const OUString& rImageName ) const;

    sal_uInt16      GetImagePos( sal_uInt16 nId ) const;
    sal_uInt16      GetImagePos( const OUString& rImageName ) const;

    sal_uInt16      GetImageId( sal_uInt16 nPos ) const;

    OUString        GetImageName( sal_uInt16 nPos ) const;
    void            GetImageNames( std::vector< OUString >& rNames ) const;

    bool            operator==( const ImageList& rImageList ) const;
    bool            operator!=( const ImageList& rImageList ) const { return !(ImageList::operator==( rImageList )); }

private:

    std::shared_ptr<ImplImageList> mpImplData;

    void    ImplInit( sal_uInt16 nItems, const Size &rSize );
    sal_uInt16  ImplGetImageId( const OUString& rImageName ) const;
};

#endif // INCLUDED_VCL_IMAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
