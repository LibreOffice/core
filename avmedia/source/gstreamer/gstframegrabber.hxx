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

#ifndef INCLUDED_AVMEDIA_SOURCE_GSTREAMER_GSTFRAMEGRABBER_HXX
#define INCLUDED_AVMEDIA_SOURCE_GSTREAMER_GSTFRAMEGRABBER_HXX

#include "gstplayer.hxx"
#include <com/sun/star/media/XFrameGrabber.hpp>
#include <cppuhelper/implbase.hxx>
#include <boost/noncopyable.hpp>

namespace avmedia { namespace gstreamer {


// - FrameGrabber -


typedef ::cppu::WeakImplHelper< css::media::XFrameGrabber,
                                css::lang::XServiceInfo > FrameGrabber_BASE;

class FrameGrabber : public FrameGrabber_BASE, private boost::noncopyable
{
    GstElement *mpPipeline;
    void disposePipeline();
public:
    // static create method instead of public Ctor
    static FrameGrabber* create( const OUString &rURL );

    virtual ~FrameGrabber();

    // XFrameGrabber
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL grabFrame( double fMediaTime ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

private:
    explicit FrameGrabber( const OUString &aURL );
};

} // namespace gst
} // namespace avmedia

#endif // INCLUDED_AVMEDIA_SOURCE_GSTREAMER_GSTFRAMEGRABBER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
