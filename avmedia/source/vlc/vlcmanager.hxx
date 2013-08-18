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

#ifndef _VLCMANAGER_HXX
#define _VLCMANAGER_HXX
#include <boost/shared_ptr.hpp>
#include "vlccommon.hxx"

#include "com/sun/star/media/XManager.hpp"
#include "wrapper/EventHandler.hxx"

namespace avmedia {
namespace vlc {

class Manager : public ::cppu::WeakImplHelper2 < ::com::sun::star::media::XManager,
                                                    ::com::sun::star::lang::XServiceInfo >
{
    boost::shared_ptr<VLC::EventHandler> mEventHandler;
public:
    Manager( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMgr );
    ~Manager();

    ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > SAL_CALL createPlayer( const rtl::OUString& aURL ) throw (::com::sun::star::uno::RuntimeException);

    rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL supportsService( const rtl::OUString& serviceName ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer >  mPlayer;
};

}
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */