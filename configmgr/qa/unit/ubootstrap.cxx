/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ubootstrap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 13:59:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
