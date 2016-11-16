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

// a simple resource manager : no stacks, no sharing of the impl class, only loading of strings and blobs
// but thread-safety !! :)

#ifndef INCLUDED_TOOLS_SIMPLERM_HXX
#define INCLUDED_TOOLS_SIMPLERM_HXX

#include <osl/mutex.hxx>
#include <tools/resid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/toolsdllapi.h>

class InternalResMgr;

class TOOLS_DLLPUBLIC SimpleResMgr final
{
    osl::Mutex              m_aAccessSafety;
    InternalResMgr*         m_pResImpl;

public:
                            /** creates a new SimpleResManager
                                @param pPrefixName
                                    denotes the prefix of the resource file name,
                                    in ThreadTextEncoding
                                @param rLocale
                                    denotes the locale of the resource file to
                                    load. If empty (SYSTEM), a default locale will be used.
                            */
                            SimpleResMgr( const sal_Char* pPrefixName,
                                          const LanguageTag& rLocale);

                            ~SimpleResMgr();

    static SimpleResMgr*    Create( const sal_Char* pPrefixName,
                                    const LanguageTag& rLocale );// only in VCL

    bool                    IsValid() const { return m_pResImpl != nullptr; }

    /** reads the string with the given resource id
        @param  nId
            the resource id of the string to read
        @return
            the string with the given resource id, or an empty string if the id does not denote
            an existent string
        @seealso IsAvailable
    */
    OUString           ReadString( sal_uInt32 nId );

    /** checks whether a certain resource is available
        @param  _resourceType
            the type of the resource to check. Currently, only RSC_STRING (strings) and RSC_RESOURCE (blobs)
            are supported, for every other type, <FALSE/> will be returned.
        @param  _resourceId
            the id of the resource to lookup.
        @return
            <TRUE/> if and only if a resource of the given type, with the given id, is available.
    */
    bool                    IsAvailable( RESOURCE_TYPE _resourceType, sal_uInt32 _resourceId );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
