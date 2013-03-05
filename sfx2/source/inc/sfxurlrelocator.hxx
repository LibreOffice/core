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

#ifndef _SFX_SFXURLRELOCATOR_HXX_
#define _SFX_SFXURLRELOCATOR_HXX_

#include <com/sun/star/util/XOfficeInstallationDirectories.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>

class SfxURLRelocator_Impl
{
    ::osl::Mutex maMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >               mxContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XOfficeInstallationDirectories > mxOfficeInstDirs;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XMacroExpander >                 mxMacroExpander;

public:
    static bool                 propertyCanContainOfficeDir( const rtl::OUString & rPropName );
    void                        initOfficeInstDirs();
    void                        makeRelocatableURL( rtl::OUString & rURL );
    void                        makeAbsoluteURL( rtl::OUString & rURL );

    SfxURLRelocator_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext );
    ~SfxURLRelocator_Impl();

private:
    void implExpandURL( ::rtl::OUString& io_url );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
