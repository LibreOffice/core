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

#include <tools/tempfile.hxx>
#include "comdep.hxx"

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>
#include <rtl/instance.hxx>
#include <tools/time.hxx>
#include <tools/debug.hxx>

#include <stdio.h>

#ifdef UNX
#define _MAX_PATH 260
#endif

using namespace osl;

namespace { struct TempNameBase_Impl : public rtl::Static< ::rtl::OUString, TempNameBase_Impl > {}; }

struct TempFile_Impl
{
    String      aName;
};

extern rtl::OUString GetSystemTempDirPath_Impl();

rtl::OUString GetSystemTempDirPath_Impl()
{
    rtl::OUString aTmpURL, aPath;
    osl::FileBase::getTempDirURL( aTmpURL );
    osl::FileBase::getSystemPathFromFileURL( aTmpURL, aPath );
    return aPath;
}

#define TMPNAME_SIZE  ( 1 + 5 + 5 + 4 + 1 )

OUString ConstructTempDir_Impl()
{
    // use system directory
    ::rtl::OUString& rTempNameBase_Impl = TempNameBase_Impl::get();
    if ( rTempNameBase_Impl.isEmpty() )
        rTempNameBase_Impl = GetSystemTempDirPath_Impl();
    OUString aName = rTempNameBase_Impl;

    // Make sure that directory ends with a separator
    if( !aName.endsWith( "/" ) )
        aName += "/";

    return aName;
}

void CreateTempName_Impl( String& rName, sal_Bool bKeep, sal_Bool bDir = sal_True )
{
    // add a suitable tempname
    // Prefix can have 5 chars, leaving 3 for numbers. 26 ** 3 == 17576
    // ER 13.07.00  why not radix 36 [0-9A-Z] ?!?
    const unsigned nRadix = 26;
    String aName( rName );
    aName += rtl::OUString("sv");

    rName.Erase();
    static unsigned long u = Time::GetSystemTicks();
    for ( unsigned long nOld = u; ++u != nOld; )
    {
        u %= (nRadix*nRadix*nRadix);
        rtl::OUString aTmp = rtl::OUStringBuffer(aName).
            append((sal_Int32)(unsigned)u, nRadix).
            append(".tmp").
            makeStringAndClear();

        if ( bDir )
        {
            FileBase::RC err = Directory::create( aTmp );
            if (  err == FileBase::E_None )
            {
                // !bKeep: only for creating a name, not a file or directory
                if ( bKeep || Directory::remove( aTmp ) == FileBase::E_None )
                    rName = aTmp;
                break;
            }
            else if ( err != FileBase::E_EXIST )
            {
                // if f.e. name contains invalid chars stop trying to create dirs
                break;
            }
        }
        else
        {
            DBG_ASSERT( bKeep, "Too expensive, use directory for creating name!" );
            File aFile( aTmp );
            FileBase::RC err = aFile.open(osl_File_OpenFlag_Create);
            if (  err == FileBase::E_None )
            {
                rName = aTmp;
                aFile.close();
                break;
            }
            else if ( err != FileBase::E_EXIST )
            {
                 // if f.e. name contains invalid chars stop trying to create files
                 break;
            }
        }
    }
}

String TempFile::CreateTempName()
{
    // get correct directory
    String aName = ConstructTempDir_Impl();

    // get TempFile name with default naming scheme
    CreateTempName_Impl( aName, sal_False );

    return aName;
}

TempFile::TempFile()
    : pImp( new TempFile_Impl )
    , bKillingFileEnabled( sal_False )
{
    // get correct directory
    pImp->aName = ConstructTempDir_Impl();

    // get TempFile with default naming scheme
    CreateTempName_Impl( pImp->aName, sal_True );
}

TempFile::TempFile( const String& rLeadingChars, const String* pExtension )
    : pImp( new TempFile_Impl )
    , bKillingFileEnabled( sal_False )
{
    // get correct directory
    String aName = ConstructTempDir_Impl();

    // now use special naming scheme ( name takes leading chars and an index counting up from zero
    aName += rLeadingChars;
    for ( sal_Int32 i=0;; i++ )
    {
        rtl::OUStringBuffer aTmpBuffer(aName);
        aTmpBuffer.append(i);
        if ( pExtension )
            aTmpBuffer.append(*pExtension);
        else
            aTmpBuffer.append(".tmp");
        rtl::OUString aTmp = aTmpBuffer.makeStringAndClear();

        File aFile( aTmp );
        FileBase::RC err = aFile.open(osl_File_OpenFlag_Create);
        if ( err == FileBase::E_None )
        {
            pImp->aName = aTmp;
            aFile.close();
            break;
        }
        else if ( err != FileBase::E_EXIST )
            // if f.e. name contains invalid chars stop trying to create dirs
            break;
    }
}

TempFile::~TempFile()
{
    if ( bKillingFileEnabled )
    {
        File::remove( pImp->aName );
    }

    delete pImp;
}

String TempFile::GetName() const
{
    rtl::OUString aTmp;
    aTmp = pImp->aName;
    return aTmp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
