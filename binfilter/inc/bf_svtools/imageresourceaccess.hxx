/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SVTOOLS_INC_IMAGERESOURCEACCESS_HXX
#define SVTOOLS_INC_IMAGERESOURCEACCESS_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "bf_svtools/svtdllapi.h"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
/** === end UNO includes === **/

class SvStream;
//........................................................................
namespace binfilter
{
//........................................................................

    //====================================================================
    //= ImageResourceAccess
    //====================================================================
    /** helper class for obtaining streams (which also can be used with the ImageProducer)
        from a resource
    */
    class ImageResourceAccess
    {
    private:
        ImageResourceAccess();    // never implemented

    public:
        /** determines whether the given URL denotes an image within a resource
        */
         static  bool        isImageResourceURL( const ::rtl::OUString& _rURL );

        /** for a given URL of an image within a resource, this method retrieves
            an SvStream for this image.

            This method works for arbitrary URLs denoting an image, since the
            <type scope="com::sun::star::graphics">GraphicsProvider</type> service is used
            to resolve the URL. However, obtaining the stream is expensive (since
            the image must be copied), so you are strongly encouraged to only use it
            when you know that the image is small enough.
        */
         static  SvStream*   getImageStream(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
                    const ::rtl::OUString& _rImageResourceURL
                );
    };

//........................................................................
}
//........................................................................

#endif // DBA14_SVTOOLS_INC_IMAGERESOURCEACCESS_HXX

