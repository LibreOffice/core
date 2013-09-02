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
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#define _UNOTOOLS_LOCALFILEHELPER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include "unotools/unotoolsdllapi.h"

#include <rtl/ustring.hxx>

namespace utl
{
    class UNOTOOLS_DLLPUBLIC LocalFileHelper
    {
    public:
        /**
        Converts a "physical" file name into a "UCB compatible" URL ( if possible ).
        If no UCP is available for the local file system, sal_False and an empty URL is returned.
        Returning sal_True and an empty URL means that the URL doesn't point to a local file.
        */
        static bool ConvertPhysicalNameToURL(const OUString& rName, OUString& rReturn);
        static sal_Bool ConvertSystemPathToURL( const OUString& rName, const OUString& rBaseURL, OUString& rReturn );

        /**
        Converts a "UCB compatible" URL into a "physical" file name.
        If no UCP is available for the local file system, sal_False and an empty file name is returned,
        otherwise sal_True and a valid URL, because a file name can always be converted if a UCP for the local
        file system is present ( watch: this doesn't mean that this file really exists! )
        */
        static bool ConvertURLToPhysicalName( const OUString& rName, OUString& rReturn );
        static sal_Bool ConvertURLToSystemPath( const OUString& rName, OUString& rReturn );

        static sal_Bool IsLocalFile(const OUString& rName);
        static sal_Bool IsFileContent(const OUString& rName);

        static          ::com::sun::star::uno::Sequence< OUString >
                                GetFolderContents( const OUString& rFolder, sal_Bool bFolder );
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
