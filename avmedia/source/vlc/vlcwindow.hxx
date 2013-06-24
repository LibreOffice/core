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
#ifndef _VLCWINDOW_HXX
#define _VLCWINDOW_HXX

#include "vlccommon.hxx"

namespace avmedia {
namespace vlc {

class VLCWindow : public ::cppu::WeakImplHelper2 < ::com::sun::star::media::XPlayerWindow,
                                                   ::com::sun::star::lang::XServiceInfo >
{
public:
    void SAL_CALL update();
    ::sal_Bool SAL_CALL setZoomLevel( css::media::ZoomLevel ZoomLevel );
    css::media::ZoomLevel SAL_CALL getZoomLevel();
    void SAL_CALL setPointerType( ::sal_Int32 SystemPointerType );

    ::rtl::OUString SAL_CALL getImplementationName();
    ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames();
};

}
}

#endif // _VLCWINDOW_HXX
