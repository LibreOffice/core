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

#pragma once

#include <com/sun/star/media/XFrameGrabber.hpp>
#include <cppuhelper/implbase.hxx>
#include "vlccommon.hxx"
#include <wrapper/Wrapper.hxx>

namespace avmedia::vlc {

typedef ::cppu::WeakImplHelper< css::media::XFrameGrabber,
                                css::lang::XServiceInfo > FrameGrabber_BASE;

class VLCFrameGrabber : public FrameGrabber_BASE
{
    wrapper::Instance mInstance;
    wrapper::Media mMedia;
    wrapper::Player mPlayer;
    wrapper::EventHandler& mEventHandler;
public:
    VLCFrameGrabber( wrapper::EventHandler& eh, const OUString& url );

    css::uno::Reference< css::graphic::XGraphic > SAL_CALL grabFrame( double fMediaTime ) override;

    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService( const OUString& serviceName ) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
