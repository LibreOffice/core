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

#include "common.hxx"
#include <osl/file.hxx>

// Do all those evil things to make the tests actually work ...
Magic::Magic()
{
    rtl_uString *curWd = NULL;
    osl_getProcessWorkingDir( &curWd );

    rtl::OUString aKey;
    rtl::OUString aValue( curWd );

    // Unless you do this, obviously you don't want your paths to be correct
    // or file access to work properly
    aKey = rtl::OUString::createFromAscii( "BaseInstallation" );
    rtl_bootstrap_set( aKey.pData, aValue.pData );
    // Unless you do this, obviously you don't deserve to have anything work
    aKey = rtl::OUString::createFromAscii( "CFG_INIFILE" );
    aValue += rtl::OUString::createFromAscii( "/configmgrrc" );
    rtl_bootstrap_set( aKey.pData, aValue.pData );

    // FIXME: Create a safe /tmp directory - unfotunately that
    // either requires dependencies we don't have, or some cut &
    // paste action of unotools/source/ucbhelper/tempfile.cxx
    if (osl::File::getTempDirURL(maTempDir) != osl::FileBase::E_None) {
        t_print ("no tmp dir");
        CPPUNIT_FAIL ("no tmp dir");
    }
    maTempDir += rtl::OUString::createFromAscii("/unittstconfig");
    osl::FileBase::RC err = osl::Directory::create (maTempDir);

    if (err != osl::FileBase::E_None) {
        t_print ("tmp dir '%s' already exists\n",
                 rtl::OUStringToOString (maTempDir, RTL_TEXTENCODING_UTF8).getStr());
        CPPUNIT_FAIL ("tmp user config dir already exists!");
    }

    aKey = rtl::OUString::createFromAscii( "UserInstallation" );
    rtl_bootstrap_set( aKey.pData, maTempDir.pData );
}

void removeRecursive (const rtl::OUString& aPath)
{
    sal_Int32 nMask = FileStatusMask_Type | FileStatusMask_FileURL;

//    fprintf (stderr, "Remove recursive '%s'\n", rtl::OUStringToOString (aPath, RTL_TEXTENCODING_UTF8).getStr());

    osl::DirectoryItem aItem;
    osl::FileStatus aStatus( nMask );
    osl::FileBase::RC nError;

    nError = osl::DirectoryItem::get( aPath, aItem );
    CPPUNIT_ASSERT_MESSAGE ("invalid path", nError == osl::FileBase::E_None);

    nError = aItem.getFileStatus( aStatus );
    CPPUNIT_ASSERT_MESSAGE ("invalid file", nError == osl::FileBase::E_None);

    if (aStatus.getFileType() == osl::FileStatus::Regular ||
        aStatus.getFileType() == osl::FileStatus::Link )
    {
        nError = osl::File::remove( aPath );
//        fprintf (stderr, "Remove file '%s'\n", rtl::OUStringToOString (aPath, RTL_TEXTENCODING_UTF8).getStr());
        CPPUNIT_ASSERT_MESSAGE ("removing file", nError == osl::FileBase::E_None);
    }
    else if (aStatus.getFileType() == osl::FileStatus::Directory)
    {
        osl::Directory aDirectory (aPath);
        nError = aDirectory.open();
        CPPUNIT_ASSERT_MESSAGE ("opening dir", nError == osl::FileBase::E_None);

        rtl::OUString name;

        while (true) {
            nError = aDirectory.getNextItem( aItem );
            if ( nError != osl::FileBase::E_None )
                break;
              nError = aItem.getFileStatus( aStatus );
            CPPUNIT_ASSERT_MESSAGE ("getting status", nError == osl::FileBase::E_None);
            removeRecursive (aStatus.getFileURL());
        }
          aDirectory.close();

        nError = osl::Directory::remove( aPath );
//        fprintf (stderr, "Remove dir '%s'\n", rtl::OUStringToOString (aPath, RTL_TEXTENCODING_UTF8).getStr());

        CPPUNIT_ASSERT_MESSAGE ("removing directory", nError == osl::FileBase::E_None);
    }
}

Magic::~Magic()
{
    removeRecursive (maTempDir);
}
