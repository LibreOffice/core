/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// ----------------
// - SimpleResMgr -
// ----------------

// a simple resource manager : no stacks, no sharing of the impl class, only loading of strings and blobs
// but thread-safety !! :)

#ifndef _TOOLS_SIMPLERESMGR_HXX_
#define _TOOLS_SIMPLERESMGR_HXX_

#include <osl/mutex.hxx>
#include <tools/resid.hxx>
#include <i18npool/lang.h>
#include <tools/string.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include "tools/toolsdllapi.h"

class InternalResMgr;
class TOOLS_DLLPUBLIC SimpleResMgr
{
protected:
    osl::Mutex              m_aAccessSafety;
    InternalResMgr*         m_pResImpl;

public:
                            /** creates a new SimpleResManager
                                @param pPrefixName
                                    denotes the prefix of the resource file name, in ThreadTextEncoding
                                @param rLocale
                                    denotes the locale of the resource file to load. If empty, a default locale
                                    will be used.
                            */
                            SimpleResMgr( const sal_Char* pPrefixName,
                                          const ::com::sun::star::lang::Locale& _rLocale);

                            /** creates a new SimpleResManager
                                @param _rPrefixName
                                    denotes the prefix of the resource file name
                                @param _inout_Locale
                                    denotes the locale of the resource file to load. If empty, no default handling
                                    (like in the other constructor) will happen, instead an unlocalized version will be
                                    attempted to be loaded.
                                    Upon return, the variable will contain the actual locale of the loaded resource file.
                                    For instance, if "en-US" is requested, but only "en" exists, the latter will be loaded
                                    and returned. Furthermore, if an unlocalized resource file with only the base name exists,
                                    this one will be loaded as final fallback.
                            */
                            SimpleResMgr( const ::rtl::OUString& _rPrefixName, ::com::sun::star::lang::Locale& _inout_Locale );
    virtual                 ~SimpleResMgr();

    static SimpleResMgr*    Create( const sal_Char* pPrefixName,
                                      ::com::sun::star::lang::Locale aLocale = ::com::sun::star::lang::Locale( rtl::OUString(),
                                                                                                               rtl::OUString(),
                                                                                                               rtl::OUString()));// nur in VCL

    bool                    IsValid() const { return m_pResImpl != NULL; }

    /** retrieves the locale of the resource file represented by this instance
        @precond
            IsValid returns <TRUE/>
    */
    const ::com::sun::star::lang::Locale&
                            GetLocale() const;

    /** reads the string with the given resource id
        @param  nId
            the resource id of the string to read
        @return
            the string with the given resource id, or an empty string if the id does not denote
            an existent string
        @seealso IsAvailable
    */
    UniString               ReadString( sal_uInt32 nId );

    /** checks whether a certain resource is availble
        @param  _resourceType
            the type of the resource to check. Currently, only RSC_STRING (strings) and RSC_RESOURCE (blobs)
            are supported, for every other type, <FALSE/> will be returned.
        @param  _resourceId
            the id of the resource to lookup.
        @return
            <TRUE/> if and only if a resource of the given type, with the given id, is available.
    */
    bool                    IsAvailable( RESOURCE_TYPE _resourceType, sal_uInt32 _resourceId );

    sal_uInt32              ReadBlob( sal_uInt32 nId, void** pBuffer );
    void                    FreeBlob( void* pBuffer );
};

#endif // _TOOLS_SIMPLERESMGR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
