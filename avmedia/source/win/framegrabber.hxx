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

#ifndef INCLUDED_AVMEDIA_SOURCE_WIN_FRAMEGRABBER_HXX
#define INCLUDED_AVMEDIA_SOURCE_WIN_FRAMEGRABBER_HXX

#include "wincommon.hxx"
#include <cppuhelper/implbase.hxx>
#include "com/sun/star/media/XFrameGrabber.hpp"

struct IMediaDet;

namespace avmedia { namespace win {

class FrameGrabber : public ::cppu::WeakImplHelper< css::media::XFrameGrabber,
                                                    css::lang::XServiceInfo >
{
public:

    explicit FrameGrabber( const css::uno::Reference< css::lang::XMultiServiceFactory >& rxMgr );
    ~FrameGrabber();

    bool    create( const OUString& rURL );

    // XFrameGrabber
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL grabFrame( double fMediaTime ) throw (css::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException);

private:

    css::uno::Reference< css::lang::XMultiServiceFactory >    mxMgr;
    OUString                                                  maURL;

    IMediaDet* implCreateMediaDet( const OUString& rURL ) const;
};

} // namespace win
} // namespace avmedia

#endif // INCLUDED_AVMEDIA_SOURCE_WIN_FRAMEGRABBER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
