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

#ifndef VCL_THROBBER_HXX
#define VCL_THROBBER_HXX

#include "vcl/dllapi.h"
#include "vcl/imgctrl.hxx"
#include "vcl/timer.hxx"

#include <com/sun/star/graphic/XGraphic.hpp>

#include <vector>

class VCL_DLLPUBLIC Throbber : public ImageControl
{
public:
    enum ImageSet
    {
        /// no (default) images at all
        IMAGES_NONE,
        /// automatically decide between different image sets, depending on what fits best the actual size
        IMAGES_AUTO,
        /// default images, 16x16 pixels
        IMAGES_16_PX,
        /// default images, 32x32 pixels
        IMAGES_32_PX,
        /// default images, 64x64 pixels
        IMAGES_64_PX,
    };

public:
                    Throbber( Window* i_parentWindow, WinBits i_style, const ImageSet i_imageSet = IMAGES_AUTO );
                    Throbber( Window* i_parentWindow, const ResId& i_resId, const ImageSet i_imageSet = IMAGES_AUTO );
                    ~Throbber();

    // Properties
    void            setStepTime( sal_Int32 nStepTime )  { mnStepTime = nStepTime; }
    sal_Int32       getStepTime() const                 { return mnStepTime; }

    void            setRepeat( sal_Bool bRepeat )       { mbRepeat = bRepeat; }
    sal_Bool        getRepeat() const                   { return mbRepeat; }

    // animation control
    void start();
    void stop();
    bool isRunning() const;

    void setImageList( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > >& ImageList );
    void setImageList( ::std::vector< Image > const& i_images );

    // default images
    static ::std::vector< ::rtl::OUString >
        getDefaultImageURLs( const ImageSet i_imageSet );

protected:
    // Window overridables
    virtual void        Resize();

private:
    SAL_DLLPRIVATE void initImages();

private:
    ::std::vector< Image >  maImageList;

    sal_Bool    mbRepeat;
    sal_Int32   mnStepTime;
    sal_Int32   mnCurStep;
    sal_Int32   mnStepCount;
    AutoTimer   maWaitTimer;
    ImageSet    meImageSet;

    DECL_LINK( TimeOutHdl, void* );
};

#endif // VCL_THROBBER_HXX

