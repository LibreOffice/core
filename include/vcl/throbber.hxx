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

#ifndef INCLUDED_VCL_THROBBER_HXX
#define INCLUDED_VCL_THROBBER_HXX

#include <vcl/dllapi.h>
#include <vcl/imgctrl.hxx>
#include <vcl/timer.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>

#include <vector>

class VCL_DLLPUBLIC Throbber : public ImageControl
{
public:
    enum class ImageSet
    {
        /// automatically decide between different image sets, depending on what fits best the actual size
        Auto,
        /// default images, 16x16 pixels
        N16px,
        /// default images, 32x32 pixels
        N32px,
        /// default images, 64x64 pixels
        N64px,
    };

public:
    Throbber(vcl::Window* i_parentWindow, WinBits i_style);
    virtual ~Throbber() override;
    virtual void dispose() override;

    // Properties
    void            setStepTime( sal_Int32 nStepTime )  { mnStepTime = nStepTime; }
    sal_Int32       getStepTime() const                 { return mnStepTime; }

    void            setRepeat( bool bRepeat )       { mbRepeat = bRepeat; }
    bool        getRepeat() const                   { return mbRepeat; }

    // animation control
    void start();
    void stop();
    bool isRunning() const;

    void setImageList( ::std::vector< Image > const& i_images );

    // default images
    static ::std::vector< OUString >
        getDefaultImageURLs( const ImageSet i_imageSet );

protected:
    // Window overridables
    virtual void        Resize() override;

private:
    SAL_DLLPRIVATE void initImages();

private:
    ::std::vector< Image >  maImageList;

    bool    mbRepeat;
    sal_Int32   mnStepTime;
    sal_Int32   mnCurStep;
    AutoTimer   maWaitTimer;
    ImageSet    meImageSet;

    DECL_LINK( TimeOutHdl, Timer*, void );
};

#endif // INCLUDED_VCL_THROBBER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
