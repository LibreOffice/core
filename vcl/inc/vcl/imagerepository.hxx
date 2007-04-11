/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imagerepository.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:56:48 $
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

#ifndef VCL_IMAGEREPOSITORY_HXX
#define VCL_IMAGEREPOSITORY_HXX

#include <vcl/dllapi.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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

