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
#include <cppuhelper/implbase2.hxx>
#include <boost/noncopyable.hpp>

namespace avmedia { namespace gstreamer {


// - FrameGrabber -


typedef ::cppu::WeakImplHelper2< ::com::sun::star::media::XFrameGrabber,
                                 ::com::sun::star::lang::XServiceInfo > FrameGrabber_BASE;

class FrameGrabber : public FrameGrabber_BASE, private boost::noncopyable
{
    GstElement *mpPipeline;
    void disposePipeline();
public:
    // static create method instead of public Ctor
    static FrameGrabber* create( const OUString &rURL );

    virtual ~FrameGrabber();

    // XFrameGrabber
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL grabFrame( double fMediaTime ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    FrameGrabber( const OUString &aURL );
};

} // namespace gst
} // namespace avmedia

#endif // INCLUDED_AVMEDIA_SOURCE_GSTREAMER_GSTFRAMEGRABBER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
