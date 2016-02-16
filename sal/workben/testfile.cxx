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

#define UNICODE

#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/time.h>
#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef UNX
#include <wchar.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

using namespace osl;

rtl::OUString   root;

rtl::OUString   dir1;
rtl::OUString   dir2;
rtl::OUString   dir_on_server;
rtl::OUString   dir_not_exist;
rtl::OUString   dir_not_exist_on_server;
rtl::OUString   dir_wrong_semantic;

rtl::OUString   file1;
rtl::OUString   file2;
rtl::OUString   file3;
rtl::OUString   file_on_server;
rtl::OUString   file_not_exist;

void print_error(const ::rtl::OString& str, FileBase::RC rc);

void PressKey()
{
    printf("\nPress Return !\n");
    getchar();
}

void printFileName(const ::rtl::OUString& str)
{
    rtl::OString aString;

    aString = rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );

    printf( "%s", aString.getStr() );

    return;
}

// Initialization

sal_Bool testLineBreak( sal_Char *pCount , sal_uInt64 nLen , sal_uInt32 *cLineBreak )
{
    sal_Bool fSuccess=sal_False;
    *cLineBreak=0;

    if (nLen==0)
        return fSuccess;

    if ( *pCount==13 )
    {
        if (nLen>=1 && *(pCount+1)==10)
            *cLineBreak=2;
        else
            *cLineBreak=1;

        if (nLen>=2 && *(pCount+2)==10)
            (*cLineBreak)++;

        fSuccess=sal_True;
    }
    else if ( *pCount==10 )
    {
        *cLineBreak=1;
        fSuccess=sal_True;
    }

    return fSuccess;
}

// Initialization

sal_Bool Initialize()
{
    DirectoryItem   aItem;
    FileStatus      aStatus( osl_FileStatus_Mask_All );
    rtl_uString     *strExeFileURL=NULL;
    oslProcessError ProcessError;

    rtl::OUString   iniFileURL;
    File            *pFile;

    FileBase::RC    rc;

    sal_uInt64      uBytesRequested;
    sal_uInt64      uBytesRead;
    sal_Char        *pBuffer;
    sal_Char        *pBegin;
    sal_Char        *pCount;

    rtl::OUString   dir[12];

    // Open to the ini-file

    ProcessError=osl_getExecutableFile(&strExeFileURL);

    if ( ProcessError == osl_Process_E_None)
    {
        sal_Unicode *pExeFileCount=rtl_uString_getStr(strExeFileURL)+rtl_uString_getLength(strExeFileURL);

        // Search for the last slash in the Path
        while (*pExeFileCount!=L'/' && pExeFileCount>rtl_uString_getStr(strExeFileURL))
            pExeFileCount--;

        // iniFileURL = strExeFileURL without the filename of the exe-File
        iniFileURL=rtl::OUString( rtl_uString_getStr(strExeFileURL) ,(int) (pExeFileCount-rtl_uString_getStr(strExeFileURL)) );

        // add "/testfile.ini" to iniFileURL
        iniFileURL+=rtl::OUString("/testfile.ini");

        // Open the ini-File
        pFile=new File( iniFileURL );
        rc=pFile->open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
        if ( rc!=FileBase::E_None )
        {
            rtl_uString_release(strExeFileURL);
            return sal_False;
        }
    }
    else
    {
        rtl_uString_release(strExeFileURL);
        return sal_False;
    }

    // Get filesize of the ini-File

    rc=DirectoryItem::get( iniFileURL, aItem );
    if ( rc!=FileBase::E_None )
        return sal_False;

    rc=aItem.getFileStatus( aStatus );
    if ( rc!=FileBase::E_None )
        return sal_False;

    uBytesRequested=aStatus.getFileSize();

    // read ini-File
    rc=pFile->setPos( osl_Pos_Absolut, 0 );
    pBuffer=(sal_Char*) rtl_allocateMemory( (sal_uInt32) (uBytesRequested+1)*sizeof(sal_Char) );
    memset( pBuffer, 0, (sal_uInt32)(uBytesRequested+1)*sizeof(sal_Char) );

    rc=pFile->read( pBuffer , uBytesRequested , uBytesRead );
    if ( rc!=FileBase::E_None )
        return sal_False;

    pBegin=pBuffer;
    pCount=pBegin;

    for ( int i=0 ; i<12 ; i++ )
    {
        sal_uInt32      cLineBrake=0;
        while ( (static_cast<sal_uInt64>(pCount-pBuffer) < uBytesRead) && *pCount!='=')
            pCount++;

        pCount++;
        pBegin=pCount;

        while ( (static_cast<sal_uInt64>(pCount-pBuffer) < uBytesRead) && !testLineBreak(pCount,uBytesRead-(pCount-pBuffer), &cLineBrake))
            pCount++;

        dir[i]=rtl::OUString(pBegin, pCount-pBegin, RTL_TEXTENCODING_ASCII_US);

        pCount+=cLineBrake;
        pBegin=pCount;
    }

    root=rtl::OUString(dir[0]);
    dir1=rtl::OUString(dir[1]);
    dir2=rtl::OUString(dir[2]);
    dir_on_server=rtl::OUString(dir[3]);
    dir_not_exist=rtl::OUString(dir[4]);
    dir_not_exist_on_server=rtl::OUString(dir[5]);
    dir_wrong_semantic=rtl::OUString(dir[6]);

    file1=rtl::OUString(dir[7]);
    file2=rtl::OUString(dir[8]);
    file3=rtl::OUString(dir[9]);
    file_on_server=rtl::OUString(dir[10]);
    file_not_exist=rtl::OUString(dir[11]);

    // close the ini-file
    rc=pFile->close();

    rtl_freeMemory( pBuffer );

    // Create directories
    rc=Directory::create( dir1 );
    if ( rc!=FileBase::E_None )
        return sal_False;

    rc=Directory::create( dir2 );
    if ( rc!=FileBase::E_None )
        return sal_False;

    rc=Directory::create( dir_on_server );
    if ( rc!=FileBase::E_None )
        return sal_False;

    pFile=new File( file1 );
    rc=pFile->open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    if ( rc!=FileBase::E_None )
        return sal_False;
    rc=pFile->close();
    delete pFile;

    pFile=new File( file2 );
    rc=pFile->open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    if ( rc!=FileBase::E_None )
        return sal_False;
    rc=pFile->close();
    delete pFile;

    pFile=new File( file_on_server );
    rc=pFile->open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    if ( rc!=FileBase::E_None )
        return sal_False;
    rc=pFile->close();
    delete pFile;

    return sal_True;
}

// Shutdown

sal_Bool Shutdown()
{
    sal_Bool        fSuccess=sal_True;
    FileBase::RC    rc;
    File            *pFile;

    // remove created files

    pFile=new File( file1 );
    rc=pFile->remove( file1 );
    if ( rc!=FileBase::E_None )
        fSuccess=sal_False;
    delete pFile;

    pFile=new File( file2 );
    rc=pFile->remove( file2 );
    if ( rc!=FileBase::E_None )
        fSuccess=sal_False;
    delete pFile;

    // remove created directories

    rc=Directory::remove( dir1 );
    if ( rc!=FileBase::E_None )
        fSuccess=sal_False;

    rc=Directory::remove( dir2 );
    if ( rc!=FileBase::E_None )
        fSuccess=sal_False;

    // remove created file on the server

    pFile=new File( file_on_server );
    rc=pFile->remove( file_on_server );
    if ( rc!=FileBase::E_None )
        fSuccess=sal_False;
    delete pFile;

    // remove created directory on the server

    rc=Directory::remove( dir_on_server );
    if ( rc!=FileBase::E_None )
        fSuccess=sal_False;

    return fSuccess;
}

// helper functions

// Show FileType
void showFileType( FileStatus::Type aType )
{
    if ( aType==FileStatus::Directory )
        printf( "FileType: Directory \n" );
    else if ( aType==FileStatus::Volume )
        printf( "FileType: Volume \n" );
    else if ( aType==FileStatus::Regular )
        printf( "FileType: Regular \n" );
    else if ( aType==FileStatus::Unknown )
        printf( "FileType: Unknown \n" );

}

// Show Attributes
void showAttributes( sal_uInt64 uAttributes )
{
    if ( uAttributes==0 )
        printf( "No Attributes \n" );
    if ( uAttributes & osl_File_Attribute_ReadOnly )
        printf( "Attribute: ReadOnly \n" );
    if ( uAttributes & osl_File_Attribute_Hidden )
        printf( "Attribute: Hidden \n" );
    if ( uAttributes & osl_File_Attribute_Executable )
        printf( "Attribute: Executable \n");
    if ( uAttributes & osl_File_Attribute_GrpWrite )
        printf( "Attribute: GrpWrite \n");
    if ( uAttributes & osl_File_Attribute_GrpRead )
        printf( "Attribute: GrpRead \n" );
    if ( uAttributes & osl_File_Attribute_GrpExe )
        printf( "Attribute: GrpExe \n" );
    if ( uAttributes & osl_File_Attribute_OwnWrite )
        printf( "Attribute: OwnWrite \n");
    if ( uAttributes & osl_File_Attribute_OwnRead )
        printf( "Attribute: OwnRead \n" );
    if ( uAttributes & osl_File_Attribute_OwnExe )
        printf( "Attribute: OwnExe \n" );
    if ( uAttributes & osl_File_Attribute_OthWrite )
        printf( "Attribute: OthWrite \n" );
    if ( uAttributes & osl_File_Attribute_OthRead )
        printf( "Attribute: OthRead \n");
    if ( uAttributes & osl_File_Attribute_OthExe )
        printf( "Attribute: OthExe \n" );

    return;
}

// Show Time
void showTime( TimeValue aTime )
{
    TimeValue   aLocalTimeVal, aSystemTimeVal , aSysTimeVal;
    oslDateTime aDateTime, aSystemTime;

    if ( osl_getLocalTimeFromSystemTime( &aTime, &aLocalTimeVal ) )
    {
        if ( osl_getDateTimeFromTimeValue( &aLocalTimeVal, &aDateTime ) )
        {
            printf("\t%02i.%02i.%4i , %02i.%02i.%02i Uhr\n", aDateTime.Day, aDateTime.Month, aDateTime.Year, aDateTime.Hours, aDateTime.Minutes, aDateTime.Seconds);
        }
        else
            printf("Error !\n");
    }

    if ( osl_getDateTimeFromTimeValue( &aTime, &aSystemTime ) )
    {
        printf("SystemTime: \t\t%02i.%02i.%4i , %02i.%02i.%02i Uhr\n", aSystemTime.Day, aSystemTime.Month, aSystemTime.Year, aSystemTime.Hours, aSystemTime.Minutes, aSystemTime.Seconds);
    }
    else
        printf("Error !\n");

    //Verify

    if ( osl_getTimeValueFromDateTime( &aSystemTime, &aSystemTimeVal ) )
    {
        if ( ( aSystemTimeVal.Seconds == aTime.Seconds ) && ( aSystemTimeVal.Nanosec == aTime.Nanosec ))
            printf ("Verify : TimeValue : ok! \n");
        else
        {
            printf ("Verify : TimeValue : Error! \n");
            printf ("aTime : %u \n", aTime.Seconds);
            printf ("aSystemTimeVal : %u \n", aSystemTimeVal.Seconds);
        }
    }
    else
        printf ("Verify : TimeValue : Error! \n");

    if ( osl_getSystemTimeFromLocalTime( &aLocalTimeVal , &aSysTimeVal ) )
    {
        if ( ( aSysTimeVal.Seconds == aTime.Seconds ) && ( aSysTimeVal.Nanosec == aTime.Nanosec ))
            printf ("Verify : SystemTime : ok! \n");
        else
        {
            printf ("Verify : SystemTime : Error! \n");
            printf ("aTime : %u\n", aTime.Seconds);
            printf ("aSystemTimeVal : %u\n", aSysTimeVal.Seconds);
        }
    }
    else
        printf ("Verify : SystemTime : Error! \n");

    return;
}

TimeValue getSystemTime()
{
    TimeValue   aTime;
    time_t ltime;

    time( &ltime );

    aTime.Seconds = ltime;
    aTime.Nanosec = 0;

    return aTime;
}

// DirectoryOpenAndCloseTest

void DirectoryOpenAndCloseTest()
{
    FileBase::RC    rc;
    Directory   *pDir;

    printf( "--------------------------------------------\n");
    printf( "Directory-Open-And-Close-Test\n");
    printf( "--------------------------------------------\n\n");

    // open an existing directory

    pDir=new Directory( dir1 );
    printf( "Open an existing directory: ");
    printFileName( dir1 );
    printf( "\n" );

    rc= pDir->open();
    print_error( rtl::OString( "Open Directory" ), rc );

    if ( pDir->isOpen() )
    {
        print_error( rtl::OString( "Directory is Open" ), rc );
    }

    // Close Directory
    rc=pDir->close();
    print_error( rtl::OString( "Close Directory" ), rc );

    delete pDir;
    printf( "\n" );

    // open a not existing directory

    pDir=new Directory( dir_not_exist );

    printf( "Open a not existing directory: ");
    printFileName( dir_not_exist );
    printf( "\n" );

    rc= pDir->open();

    print_error( rtl::OString( "Open Directory" ), rc );

    delete pDir;

    printf( "\n" );

    // open a directory with a wrong semantic

    pDir=new Directory( dir_wrong_semantic );

    printf( "Open a directory with a wrong semantic: ");
    printFileName( dir_wrong_semantic );
    printf( "\n" );

    rc= pDir->open();
    print_error( rtl::OString( "Open Directory" ), rc );

    delete pDir;

    printf( "\n" );

    // open an existing directory on a server

    pDir=new Directory( dir_on_server );

    printf( "Open an existing directory on a server: ");
    printFileName( dir_on_server );
    printf( "\n" );

    rc= pDir->open();
    print_error( rtl::OString( "Open Directory" ), rc );

    // Close Directory
    rc=pDir->close();
    print_error( rtl::OString( "Close Directory" ), rc );

    delete pDir;
    printf( "\n" );

    // open a not existing directory on a server

    pDir=new Directory( dir_not_exist_on_server );

    printf( "Open a not existing directory on a server: ");
    printFileName( dir_not_exist_on_server );
    printf( "\n" );

    rc= pDir->open();
    print_error( rtl::OString( "Open Directory" ), rc );

    delete pDir;
    printf( "\n" );

    // Close a not existing directory

    pDir=new Directory( dir_not_exist );
    printf( "Close a not existing directory: ");
    printFileName( dir_not_exist );
    printf( "\n" );

    rc=pDir->close();
    print_error( rtl::OString( "Close Directory" ), rc );

    PressKey();
    return;

}

// DirectoryCreateAndRemoveTest

void DirectoryCreateAndRemoveTest()
{
    FileBase::RC    rc,rc1;
    Directory   *pDir;

    printf( "--------------------------------------------\n" );
    printf( "Directory-Create-And-Remove-Test\n" );
    printf( "--------------------------------------------\n\n" );

    // Create directory

    printf( "Create a not existing directory: ");
    printFileName( dir_not_exist );
    printf( "\n" );

    rc=Directory::create( dir_not_exist) ;
    print_error( rtl::OString( "Create Directory" ), rc );

    // Verify
    pDir=new Directory( dir_not_exist );

    rc= pDir->open();
    print_error( rtl::OString( "Verify" ), rc );
    pDir->close();
    delete pDir;

    printf( "\n" );

    // Create a directory on a server

    printf( "Create a not existing directory on a server: ");
    printFileName( dir_not_exist_on_server );
    printf( "\n" );

    rc=Directory::create( dir_not_exist_on_server );
    print_error( rtl::OString( "Create Directory" ), rc );

    // Verify
    pDir=new Directory( dir_not_exist_on_server );
    rc= pDir->open();
    print_error( rtl::OString( "Verify" ), rc );
    pDir->close();
    delete pDir;

    printf( "\n" );

    // Remove Directories

    printf( "Remove the created directories: \n" );

    rc=Directory::remove( dir_not_exist );

    rc1=Directory::remove( dir_not_exist_on_server );

    if ( rc==FileBase::E_None && rc1==FileBase::E_None )
        print_error( rtl::OString( "Remove Directories" ), FileBase::E_None );
    else if ( rc!=FileBase::E_None )
        print_error( rtl::OString( "Remove local Directory" ),rc );
    else
        print_error( rtl::OString( "Remove Directory on a server" ),rc1 );

    printf( "\n" );

    // Remove a not existing directory

    printf( "Remove a not existing directory: ");
    printFileName( dir_not_exist );
    printf( "\n" );

    rc=Directory::remove( dir_not_exist );
    print_error( rtl::OString( "Remove" ),rc );

    PressKey();
    return;
}

// FileOpenAndCloseTest

static void FileOpenAndCloseTest()
{
    FileBase::RC    rc;

    printf( "--------------------------------------------\n" );
    printf( "File-Open-And-Close-Test\n" );
    printf( "--------------------------------------------\n\n" );

    File    *pFile;

    pFile=new File( file1 );

    printf( "Open an existing file: ");
    printFileName( file1 );
    printf( "\n" );

    // open an existing file (Read)

    rc=pFile->open( osl_File_OpenFlag_Read );
    print_error( rtl::OString( "Open File (Read)" ), rc );

    // close the file

    rc=pFile->close();
    print_error( rtl::OString( "Close File" ), rc );

    printf( "\n" );

    // open an existing file (Write)

    rc=pFile->open( osl_File_OpenFlag_Write );
    print_error( rtl::OString( "Open File (Write)" ), rc );

    // close the file

     rc=pFile->close();
    print_error( rtl::OString( "Close File" ), rc );

    printf( "\n" );

    // close the file a second time

     rc=pFile->close();
    print_error( rtl::OString( "Close the file a second time" ), rc );

    delete pFile;
    PressKey();
}

// FileCreateAndRemoveTest

void FileCreateAndRemoveTest()
{
    FileBase::RC    rc;
    File    *pFile;

    printf( "--------------------------------------------\n" );
    printf( "File-Create-And-Remove-Test\n" );
    printf( "--------------------------------------------\n\n" );

    pFile=new File( file_not_exist );

    printf( "Create File: ");
    printFileName( file_not_exist );
    printf( "\n" );

    // open (create) a not existing file (Read and write)

    rc = pFile->open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );

    print_error( rtl::OString( "Create and Open File (Read & Write)" ), rc );

    // close the file

    rc=pFile->close();
    print_error( rtl::OString( "Close File" ), rc );

    // remove the file

    rc=pFile->remove( file_not_exist );
    print_error( rtl::OString(" Remove File" ), rc );

    printf( "\n" );

    // remove the same file a second time

    rc=pFile->remove( file_not_exist );
    print_error( rtl::OString( "Remove a not existing File" ), rc );

    // remove an open file

    pFile->open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );

    rc=pFile->remove( file_not_exist );
    print_error( rtl::OString( "Remove an open File" ), rc );

    pFile->close();
    pFile->remove( file_not_exist );

    PressKey();

    return;
}

// FileWriteAndReadTest

void FileWriteAndReadTest()
{
    FileBase::RC    rc;

    sal_uInt64 uWritten;
    sal_uInt64 uRead;
    const sal_Char *pWriteBuffer="Hier kommt der Osterhase !";
    sal_uInt64  nLen=strlen( pWriteBuffer );
    sal_Char *pReadBuffer;

    printf( "--------------------------------------------\n" );
    printf( "File-Write-And-Read-Test\n" );
    printf( "--------------------------------------------\n\n" );

    File    *pFile;

    pFile=new File( file_not_exist );

    printf( "Create File: ");
    printFileName( file_not_exist );
    printf("\n");

    // open (create) a not existing file (Read and write)

    rc = pFile->open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );

    print_error( rtl::OString( "Create and Open File (Read & Write)" ), rc );

    printf( "\n" );

    // write a string to the file

    rc=pFile->write( pWriteBuffer , nLen , uWritten );
    print_error( rtl::OString( "Write File" ), rc );

    if(uWritten==nLen)
        printf( "Verify: OK! \n" );
    else
        printf( "Verify: Error\n" );

    printf( "\n" );

    // move the filepointer to the beginning

    rc=pFile->setPos( osl_Pos_Absolut , 0 );
    print_error( rtl::OString( "Set FilePointer to the beginning of the file" ), rc );

    printf( "\n" );

    // read the string

    pReadBuffer=(sal_Char*) rtl_allocateMemory( (sal_uInt32)(nLen+1)*sizeof(sal_Char) );
    memset( pReadBuffer, 0, (sal_uInt32)(nLen+1)*sizeof(sal_Char) );
    rc=pFile->read( pReadBuffer , nLen,uRead );
    print_error( rtl::OString( "Read File" ), rc );

    if (strcmp(pWriteBuffer, pReadBuffer)==0)
    {
        printf( "Verify: OK !\n" );
        printf( "Text: %s\n",pReadBuffer );
    }
    else
        printf( "Verify: Error\n" );

    rtl_freeMemory( pReadBuffer );

    printf( "\n" );

    // close the file
    rc=pFile->close();
    print_error( rtl::OString( "Close File" ), rc );

    // remove the file
    rc=pFile->remove( file_not_exist );
    print_error( rtl::OString( "Remove File" ), rc );

    PressKey();

    return;

}

// FileCopyMoveTest

void FileCopyAndMoveTest()
{
    FileBase::RC    rc;

    printf( "--------------------------------------------\n" );
    printf( "File-Copy-Move-Test\n" );
    printf( "--------------------------------------------\n\n" );

    File    *pFile;
    rtl::OUString   destPath(dir2);

    // FileCopyTest

    destPath+=rtl::OUString("/");
    destPath+=file3;

    printf( "Copy the file ");
    printFileName( file1 );
    printf( " to ");
    printFileName( destPath );
    printf( "\n" );

    rc=File::copy( file1 , destPath );
    print_error( rtl::OString( "FileCopy" ), rc );

    pFile=new File( destPath );

    rc=pFile->open( osl_File_OpenFlag_Read );
    if ( rc == FileBase::E_None)
    {
        printf( "Verify: OK!\n" );
        pFile->close();
        File::remove( destPath );
    }
    else
        printf( "Verify: Error!\n" );

    delete pFile;

    printf( "\n" );

    // Copy a file to a not existing directory

    destPath=rtl::OUString( dir_not_exist );
    destPath+=rtl::OUString("/");
    destPath+=file3;

    printf( "Copy a file to a not existing directory \n");
    printf( "Copy the file %s to %s\n",
        rtl::OUStringToOString( file1, RTL_TEXTENCODING_ASCII_US ).getStr(),
        rtl::OUStringToOString( destPath, RTL_TEXTENCODING_ASCII_US ).getStr() );

    rc=File::copy( file1, destPath );
    print_error( rtl::OString( "FileCopy" ), rc );

    printf( "\n" );

    // Copy a directory

    printf( "Copy the directory: ");
    printFileName( dir1 );
    printf( " to ");
    printFileName( dir2 );
    printf( "\n" );

    rc=File::copy( dir1 , dir2 );
    print_error( rtl::OString( "FileCopy" ), rc );

    printf( "\n" );

    // FileMoveTest

    destPath=rtl::OUString( dir2 );
    destPath+=rtl::OUString("/");
    destPath+=file3;

    printf( "Move the file ");
    printFileName( file1 );
    printf( " to ");
    printFileName( destPath );
    printf( "\n" );

    rc=File::move( file1, destPath );
    print_error( rtl::OString( "FileMove" ), rc );

    pFile=new File( destPath );

    rc=pFile->open( osl_File_OpenFlag_Read );
    if ( rc==FileBase::E_None )
    {
        pFile->close();

        delete pFile;
        pFile=new File( file1 );

        rc=pFile->open( osl_File_OpenFlag_Read );

        if ( rc!=FileBase::E_None )
        {
            printf( "Verify: OK!\n" );
            File::move( destPath, file1 );
        }
        else
        {
            printf( "Verify: Error!\n" );
            pFile->close();
            File::remove( destPath );
        }
    }
    else
        printf( "Verify: Error!\n" );

    delete pFile;

    printf( "\n" );

    // Move a file to a not existing directory

    destPath=rtl::OUString( dir_not_exist );
    destPath+=rtl::OUString("/");
    destPath+=file3;

    printf( "Move a file to a not existing directory: \n");
    printf( "Move the file ");
    printFileName( file1 );
    printf( " to ");
    printFileName( destPath );
    printf( "\n" );

    rc=File::move( file1 , destPath );
    print_error( rtl::OString( "FileMove" ), rc );

    printf( "\n" );

    // Move a directory

    printf( "Move a directory: \n");

    printf( "Move the directory ");
    printFileName( dir1 );
    printf( " to ");
    printFileName( dir_not_exist );
    printf( "\n" );

    rc=File::move( dir1 , dir_not_exist);
    print_error( rtl::OString( "FileMove" ), rc );

    if ( rc == FileBase::E_None )
        File::move( dir_not_exist , dir1);

    printf( "\n" );

    PressKey();
    return;
}

// FileSizeTest

void FileSizeTest()
{
    FileBase::RC    rc;
    sal_uInt64      filesize;
    DirectoryItem   aItem;

    printf( "--------------------------------------------\n" );
    printf( "File-Size-Test\n" );
    printf( "--------------------------------------------\n\n" );

    File    aFile( file_not_exist );

    printf( "Create File: ");
    printFileName( file_not_exist );
    printf( "\n\n");

    rc = aFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    print_error( rtl::OString( "Create and Open File (Read & Write)" ), rc );
    printf( "\n" );

    if ( rc == FileBase::E_None )
    {

        // Set Filesize to 5000

        printf( "Set FileSize to 5000\n" );
        rc=aFile.setSize( 5000 );
        print_error( rtl::OString( "Set FileSize" ), rc );

        printf( "\n" );
        printf( "Verify:\n" );

        // Check whether Filesize is set to 5000

        rc=DirectoryItem::get( file_not_exist, aItem );
        print_error( rtl::OString( "Get DirectoryItem" ), rc );

        if ( rc == FileBase::E_None )
        {
            FileStatus rStatus( osl_FileStatus_Mask_FileSize  );
            rc=aItem.getFileStatus( rStatus );
            print_error( rtl::OString( "Get FileStatus" ), rc );

            if ( rc == FileBase::E_None )
            {
                filesize=rStatus.getFileSize();

                if ( filesize == 5000 )
                    printf( "\nOK : FileSize: %" SAL_PRIuUINT64 "\n", filesize );
                else
                    printf( "\nError : FileSize: %" SAL_PRIuUINT64 "\n", filesize );
            }
        }

        printf( "\n" );

        // Set Filesize to -1

        printf( "Set FileSize to -1\n" );
        rc=aFile.setSize( -1 );
        print_error( rtl::OString( "Set FileSize" ), rc );

        printf( "\n" );

        // close the file
        rc=aFile.close();
        print_error( rtl::OString( "Close File" ), rc );

        // remove the file
        rc=File::remove( file_not_exist );
        print_error( rtl::OString( "Remove File" ), rc );
    }

    PressKey();

    return;
}

// FilePointerTest

void FilePointerTest()
{
    FileBase::RC    rc;
    sal_uInt64 filepointer;

    printf( "--------------------------------------------\n" );
    printf( "File-Pointer-Test\n" );
    printf( "--------------------------------------------\n\n" );

    File    rFile( file_not_exist );

    printf( "Create File: ");
    printFileName( file_not_exist );
    printf( "\n\n");

    rc = rFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    print_error( rtl::OString( "Create and Open File (Read & Write) "), rc );
    printf( "\n" );

    if ( rc==FileBase::E_None )
    {

        // get the position of the filepointer

        rc =rFile.getPos( filepointer );
        print_error( rtl::OString( "GetPos" ), rc );
        printf( "Position of the FilePointer: %" SAL_PRIuUINT64 "\n", filepointer );

        printf( "\n" );

        // set the filepointer at the end of a file

        printf( "Set FileSize to 5000\n" );
        rFile.setSize( 5000 );

        printf( "Set the FilePointer at the end of the file (5000)\n" );
        rc=rFile.setPos( osl_Pos_End,0 );
        print_error( rtl::OString( "SetPos" ), rc );

        rc=rFile.getPos( filepointer );

        if ( filepointer==5000 )
        {
            print_error( rtl::OString( "GetPos" ), rc );
            printf( "\nVerify: OK !\n" );
            printf( "Filepointer-Position: %" SAL_PRIuUINT64 "\n",filepointer );
        }
        else
        {
            print_error( rtl::OString( "GetPos" ), rc );
            printf( "\nFilePointer-Test: Error\n" );
            printf( "Filepointer-Position: %" SAL_PRIuUINT64 " != 5000 \n",filepointer );
        }

        printf( "\n" );

        // close the file
        rc=rFile.close();
        print_error( rtl::OString( "Close File" ), rc );

        // remove the file
        rc=File::remove( file_not_exist );
        print_error( rtl::OString( "Remove File" ), rc );
    }

    PressKey();

    return;
}

// FileAttributesTest

void verifyFileAttributes()
{
    FileBase::RC    rc;
    DirectoryItem   aItem;
    FileStatus rStatus( osl_FileStatus_Mask_Attributes  );

    printf( "\nVerify:\n" );

    rc=DirectoryItem::get( file1, aItem );

    if ( rc==FileBase::E_None )
    {
        rc=aItem.getFileStatus( rStatus );

        if ( rc==FileBase::E_None )
        {
            sal_uInt64 uAttributes=rStatus.getAttributes();
            showAttributes(uAttributes);
            printf( "\n" );
        }
        else
            print_error( rtl::OString( "Get FileStatus" ), rc );
    }
    else
        print_error( rtl::OString( "Get DirectoryItem" ), rc );

    return;
}

#ifdef UNX
void FileAttributesTest()
{
    FileBase::RC    rc;

    printf( "--------------------------------------------\n" );
    printf( "File-Attributes-Test\n" );
    printf( "--------------------------------------------\n\n" );

    printf( "File: ");
    printFileName( file1 );
    printf( "\n\n" );

    rc=File::setAttributes( file1, osl_File_Attribute_GrpWrite );
    print_error( rtl::OString( "Set Attribute: GrpWrite" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, 0 );
    if ( rc!=FileBase::E_None )
        print_error( rtl::OString( "Reset Attributes" ), rc );

    rc=File::setAttributes( file1, osl_File_Attribute_GrpRead );
    print_error( rtl::OString( "Set Attribute: GrpRead" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, 0 );
    if ( rc!=FileBase::E_None )
        print_error( rtl::OString( "Reset Attributes" ), rc );

    rc=File::setAttributes( file1, osl_File_Attribute_GrpExe );
    print_error( rtl::OString( "Set Attribute: GrpExe" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, 0 );
    if ( rc!=FileBase::E_None )
        print_error( rtl::OString( "Reset Attributes" ), rc );

    rc=File::setAttributes( file1, osl_File_Attribute_OwnWrite );
    print_error( rtl::OString( "Set Attribute: OwnWrite" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, 0 );
    if ( rc!=FileBase::E_None )
        print_error( rtl::OString( "Reset Attributes" ), rc );

    rc=File::setAttributes( file1, osl_File_Attribute_OwnRead );
    print_error( rtl::OString( "Set Attribute: OwnRead" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, 0 );
    if ( rc!=FileBase::E_None )
        print_error( rtl::OString( "Reset Attributes" ), rc );

    rc=File::setAttributes( file1, osl_File_Attribute_OwnExe );
    print_error( rtl::OString( "Set Attribute: OwnExe" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, 0 );
    if ( rc!=FileBase::E_None )
        print_error( rtl::OString( "Reset Attributes" ), rc );

    rc=File::setAttributes( file1, osl_File_Attribute_OthWrite );
    print_error( rtl::OString( "Set Attribute: OthWrite" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, 0 );
    if ( rc!=FileBase::E_None )
        print_error( rtl::OString( "Reset Attributes" ), rc );

    rc=File::setAttributes( file1, osl_File_Attribute_OthRead );
    print_error( rtl::OString( "Set Attribute: OthRead" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, 0 );
    if ( rc!=FileBase::E_None )
        print_error( rtl::OString( "Reset Attributes" ), rc );

    rc=File::setAttributes( file1, osl_File_Attribute_OthExe );
    print_error( rtl::OString( "Set Attribute: OthExe" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, 0 );
    if ( rc!=FileBase::E_None )
        print_error( rtl::OString( "Reset Attributes" ), rc );

    rc=File::setAttributes( file1, osl_File_Attribute_GrpWrite | osl_File_Attribute_GrpRead | osl_File_Attribute_GrpExe | osl_File_Attribute_OwnWrite | osl_File_Attribute_OwnRead | osl_File_Attribute_OwnExe | osl_File_Attribute_OthWrite | osl_File_Attribute_OthRead | osl_File_Attribute_OthExe );
    print_error( rtl::OString( "Set all Attributes" ), rc );

    verifyFileAttributes();

    PressKey();

    return;
}
#endif

#ifdef _WIN32
void FileAttributesTest()
{
    FileBase::RC    rc;

    printf( "--------------------------------------------\n" );
    printf( "File-Attributes-Test\n" );
    printf( "--------------------------------------------\n\n" );

    printf( "File: ");
    printFileName( file1 );
    printf( "\n\n" );

    rc=File::setAttributes( file1, osl_File_Attribute_ReadOnly );
    print_error( rtl::OString( "Set Attribute: ReadOnly" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, 0 );
    print_error( rtl::OString( "Reset Attributes" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, osl_File_Attribute_Hidden );
    print_error( rtl::OString( "Set Attribute: Hidden" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, 0 );
    print_error( rtl::OString( "Reset Attributes" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, osl_File_Attribute_Hidden | osl_File_Attribute_ReadOnly );
    print_error( rtl::OString( "Set Attribute: Hidden & ReadOnly" ), rc );

    verifyFileAttributes();

    rc=File::setAttributes( file1, 0 );
    print_error( rtl::OString( "Reset Attributes") , rc );

    verifyFileAttributes();

    PressKey();

    return;
}
#endif

// FileTimeTest

void FileTimeTest()
{
    FileBase::RC    rc;

    DirectoryItem   aItem;

    struct tm sSysCreationTime;
    sSysCreationTime.tm_sec = 0;
    sSysCreationTime.tm_min = 20;
    sSysCreationTime.tm_hour = 12;
    sSysCreationTime.tm_mday = 4;
    sSysCreationTime.tm_mon = 9;
    sSysCreationTime.tm_year = 99;

    struct tm sSysAccessTime;
    sSysAccessTime.tm_sec = 0;
    sSysAccessTime.tm_min = 40;
    sSysAccessTime.tm_hour = 1;
    sSysAccessTime.tm_mday = 6;
    sSysAccessTime.tm_mon = 5;
    sSysAccessTime.tm_year = 98;

    struct tm sSysModifyTime;
    sSysModifyTime.tm_sec = 0;
    sSysModifyTime.tm_min = 1;
    sSysModifyTime.tm_hour = 24;
    sSysModifyTime.tm_mday = 13;
    sSysModifyTime.tm_mon = 11;
    sSysModifyTime.tm_year = 95;

    sal_uInt32 aSysCreationTime = mktime( &sSysCreationTime );
    sal_uInt32 aSysAccessTime =  mktime( &sSysAccessTime );
    sal_uInt32 aSysModifyTime =  mktime( &sSysModifyTime );

    TimeValue       aCreationTime   = { aSysCreationTime, 0};
    TimeValue       aAccessTime = { aSysAccessTime, 0};
    TimeValue       aModifyTime = { aSysModifyTime, 0};

    TimeValue       aCreationTimeRead;
    TimeValue       aAccessTimeRead;
    TimeValue       aModifyTimeRead;

    printf( "--------------------------------------------\n" );
    printf( "File-Time-Test\n" );
    printf( "--------------------------------------------\n\n" );

    printf( "File: ");
    printFileName( file1 );
    printf( "\n\n" );

    printf( "CreationTime \t : ");
    showTime( aCreationTime );

    printf( "\nAccessTime \t : ");
    showTime( aAccessTime );

    printf( "\nModifyTime \t : ");
    showTime( aModifyTime );

    // setTime

    printf( "\n" );
    rc=File::setTime( file1 , aCreationTime , aAccessTime , aModifyTime );
    print_error( rtl::OString( "SetTime" ), rc );

    // Verify

    FileStatus rStatus( osl_FileStatus_Mask_CreationTime | osl_FileStatus_Mask_AccessTime | osl_FileStatus_Mask_ModifyTime);

    printf( "\nVerify:\n" );

    rc=DirectoryItem::get( file1, aItem );
    print_error( rtl::OString( "Get DirectoryItem" ), rc );

    if ( rc==FileBase::E_None )
    {
        rc=aItem.getFileStatus( rStatus );
        print_error( rtl::OString( "Get FileStatus" ), rc );
        printf( "\n" );

        if ( rc==FileBase::E_None )
        {

            // GetCreationTime

            aCreationTimeRead=rStatus.getCreationTime();
#ifdef _WIN32
            if ( aCreationTime.Seconds == aCreationTimeRead.Seconds && aCreationTime.Nanosec == aCreationTimeRead.Nanosec )
                printf( "GetCreationTime: ok : " );
            else
                printf( "GetCreationTime: Error : " );

            showTime( aCreationTimeRead );
            printf( "\n" );
#endif

            // GetAccessTime

            aAccessTimeRead=rStatus.getAccessTime();

            if ( aAccessTime.Seconds == aAccessTimeRead.Seconds && aAccessTime.Nanosec == aAccessTimeRead.Nanosec )
                printf( "GetAccessTime: ok : " );
            else
                printf( "GetAccessTime: Error : " );

            showTime( aAccessTimeRead );
            printf( "\n" );

            // GetModifyTime

            aModifyTimeRead=rStatus.getModifyTime();

            if ( aModifyTime.Seconds == aModifyTimeRead.Seconds && aModifyTime.Nanosec == aModifyTimeRead.Nanosec )
                printf( "GetModifyTime: ok : " );
            else
                printf( "GetModifyTime: Error : " );

            showTime( aModifyTimeRead );
            printf( "\n" );
        }
    }

    PressKey();
    return;
}

// DirectoryItemTest

void DirectoryItemTest()
{
    FileBase::RC    rc;
    Directory       *pDir;
    DirectoryItem   aItem;
    FileStatus      *pStatus;
    File            *pFile;

    printf( "--------------------------------------------\n" );
    printf( "Directory-Item-Test\n" );
    printf( "--------------------------------------------\n\n" );

    // get DirectoryItem from an existing directory

    printf( "Get DirectoryItem from an existing Directory: ");
    printFileName( dir1 );
    printf( "\n");

    rc=DirectoryItem::get( dir1 , aItem );
    print_error( rtl::OString( "GetDirectoryItem" ), rc );

    pStatus=new FileStatus( osl_FileStatus_Mask_All );
    rc=aItem.getFileStatus( *pStatus );

    if ( rc==FileBase::E_None )
    {
        printf( "GetFileStatus: FileURL: ");
        printFileName(pStatus->getFileURL() );
        printf( "\n" );
    }

    delete pStatus;

    printf( "\n" );

    // get DirectoryItem from a not existing directory

    printf( "Get DirectoryItem from a not existing Directory: ");
    printFileName( dir_not_exist );
    printf( "\n" );

    rc=DirectoryItem::get( dir_not_exist , aItem );
    print_error( rtl::OString( "Get DirectoryItem" ), rc );

    printf( "\n" );

    // get DirectoryItem from an existing file

    printf( "Get DirectoryItem from an existing File: ");
    printFileName( file1 );
    printf( "\n" );

    rc=DirectoryItem::get( file1 , aItem );
    print_error( rtl::OString( "GetDirectoryItem" ), rc );

    pStatus=new FileStatus( osl_FileStatus_Mask_All );
    rc=aItem.getFileStatus( *pStatus );

    if ( rc==FileBase::E_None )
    {
        printf( "GetFileStatus: FileURL: ");
        printFileName( pStatus->getFileURL() );
        printf( "\n" );
    }

    delete pStatus;

    printf( "\n" );

    // get DirectoryItem from a not existing file

    printf( "Get DirectoryItem from a not existing File: ");
    printFileName( file_not_exist );
    printf( "\n" );

    rc=DirectoryItem::get( file_not_exist , aItem );
    print_error( rtl::OString( "Get DirectoryItem" ), rc );

    printf( "\n" );

    // get DirectoryItem from a directory with a wrong semantic

    printf( "Get DirectoryItem from a Directory with a wrong semantic: ");
    printFileName( dir_not_exist );
    printf( "\n" );

    rc=DirectoryItem::get( dir_wrong_semantic, aItem );
    print_error( rtl::OString( "Get DirectoryItem" ),rc );

    printf( "\n" );

    // get DirectoryItem from a file-handle

    pFile=new File( file1 );

    rc=pFile->open( osl_File_OpenFlag_Read );
    if ( rc==FileBase::E_None )
    {
        printf( "Get DirectoryItem from a File-Handle: ");
        printFileName( file1 );
        printf( "\n" );

        rc=DirectoryItem::get( file1 , aItem );
        print_error( rtl::OString( "GetDirectoryItem" ), rc );

        pStatus=new FileStatus( osl_FileStatus_Mask_All );
        rc=aItem.getFileStatus( *pStatus );

        if ( rc==FileBase::E_None )
        {
            printf( "GetFileStatus: FileURL:  ");
            printFileName( pStatus->getFileURL() );
            printf( "\n");
        }

        delete pStatus;

        pFile->close();
    }

    delete pFile;

    printf( "\n" );

    // get DirectoryItem

    printf( "Get DirectoryItem from an empty File-Handle\n" );
    rc=DirectoryItem::get( file1 , aItem );
    print_error( rtl::OString( "GetDirectoryItem" ), rc );

    printf( "\n" );

    // GetNextItem from a directory

    pDir=new Directory( dir1 );
    printf( "Get next DirectoryItem from a directory: ");
    printFileName( dir1);
    printf( "\n" );

    rc= pDir->open();
    print_error( rtl::OString( "Open Directory" ), rc );

    printf( "\n" );

    if ( pDir->isOpen() )
    {

        // get all files from the directory

        rtl::OUString   str;
        rtl::OUString   str1[2];

        aItem=DirectoryItem();
        rc=pDir->getNextItem( aItem );
        print_error( rtl::OString( "GetNextItem" ),rc );

        while( rc==FileBase::E_None )
        {

            FileStatus rStatus( osl_FileStatus_Mask_All );
            aItem.getFileStatus( rStatus );

            str=rStatus.getFileName();
            printf( "Filename: ");
            printFileName( str );
            printf( "\n");

            aItem=DirectoryItem();
            rc=pDir->getNextItem( aItem );
            print_error( rtl::OString( "GetNextItem" ),rc );
        }

        printf( "\n" );

        // Reset-Test

        for (int i=0; i<2; i++)
        {
            aItem=DirectoryItem();
            rc=pDir->reset();
            rc=pDir->getNextItem( aItem );

            FileStatus rStatus( osl_FileStatus_Mask_All );
            aItem.getFileStatus( rStatus );

            str1[i]=rStatus.getFileName();
        }

        if ( str1[0] == str1[1] )
            print_error( rtl::OString( "Reset" ),FileBase::E_None );
        else
            print_error( rtl::OString( "Reset" ),FileBase::E_invalidError );

        printf( "\n" );

        // Close Directory
        rc=pDir->close();
        print_error( rtl::OString( "Close Directory" ), rc );
    }

    printf( "\n");

    // GetNextItem from a closed directory

    printf( "Get next DirectoryItem from a closed directory: ");
    printFileName( dir1 );
    printf( "\n" );

    aItem=DirectoryItem();
    rc=pDir->getNextItem( aItem );
    print_error( rtl::OString( "GetNextItem" ),rc );

    delete pDir;

    PressKey();
    return;
}

// FileStatusTest (for different types)

void FileStatusTest( FileStatus *pStatus )
{

    // GetFileType of the directory

    FileStatus::Type    aType;

    printf( "\ngetFileType:\n" );
    aType=pStatus->getFileType();
    showFileType( aType );

    // GetAttributes

    sal_uInt64          uAttributes;

    printf( "\ngetAttributes:\n" );
    uAttributes=pStatus->getAttributes();
    showAttributes( uAttributes );

    // GetCreationTime

    TimeValue   aCreationTime;

    printf( "\ngetCreationTime:\n" );
    aCreationTime=pStatus->getCreationTime();

    printf( "CreationTime: " );
    showTime( aCreationTime );

    // GetAccessTime

    TimeValue   aAccessTime;

    printf( "\ngetAccessTime:\n" );
    aAccessTime=pStatus->getAccessTime();

    printf( "AccessTime: " );
    showTime( aAccessTime );

    // GetModifyTime

    TimeValue   aModifyTime;

    printf( "\ngetModifyTime:\n" );
    aModifyTime=pStatus->getModifyTime();

    printf( "ModifyTime: " );
    showTime( aModifyTime );

    // GetFileSize

    sal_uInt64          FileSize;

    printf( "\ngetFileSize:\n" );

    FileSize=pStatus->getFileSize();
    printf( "FileSize: %" SAL_PRIuUINT64 "\n", FileSize);

    // GetFileName

    rtl::OUString           FileName;

    printf( "\ngetFileName:\n" );

    FileName=pStatus->getFileName();
    printf( "FileName: ");
    printFileName( FileName );
    printf( "\n" );

    // GetFileURL

    rtl::OUString           FileURL;

    printf( "\ngetFileURL:\n" );

    FileURL=pStatus->getFileURL();
    printf( "FileURL: ");
    printFileName( FileURL );
    printf( "\n" );

    // GetLinkTargetURL

    rtl::OUString           LinkTargetURL;

    printf( "\ngetLinkTargetURL:\n");

    LinkTargetURL=pStatus->getLinkTargetURL();
    printf( "LinkTargetURL: ");
    printFileName( LinkTargetURL );
    printf( "\n" );

    return;
}

// DirectoryFileStatusTest

void DirectoryFileStatusTest()
{
    FileBase::RC    rc;
    DirectoryItem   aItem;

    printf( "--------------------------------------------\n" );
    printf( "Directory-FileStatus-Test\n" );
    printf( "--------------------------------------------\n\n" );

    printf( "FileStatus of the directory: ");
    printFileName( dir1 );
    printf( "\n" );

    aItem=DirectoryItem();

    rc=DirectoryItem::get( dir1, aItem );
    print_error( rtl::OString( "Get DirectoryItem" ), rc );

    if ( rc==FileBase::E_None )
    {
        FileStatus *pStatus=new FileStatus( osl_FileStatus_Mask_All );
        rc=aItem.getFileStatus( *pStatus );

        FileStatusTest( pStatus );

        delete pStatus;
    }

    printf( "\n" );

    PressKey();
    return;
}

// FileFileStatusTest

void FileFileStatusTest()
{
    FileBase::RC    rc;
    DirectoryItem   aItem;

    printf( "--------------------------------------------\n" );
    printf( "File-FileStatus-Test\n" );
    printf( "--------------------------------------------\n\n" );

    printf( "FileStatus of the file: ");
    printFileName( file1 );
    printf( "\n" );

    aItem=DirectoryItem();

    rc=DirectoryItem::get( file1 , aItem );
    print_error( rtl::OString( "Get DirectoryItem" ), rc );

    if ( rc==FileBase::E_None )
    {
        FileStatus *pStatus=new FileStatus( osl_FileStatus_Mask_All );
        rc=aItem.getFileStatus( *pStatus );

        FileStatusTest( pStatus );

        delete pStatus;
    }

    printf( "\n" );

    PressKey();
    return;
}

// VolumeFileStatusTest

void VolumeFileStatusTest()
{
    FileBase::RC    rc;
    DirectoryItem   aItem;

    printf( "--------------------------------------------\n" );
    printf( "Volume-FileStatus-Test\n" );
    printf( "--------------------------------------------\n\n" );

    printf( "FileStatus of the Volume: ");
    printFileName( root );
    printf( "\n" );

    aItem=DirectoryItem();

    rc=DirectoryItem::get( root , aItem );
    print_error( rtl::OString( "Get DirectoryItem" ), rc );

    if ( rc==FileBase::E_None )
    {
        FileStatus *pStatus=new FileStatus( osl_FileStatus_Mask_All) ;
        rc=aItem.getFileStatus( *pStatus );

        FileStatusTest( pStatus );

        delete pStatus;
    }

    printf( "\n" );

    PressKey();
    return;
}

// VolumeInfoTest

void VolumeInfoTest()
{
    FileBase::RC    rc;

    printf( "--------------------------------------------\n" );
    printf( "Volume-Info-Test\n" );
    printf( "--------------------------------------------\n\n" );

    printf( "VolumeInfo of the volume ");
    printFileName( root );

    printf( "\n" );

    VolumeInfo rInfo( osl_VolumeInfo_Mask_FreeSpace );
    rc=Directory::getVolumeInfo( root , rInfo );
    print_error( rtl::OString( "GetVolumeInfo" ),rc );

    printf( "\n" );

    // getRemoteFlag

    if ( rInfo.getRemoteFlag() )
        printf( "RemoteFlag: Yes\n" );
    else
        printf( "RemoteFlag: No\n" );

    // getRemoveableFlag

    if ( rInfo.getRemoveableFlag() )
        printf( "RemoveableFlag: Yes\n" );
    else
        printf( "RemoveableFlag: No\n" );

    // getTotalSpace

    sal_uInt64  TotalSpace;
    TotalSpace=rInfo.getTotalSpace();
    printf( "Total Space: %" SAL_PRIuUINT64 "\n",TotalSpace );

    // getFreeSpace

    sal_uInt64  FreeSpace;
    FreeSpace=rInfo.getFreeSpace();
    printf( "Free Space: %" SAL_PRIuUINT64 "\n",FreeSpace );

    // getUsedSpace

    sal_uInt64  UsedSpace;
    UsedSpace=rInfo.getUsedSpace();
    printf( "Used Space: %" SAL_PRIuUINT64 "\n",UsedSpace );

    // getMaxNameLength

    sal_uInt32  MaxNameLength;
    MaxNameLength=rInfo.getMaxNameLength();
    printf( "MaxNameLength: %" SAL_PRIuUINT32 "\n",MaxNameLength );

    // getMaxPathLength

    sal_uInt32  MaxPathLength;
    MaxPathLength=rInfo.getMaxPathLength();
    printf( "MaxPathLength: %" SAL_PRIuUINT32 "\n",MaxPathLength );

    // getFileSystemName

    rtl::OUString   FileSystemName;
    FileSystemName=rInfo.getFileSystemName();
    printf( "File-System-Name: ");
    printFileName( FileSystemName );
    printf( "\n" );

    PressKey();
    return;
}

// FileBaseTest

void ConvertPathTest(rtl::OUString& strPath)
{
    FileBase::RC    rc;

    rtl::OUString       strNormPath;
    rtl::OUString       strFileURL;
    rtl::OUString       strNormPathFromFileURL;
    rtl::OUString       strSystemPath;

    // normalizePath

    rc=FileBase::getFileURLFromSystemPath( strPath, strNormPath );

    if ( rc == FileBase::E_None )
    {
        printf( "Normalized Path: \t\t");
        printFileName( strNormPath );
        printf( "\n" );
    }
    else
        printf( "normalizePath: Error \n" );

    // getFileURLFromSystemPath

    if ( !strNormPath.isEmpty() )
    {
        rc=FileBase::getFileURLFromSystemPath( strNormPath, strFileURL );

        if ( rc == FileBase::E_None )
        {
            printf( "File-URL: \t\t\t");
            printFileName( strFileURL );
            printf( "\n" );
        }
        else
            printf( "getFileURLFromSystemPath: Error \n" );
    }
    else
        printf( "getFileURLFromSystemPath: not tested \n" );

    // getNormalizedPathFromFileURL

    if ( !strFileURL.isEmpty() )
    {
        rc=FileBase::getSystemPathFromFileURL( strFileURL, strNormPathFromFileURL );

        if ( rc == FileBase::E_None )
        {
            printf( "Normalized Path from File-URL: \t");
            printFileName( strNormPathFromFileURL );
            printf( "\n" );
        }
        else
            printf( "getNormalizedPathFromFileURL: Error \n" );
    }
    else
        printf( "getNormalizedPathFromFileURL: not tested \n" );

    // getSystemPathFromFileURL

    if ( !strNormPath.isEmpty() )
    {
        rc=FileBase::getSystemPathFromFileURL( strNormPath, strSystemPath );

        if ( rc == FileBase::E_None )
        {
            printf( "System-Path: \t\t\t");
            printFileName( strSystemPath );
            printf( "\n");
        }
        else
            printf( "getSystemPathFromFileURL: Error \n" );
    }
    else
        printf( "getSystemPathFromFileURL: not tested \n" );

    // Verify

    if ( strNormPathFromFileURL == strNormPath )
        printf( "\nVerify: OK ! ( Normalized-Path == Normalized-Path-From-File-URL )\n" );
    else
        printf( "\nVerify: Error ! ( Normalized-Path != Normalized-Path-From-File-URL )\n" );

    return;
}

void FileBaseTest()
{
    printf( "--------------------------------------------\n" );
    printf( "FileBase-Test\n" );
    printf( "--------------------------------------------\n\n" );

    // ConvertPath-Test (Local File)

    printf( "- Local File: ");
    printFileName( file1 );
    printf( "\n\n" );

    ConvertPathTest(file1);

    // ConvertPath-Test (File on a server)

    printf( "\n- File on server: ");
    printFileName( file_on_server );
    printf( "\n\n" );
    ConvertPathTest(file_on_server);

    PressKey();

    return;
}

// AbsolutePathTest

void DoAbsolutePathTest(rtl::OUString strDirBase, rtl::OUString strRelative)
{
    FileBase::RC    rc;

    rtl::OUString       strAbsolute;

    printf( "Base-Directory: \t");
    printFileName( strDirBase );
    printf( "\n" );
    printf( "Relative-Path: \t\t");
    printFileName ( strRelative );
    printf( "\n" );

    rc=FileBase::getAbsoluteFileURL( strDirBase, strRelative, strAbsolute );

    if ( rc == FileBase::E_None )
    {
        printf( "Absolute-Path: \t\t");
        printFileName ( strAbsolute );
        printf( "\n" );
    }
    else
        printf( "Absolute-Path: Error \n" );

    printf( "\n" );
    return;
}

void AbsolutePathTest()
{
    printf( "--------------------------------------------\n" );
    printf( "AbsolutePath-Test\n" );
    printf( "--------------------------------------------\n\n" );

    DoAbsolutePathTest(dir1, rtl::OUString("."));
    DoAbsolutePathTest(dir1, rtl::OUString(".."));
    DoAbsolutePathTest(dir1, rtl::OUString("../.."));
    DoAbsolutePathTest(dir1, rtl::OUString("../HUHU"));

    DoAbsolutePathTest(dir_on_server, rtl::OUString("."));
    DoAbsolutePathTest(dir_on_server, rtl::OUString(".."));
    DoAbsolutePathTest(dir_on_server, rtl::OUString("../.."));
    DoAbsolutePathTest(dir_on_server, rtl::OUString("../HUHU"));

    PressKey();
    return;
}

// searchPathTest

void SearchPathTest()
{
    FileBase::RC    rc;

    rtl::OUString       strNormPath(file1);
    rtl::OUString       strFileURL;
    rtl::OUString       strSystemPath;
    rtl::OUString       strResultPath;

    printf( "--------------------------------------------\n" );
    printf( "SearchPath-Test\n" );
    printf( "--------------------------------------------\n\n" );

    rc=FileBase::getFileURLFromSystemPath( strNormPath, strFileURL );
    print_error( rtl::OString( "getFileURLFromSystemPath" ), rc );
    rc=FileBase::getSystemPathFromFileURL( strNormPath, strSystemPath );
    print_error( rtl::OString( "getSystemPathFromFileURL" ), rc );

    // searchFileURL (with a normalized path)

    if ( !strNormPath.isEmpty() )
    {
        printf( "\nSearch-Normalized-Path (with a normalized path) : ");
        printFileName ( strNormPath );
        printf( "\n" );

        rc=FileBase::searchFileURL( strNormPath , rtl::OUString() , strResultPath );

        if ( rc == FileBase::E_None )
        {
            printf( "Result: \t\t\t");
            printFileName ( strResultPath );
            printf( "\n" );
        }
        else
            printf( "searchFileURL (with a normalized path): Error\n" );
    }
    else
        printf( "searchFileURL (with a normalized path): not tested\n" );

    // searchFileURL (with a File-URL)

    if ( !strFileURL.isEmpty() )
    {
        printf( "\nSearch-Normalized-Path (with a FileURL) : ");
        printFileName( strFileURL );
        printf( "\n" );

        rc=FileBase::searchFileURL( strFileURL , rtl::OUString() , strResultPath );

        if ( rc == FileBase::E_None )
        {
            printf( "Result: \t\t\t");
            printFileName ( strResultPath );
            printf( "\n" );
        }
        else
            printf( "searchFileURL (with a FileURL path): Error\n" );
    }
    else
        printf( "searchFileURL (with a FileURL path): not tested\n" );

    // searchFileURL (with a systempath)

    if ( !strSystemPath.isEmpty() )
    {
        printf( "\nSearch-Normalized-Path (with a SystemPath) : ");
        printFileName( strSystemPath );
        printf( "\n" );

        rc=FileBase::searchFileURL( strSystemPath , rtl::OUString() , strResultPath );

        if ( rc == FileBase::E_None )
        {
            printf( "Result: \t\t\t");
            printFileName( strResultPath );
            printf( "\n" );
        }
        else
            printf( "searchFileURL (with a systempath): Error\n" );
    }
    else
        printf( "searchFileURL (with a systempath): not tested\n" );

    // searchFileURL (File and no searchpath)

    printf( "\nsearchFileURL:  File (no searchpath) : ");
    printFileName( file3 );
    printf( "\n" );

    rc=FileBase::searchFileURL( file3 , rtl::OUString("") , strResultPath );

    if ( rc == FileBase::E_None )
    {
        printf( "Result: \t\t\t");
        printFileName( strResultPath );
        printf( "\n" );
    }
    else
        printf( "searchFileURL:  File not found: OK ! \n" );

    // searchFileURL (File and Path)

    printf( "\nsearchFileURL:  File : ");
    printFileName( file3 );
    printf( "\tSearchPath ");
    printFileName( dir1 );
    printf( "\n");

    rc=FileBase::searchFileURL( file3 , dir1 , strResultPath );

    if ( rc == FileBase::E_None )
    {
        printf( "Result: \t\t\t");
        printFileName( strResultPath );
        printf( "\n");
    }
    else
        printf( "searchFileURL:  File not found: Error\n" );

    // searchFileURL (File and searchpath with two entries)

    rtl::OUString       strSearchPath( dir_not_exist );
    strSearchPath+=rtl::OUString(";");
    strSearchPath+=dir_on_server;

    printf( "\nsearchFileURL:  File : ");
    printFileName( file3 );
    printf( "SearchPath ");
    printFileName( strSearchPath );
    printf( "\n");

    rc=FileBase::searchFileURL( file3 , strSearchPath , strResultPath );

    if ( rc == FileBase::E_None )
    {
        printf( "Result: \t\t\t");
        printFileName( strResultPath );
        printf( "\n" );
    }
    else
        printf( "searchFileURL:  File not found: Error\n" );

    // searchFileURL (File and searchpath (with a wrong semantic))

    strSearchPath=rtl::OUString( dir_wrong_semantic );

    printf( "\nsearchFileURL:  File : ");
    printFileName( file3 );
    printf( "SearchPath ");
    printFileName( strSearchPath );
    printf( "\n");

    rc=FileBase::searchFileURL( file3 , strSearchPath , strResultPath );

    if ( rc == FileBase::E_None )
        printf( "Error: Wrong Semantich but no error occurs !\n" );
    else
        printf( "searchFileURL:  File not found: OK !\n" );

    return;
}

// CanonicalNameTest

void getCanonicalNameTest(rtl::OUString strPath)
{
    FileBase::RC    rc;

    rtl::OUString   strValid;

    printf( "Original-Name: \t\t");
    printFileName( strPath );
    printf( "\n" );

    rc=FileBase::getCanonicalName( strPath, strValid );

    if ( rc == FileBase::E_None)
    {
        printf( "Canonical-Name: \t");
        printFileName( strValid );
        printf( "\n");

    }
    else
        printf( "Canonical-Name: Error \n" );

    printf( "\n" );
    return;
}

void CanonicalNameTest()
{
    printf( "--------------------------------------------\n" );
    printf( "CanonicalName-Test\n" );
    printf( "--------------------------------------------\n\n" );

    getCanonicalNameTest( dir1 );
    getCanonicalNameTest( dir_on_server );
    getCanonicalNameTest( file1 );
    getCanonicalNameTest( file_on_server );

    PressKey();
    return;
}

// print_error

void print_error( const ::rtl::OString& str, FileBase::RC rc )
{

    printf( "%s : ", str.getStr() );
    switch(rc)
    {
    case FileBase::E_None:
        printf("OK !\n");
        break;
    case FileBase::E_PERM:
        printf("E_PERM\n");
        break;
    case FileBase::E_NOENT:
        printf("E_NOENT\n");
        break;
    case FileBase::E_SRCH:
        printf("E_SRCH\n");
        break;
    case FileBase::E_INTR:
        printf("E_INTR\n");
        break;
    case FileBase::E_IO:
        printf("E_IO\n");
        break;
    case FileBase::E_NXIO:
        printf("E_NXIO\n");
        break;
    case FileBase::E_2BIG:
        printf("E_2BIG\n");
        break;
    case FileBase::E_NOEXEC:
        printf("E_NOEXEC\n");
        break;
    case FileBase::E_BADF:
        printf("E_BADF\n");
        break;
    case FileBase::E_CHILD:
        printf("E_CHILD\n");
        break;
    case FileBase::E_AGAIN:
        printf("E_AGAIN\n");
        break;
    case FileBase::E_NOMEM:
        printf("E_NOMEM\n");
        break;
    case FileBase::E_ACCES:
        printf("E_ACCES\n");
        break;
    case FileBase::E_FAULT:
        printf("E_FAULT\n");
        break;
    case FileBase::E_BUSY:
        printf("E_BUSY\n");
        break;
    case FileBase::E_EXIST:
        printf("E_EXIST\n");
        break;
    case FileBase::E_XDEV:
        printf("E_XDEV\n");
        break;
    case FileBase::E_NODEV:
        printf("E_NODEV\n");
        break;
    case FileBase::E_NOTDIR:
        printf("E_NOTDIR\n");
        break;
    case FileBase::E_ISDIR:
        printf("E_ISDIR\n");
        break;
    case FileBase::E_INVAL:
        printf("E_INVAL\n");
        break;
    case FileBase::E_NFILE:
        printf("E_NFILE\n");
        break;
    case FileBase::E_MFILE:
        printf("E_MFILE\n");
        break;
    case FileBase::E_NOTTY:
        printf("E_NOTTY\n");
        break;
    case FileBase::E_FBIG:
        printf("E_FBIG\n");
        break;
    case FileBase::E_NOSPC:
        printf("E_NOSPC\n");
        break;
    case FileBase::E_SPIPE:
        printf("E_SPIPE\n");
        break;
    case FileBase::E_ROFS:
        printf("E_ROFS\n");
        break;
    case FileBase::E_MLINK:
        printf("E_MLINK\n");
        break;
    case FileBase::E_PIPE:
        printf("E_PIPE\n");
        break;
    case FileBase::E_DOM:
        printf("E_DOM\n");
        break;
    case FileBase::E_RANGE:
        printf("E_RANGE\n");
        break;
    case FileBase::E_DEADLK:
        printf("E_DEADLK\n");
        break;
    case FileBase::E_NAMETOOLONG:
        printf("E_NAMETOOLONG\n");
        break;
    case FileBase::E_NOLCK:
        printf("E_NOLCK\n");
        break;
    case FileBase::E_NOSYS:
        printf("E_NOSYS\n");
        break;
    case FileBase::E_NOTEMPTY:
        printf("E_NOTEMPTY\n");
        break;
    case FileBase::E_LOOP:
        printf("E_LOOP\n");
        break;
    case FileBase::E_ILSEQ:
        printf("E_ILSEQ\n");
        break;
    case FileBase::E_NOLINK:
        printf("E_NOLINK\n");
        break;
    case FileBase::E_MULTIHOP:
        printf("E_MULTIHOP\n");
        break;
    case FileBase::E_USERS:
        printf("E_USERS\n");
        break;
    case FileBase::E_OVERFLOW:
        printf("E_OVERFLOW\n");
        break;
    default:
        printf("E_Unknown\n");
        break;
    }
    return;
}

// main

#if defined(_WIN32)
        #define MAIN _cdecl main
#else
        #define MAIN main
#endif

int MAIN( int argc, char* argv[] )
{
    //Initialization
    sal_Bool fSuccess=Initialize();
    if ( !fSuccess )
    {
        printf("Error during Initialization");
        return -1;
    }

    if (argc==1)
    {
        DirectoryOpenAndCloseTest();
        DirectoryCreateAndRemoveTest();

        FileOpenAndCloseTest();
        FileCreateAndRemoveTest();
        FileWriteAndReadTest();
        FileCopyAndMoveTest();
        FileSizeTest();
        FilePointerTest();
        FileAttributesTest();
        FileTimeTest();
        DirectoryItemTest();
        DirectoryFileStatusTest();
        VolumeFileStatusTest();
        FileFileStatusTest();
        VolumeInfoTest();
        FileBaseTest();
        SearchPathTest();
        AbsolutePathTest();
        CanonicalNameTest();
    }
    // command line arguments ?
    else
    {
        int i=1;

        while (i<argc)
        {
            if (strcmp(argv[i], "doc")==0)
                DirectoryOpenAndCloseTest();
            else if (strcmp(argv[i], "dcr")==0)
                DirectoryCreateAndRemoveTest();
            else if (strcmp(argv[i], "foc")==0)
                FileOpenAndCloseTest();
            else if (strcmp(argv[i], "fcr")==0)
                FileCreateAndRemoveTest();
            else if (strcmp(argv[i], "fwr")==0)
                FileWriteAndReadTest();
            else if (strcmp(argv[i], "fcm")==0)
                FileCopyAndMoveTest();
            else if (strcmp(argv[i], "fs")==0)
                FileSizeTest();
            else if (strcmp(argv[i], "fp")==0)
                FilePointerTest();
            else if (strcmp(argv[i], "fa")==0)
                FileAttributesTest();
            else if (strcmp(argv[i], "ft")==0)
                FileTimeTest();
            else if (strcmp(argv[i], "di")==0)
                DirectoryItemTest();
            else if (strcmp(argv[i], "dfs")==0)
                DirectoryFileStatusTest();
            else if (strcmp(argv[i], "vfs")==0)
                VolumeFileStatusTest();
            else if (strcmp(argv[i], "ffs")==0)
                FileFileStatusTest();
            else if (strcmp(argv[i], "vi")==0)
                VolumeInfoTest();
            else if (strcmp(argv[i], "fb")==0)
                FileBaseTest();
            else if (strcmp(argv[i], "sp")==0)
                SearchPathTest();
            else if (strcmp(argv[i], "ap")==0)
                AbsolutePathTest();
            else if (strcmp(argv[i], "cn")==0)
                CanonicalNameTest();

            i++;
        }
    }

    // Shutdown
    fSuccess=Shutdown();
    if ( !fSuccess )
    {
        printf("Error during Shutdown");
        return -1;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
