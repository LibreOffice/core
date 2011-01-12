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
#include "unotools/unotoolsdllapi.h"

#ifndef _UNOTOOLS_UCBHELPER_HXX
#define _UNOTOOLS_UCBHELPER_HXX

// include ---------------------------------------------------------------
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>

#include <tools/string.hxx>

namespace ucbhelper
{
    class Content;
}

namespace utl
{
    class UNOTOOLS_DLLPUBLIC UCBContentHelper
    {
    private:
        static sal_Bool             Transfer_Impl( const String& rSource, const String& rDest, sal_Bool bMoveData,
                                                        sal_Int32 nNameClash );

    public:
        static sal_Bool             IsDocument( const String& rContent );
        static sal_Bool             IsFolder( const String& rContent );
        static sal_Bool             GetTitle( const String& rContent, String& rTitle );
        static sal_Bool             Kill( const String& rContent );

        static ::com::sun::star::uno::Any GetProperty( const String& rURL, const ::rtl::OUString& rName );

        static ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                    GetFolderContents( const String& rFolder, sal_Bool bFolder, sal_Bool bSorted = sal_False );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > GetResultSet( const String& rURL );

        static sal_Bool             CopyTo( const String& rSource, const String& rDest );
        static sal_Bool             MoveTo( const String& rSource, const String& rDest, sal_Int32 nNameClash = com::sun::star::ucb::NameClash::ERROR );

        static sal_Bool             CanMakeFolder( const String& rFolder );
        static sal_Bool             MakeFolder( const String& rFolder, sal_Bool bNewOnly = sal_False );
        static sal_Bool             MakeFolder( ::ucbhelper::Content& rParent,
                                                const String& rTitle,
                                                ::ucbhelper::Content& rNewFolder,
                                                sal_Bool bNewOnly = sal_False );
        static sal_Bool             HasParentFolder( const String& rFolder );

        static sal_uLong                GetSize( const String& rContent );
        static sal_Bool             IsYounger( const String& rIsYoung, const String& rIsOlder );

        static sal_Bool             Exists( const String& rContent );
        static sal_Bool             FindInPath( const String& rPath, const String& rName, String& rFile, char cDelim = ';', sal_Bool bAllowWildCards = sal_True );
        static sal_Bool             Find( const String& rFolder, const String& rName, String& rFile, sal_Bool bAllowWildCards = sal_False );
        static sal_Bool             IsSubPath( const ::rtl::OUString& rPath, const ::rtl::OUString& rChildCandidate, const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentProvider >& xContentProvider = ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentProvider >() );
        static sal_Bool             EqualURLs( const ::rtl::OUString& aFirstURL, const ::rtl::OUString& aSecondURL );
    };
}

#endif


