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
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#define _UNOTOOLS_LOCALFILEHELPER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include "unotools/unotoolsdllapi.h"

#include <rtl/ustring.hxx>
#include <tools/string.hxx>

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
    static sal_Bool ConvertPhysicalNameToURL( const String& rName, String& rReturn );
    static sal_Bool ConvertSystemPathToURL( const String& rName, const String& rBaseURL, String& rReturn );

                    /**
                    Converts a "UCB compatible" URL into a "physical" file name.
                    If no UCP is available for the local file system, sal_False and an empty file name is returned,
                    otherwise sal_True and a valid URL, because a file name can always be converted if a UCP for the local
                    file system is present ( watch: this doesn't mean that this file really exists! )
                    */
    static sal_Bool ConvertURLToPhysicalName( const String& rName, String& rReturn );
    static sal_Bool ConvertURLToSystemPath( const String& rName, String& rReturn );

    static sal_Bool IsLocalFile( const String& rName );
    static sal_Bool IsFileContent( const String& rName );

    static          ::com::sun::star::uno::Sequence< ::rtl::OUString >
                            GetFolderContents( const ::rtl::OUString& rFolder, sal_Bool bFolder );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
