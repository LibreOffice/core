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
#ifndef _SFX_HELPER_HXX
#define _SFX_HELPER_HXX

// include ---------------------------------------------------------------

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/ucb/NameClash.hpp>

#include <tools/string.hxx>
#include <tools/errcode.hxx>
#include <tools/bigint.hxx>

// class SfxContentHelper ------------------------------------------------

class SfxContentHelper
{
private:
    static sal_Bool             Transfer_Impl( const String& rSource, const String& rDest, sal_Bool bMoveData,
                                                    sal_Int32 nNameClash );

public:
    static sal_Bool             IsDocument( const String& rContent );
    static sal_Bool             IsFolder( const String& rContent );
    static sal_Bool             GetTitle( const String& rContent, String& rTitle );
    static sal_Bool             Kill( const String& rContent );

    static ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                GetFolderContents( const String& rFolder, sal_Bool bFolder, sal_Bool bSorted = sal_False );
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                GetFolderContentProperties( const String& rFolder, sal_Bool bFolder );
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                GetResultSet( const String& rURL );
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                GetHelpTreeViewContents( const String& rURL );
    static String               GetActiveHelpString( const String& rURL );
    static sal_Bool             IsHelpErrorDocument( const String& rURL );

    static sal_Bool             CopyTo( const String& rSource, const String& rDest );
    static sal_Bool             MoveTo( const String& rSource, const String& rDest, sal_Int32 nNameClash = com::sun::star::ucb::NameClash::ERROR );

    static sal_Bool             MakeFolder( const String& rFolder );
    static ErrCode              QueryDiskSpace( const String& rPath, sal_Int64& rFreeBytes );
    static sal_uIntPtr              GetSize( const String& rContent );

    // please don't use this!
    static sal_Bool             Exists( const String& rContent );
    static sal_Bool             Find( const String& rFolder, const String& rName, String& rFile );
};

#endif // #ifndef _SFX_HELPER_HXX


