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

#ifndef INCLUDED_SVTOOLS_IMAGERESOURCEACCESS_HXX
#define INCLUDED_SVTOOLS_IMAGERESOURCEACCESS_HXX

#include <svtools/svtdllapi.h>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

class SvStream;

namespace svt
{



    //= GraphicAccess

    /** helper class for obtaining streams (which also can be used with the ImageProducer)
        from a resource
    */
    namespace GraphicAccess
    {
        /** determines whether the given URL denotes an image within a resource
         ( or an image specified by a vnd.sun.star.GraphicObject scheme URL )
        */
        SVT_DLLPUBLIC bool        isSupportedURL( const OUString& _rURL );

        /** for a given URL of an image within a resource ( or an image specified by a vnd.sun.star.GraphicObject scheme URL ), this method retrieves
            an SvStream for this image.

            This method works for arbitrary URLs denoting an image, since the
            css::graphics::GraphicsProvider service is used
            to resolve the URL. However, obtaining the stream is expensive (since
            the image must be copied), so you are strongly encouraged to only use it
            when you know that the image is small enough.
        */
        SVT_DLLPUBLIC SvStream*   getImageStream(
                    const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                    const OUString& _rImageResourceURL
                );

        /** for a given URL of an image within a resource ( or an image specified by a vnd.sun.star.GraphicObject scheme URL ), this method retrieves
            an css::io::XInputStream for this image.
        */
        SVT_DLLPUBLIC css::uno::Reference< css::io::XInputStream >
                getImageXStream(
                    const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
                    const OUString& _rImageResourceURL
                );
    }


} // namespace svt


#endif // DBA14_ INCLUDED_SVTOOLS_IMAGERESOURCEACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
