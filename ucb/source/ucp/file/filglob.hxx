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
#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_FILGLOB_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_FILGLOB_HXX

#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>


namespace fileaccess {

    class BaseContent;

    /******************************************************************************/
    /*                                                                            */
    /*                         Helper functions                                   */
    /*                                                                            */
    /******************************************************************************/


    // Returns true if dstUnqPath is a child from srcUnqPath or both are equal

    extern bool isChild( const OUString& srcUnqPath,
                                      const OUString& dstUnqPath );


    // Changes the prefix in name
    extern OUString newName( const OUString& aNewPrefix,
                                           const OUString& aOldPrefix,
                                           const OUString& old_Name );

    // returns the last part of the given url as title
    extern OUString getTitle( const OUString& aPath );

    // returns the url without last part as parentname
    // In case aFileName is root ( file:/// ) root is returned

    extern OUString getParentName( const OUString& aFileName );

    /**
     *  special copy:
     *  On test = true, the implementation determines whether the
     *  destination exists and returns the appropriate errorcode E_EXIST.
     *  osl::File::copy copies unchecked.
     */

    extern osl::FileBase::RC osl_File_copy( const OUString& strPath,
                                            const OUString& strDestPath,
                                            bool test );

    /**
     *  special move:
     *  On test = true, the implementation determines whether the
     *  destination exists and returns the appropriate errorcode E_EXIST.
     *  osl::File::move moves unchecked
     */

    extern osl::FileBase::RC osl_File_move( const OUString& strPath,
                                            const OUString& strDestPath,
                                            bool test = false );

    // This function implements the global exception handler of the file_ucp;
    // It never returns;

    extern void throw_handler( sal_Int32 errorCode,
                               sal_Int32 minorCode,
                               const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv,
                               const OUString& aUncPath,
                               BaseContent* pContent,
                               bool isHandled);
                                   // the physical URL of the object

} // end namespace fileaccess

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
