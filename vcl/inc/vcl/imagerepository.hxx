/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: imagerepository.hxx,v $
 * $Revision: 1.3 $
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
            const ::rtl::OUString& _rName,
            BitmapEx& _out_rImage,
            bool bSearchLanguageDependent
        );
    };

//........................................................................
} // namespace vcl
//........................................................................

#endif // VCL_IMAGEREPOSITORY_HXX

