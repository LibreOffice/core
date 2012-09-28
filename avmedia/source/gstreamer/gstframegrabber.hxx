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

#ifndef _FRAMEGRABBER_HXX
#define _FRAMEGRABBER_HXX

#include "gstplayer.hxx"
#include "com/sun/star/media/XFrameGrabber.hdl"
#include <cppuhelper/implbase2.hxx>

namespace avmedia { namespace gstreamer {

// ----------------
// - FrameGrabber -
// ----------------

typedef ::cppu::WeakImplHelper2< ::com::sun::star::media::XFrameGrabber,
                                 ::com::sun::star::lang::XServiceInfo > FrameGrabber_BASE;

class FrameGrabber : public FrameGrabber_BASE
{
    GstElement *mpPipeline;
    void disposePipeline();
public:
    // static create method instead of public Ctor
    static FrameGrabber* create( const OUString &rURL );

    virtual ~FrameGrabber();

    // XFrameGrabber
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL grabFrame( double fMediaTime ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

private:
    FrameGrabber( const OUString &aURL );
    FrameGrabber( const FrameGrabber& );
    FrameGrabber& operator=( const FrameGrabber& );
};

} // namespace gst
} // namespace avmedia

#endif // _FRAMEGRABBER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
