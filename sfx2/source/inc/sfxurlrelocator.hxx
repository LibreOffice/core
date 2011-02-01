/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SFX_SFXURLRELOCATOR_HXX_
#define _SFX_SFXURLRELOCATOR_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XOfficeInstallationDirectories.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>

class SfxURLRelocator_Impl
{
    ::osl::Mutex maMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >           mxFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XOfficeInstallationDirectories > mxOfficeInstDirs;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XMacroExpander >                 mxMacroExpander;

public:
    static bool                 propertyCanContainOfficeDir( const rtl::OUString & rPropName );
    void                        initOfficeInstDirs();
    void                        makeRelocatableURL( rtl::OUString & rURL );
    void                        makeAbsoluteURL( rtl::OUString & rURL );

    SfxURLRelocator_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory );
    ~SfxURLRelocator_Impl();

private:
    void implExpandURL( ::rtl::OUString& io_url );
};

#endif

