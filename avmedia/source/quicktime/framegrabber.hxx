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

#ifndef INCLUDED_AVMEDIA_SOURCE_QUICKTIME_FRAMEGRABBER_HXX
#define INCLUDED_AVMEDIA_SOURCE_QUICKTIME_FRAMEGRABBER_HXX

#include "quicktimecommon.hxx"
#include <cppuhelper/implbase.hxx>

#include "com/sun/star/media/XFrameGrabber.hpp"

namespace avmedia { namespace quicktime {

class FrameGrabber : public ::cppu::WeakImplHelper< css::media::XFrameGrabber,
                                                    css::lang::XServiceInfo >
{
public:

            FrameGrabber( const css::uno::Reference< css::lang::XMultiServiceFactory >& rxMgr );
            virtual ~FrameGrabber();

    bool    create( const OUString& rURL );

    // XFrameGrabber
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL grabFrame( double fMediaTime ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName( ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames( ) override;

private:
    css::uno::Reference< css::lang::XMultiServiceFactory >    mxMgr;
    OUString                                                  maURL;
    QTMovie*                                                  mpMovie;
    bool                                                      mbInitialized;
};

} // namespace quicktime
} // namespace avmedia

#endif // INCLUDED_AVMEDIA_SOURCE_QUICKTIME_FRAMEGRABBER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
