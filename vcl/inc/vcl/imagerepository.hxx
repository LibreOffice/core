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

#ifndef VCL_IMAGEREPOSITORY_HXX
#define VCL_IMAGEREPOSITORY_HXX

#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>

class BitmapEx;

//........................................................................
namespace vcl
{
//........................................................................

    //====================================================================
    //= ImageRepository
    //====================================================================
    // provides access to the application's image repository (image.zip)
    class VCL_DLLPUBLIC ImageRepository
    {
    public:
        /** loads an image from the application's image repository
            @param  _rName
                the name of the image to load.
            @param  _out_rImage
                will take the image upon successful return.
            @param  bSearchLanguageDependent
                determines whether a language-dependent image is to be searched.
            @return
                whether or not the image could be loaded successfully.
        */
        static bool loadImage(
            const OUString& _rName,
            BitmapEx& _out_rImage,
            bool bSearchLanguageDependent,
            bool loadMissing = false
        );

        static bool loadDefaultImage(
            BitmapEx& _out_rImage
        );
    };

//........................................................................
} // namespace vcl
//........................................................................

#endif // VCL_IMAGEREPOSITORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
