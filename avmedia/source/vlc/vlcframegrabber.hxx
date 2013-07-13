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

#ifndef _VLCFRAMEGRABBER_HXX
#define _VLCFRAMEGRABBER_HXX

#include <boost/noncopyable.hpp>
#include <com/sun/star/media/XFrameGrabber.hpp>
#include <cppuhelper/implbase2.hxx>
#include "vlccommon.hxx"

namespace avmedia {
namespace vlc {

typedef ::cppu::WeakImplHelper2< ::com::sun::star::media::XFrameGrabber,
                                 ::com::sun::star::lang::XServiceInfo > FrameGrabber_BASE;

class VLCFrameGrabber : public FrameGrabber_BASE, boost::noncopyable
{
public:
    SAL_CALL VLCFrameGrabber();

    ::com::sun::star::uno::Reference< css::graphic::XGraphic > SAL_CALL grabFrame( double fMediaTime );

    ::rtl::OUString SAL_CALL getImplementationName();
    ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& serviceName );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames();
};

}
}

#endif // _VLCFRAMEGRABBER_HXX