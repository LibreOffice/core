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
#ifndef _FILGLOB_HXX_
#define _FILGLOB_HXX_

#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>


namespace fileaccess {

    class BaseContent;

    struct equalOUString
    {
        bool operator()( const rtl::OUString& rKey1, const rtl::OUString& rKey2 ) const
        {
            return !!( rKey1 == rKey2 );
        }
    };


    struct hashOUString
    {
        size_t operator()( const rtl::OUString& rName ) const
        {
            return rName.hashCode();
        }
    };


    /******************************************************************************/
    /*                                                                            */
    /*                         Helper functions                                   */
    /*                                                                            */
    /******************************************************************************/


    // Returns true if dstUnqPath is a child from srcUnqPath or both are equal

    extern sal_Bool isChild( const rtl::OUString& srcUnqPath,
                                      const rtl::OUString& dstUnqPath );


    // Changes the prefix in name
    extern rtl::OUString newName( const rtl::OUString& aNewPrefix,
                                           const rtl::OUString& aOldPrefix,
                                           const rtl::OUString& old_Name );

    // returns the last part of the given url as title
    extern rtl::OUString getTitle( const rtl::OUString& aPath );

    // returns the url without last part as parentname
    // In case aFileName is root ( file:/// ) root is returned

    extern rtl::OUString getParentName( const rtl::OUString& aFileName );

    /**
     *  special copy:
     *  On test = true, the implementation determines whether the
     *  destination exists and returns the appropriate errorcode E_EXIST.
     *  osl::File::copy copies unchecked.
     */

    extern osl::FileBase::RC osl_File_copy( const rtl::OUString& strPath,
                                            const rtl::OUString& strDestPath,
                                            sal_Bool test = false );

    /**
     *  special move:
     *  On test = true, the implementation determines whether the
     *  destination exists and returns the appropriate errorcode E_EXIST.
     *  osl::File::move moves unchecked
     */

    extern osl::FileBase::RC osl_File_move( const rtl::OUString& strPath,
                                            const rtl::OUString& strDestPath,
                                            sal_Bool test = false );

    // This function implements the global exception handler of the file_ucp;
    // It never returns;

    extern void throw_handler( sal_Int32 errorCode,
                               sal_Int32 minorCode,
                               const com::sun::star::uno::Reference<
                               com::sun::star::ucb::XCommandEnvironment >& xEnv,
                               const rtl::OUString& aUncPath,
                               BaseContent* pContent,
                               bool isHandled = false);
                                   // the physical URL of the object

} // end namespace fileaccess

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
