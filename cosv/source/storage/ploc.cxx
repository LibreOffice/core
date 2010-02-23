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

#include <precomp.h>
#include <cosv/ploc.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/bstream.hxx>




namespace csv
{
namespace ploc
{


Path::Path( const char *        i_sPath,
            bool                i_bPathIsAlwaysDir,
            const char *        i_sDelimiter        )
    :   pRoot(0)
        // aPath,
        // sFile
{
    Set(i_sPath, i_bPathIsAlwaysDir, i_sDelimiter );
}

Path::Path( const Path & i_rPath )
    :   pRoot(i_rPath.pRoot->CreateCopy()),
        aPath(i_rPath.aPath),
        sFile(i_rPath.sFile)
{
}

Path::~Path()
{
}

Path &
Path::operator=( const Path & i_rPath )
{
    pRoot = i_rPath.pRoot->CreateCopy();
    aPath = i_rPath.aPath;
    sFile = i_rPath.sFile;
    return *this;
}


void
Path::Set( const char *        i_sPath,
           bool                i_bPathIsAlwaysDir,
           const char *        i_sDelimiter        )
{
    if ( *i_sDelimiter != '\\' AND *i_sDelimiter != '/' )
        return;

    const char *
        restPath = 0;
    pRoot = Root::Create_( restPath, i_sPath, i_sDelimiter );
    if (restPath == 0)
        return;

    aPath.Set(restPath, i_bPathIsAlwaysDir, i_sDelimiter);

    if (NOT i_bPathIsAlwaysDir)
    {
        const char *
            file = strrchr( restPath, *i_sDelimiter );
        if (file == 0)
            file = restPath;
        else
            file++;
        sFile = file;
    }
}

void
Path::SetFile( const String & i_sName )
{
    sFile = i_sName;
}

const char *
Path::FileExtension() const
{
    const char *
        ext = strrchr(sFile, '.');
    if (ext != 0)
        ++ext;
    else
        ext = "";
    return ext;
}

bool
Path::IsValid() const
{
    return RootDir().OwnDelimiter() != 0;
}

void
Path::Get( ostream & o_rPath ) const
{
    if (NOT IsValid())
        return;

    pRoot->Get( o_rPath );
    aPath.Get( o_rPath, pRoot->OwnDelimiter() );

    if ( sFile.length() > 0 )
        o_rPath << sFile;

}

void
Path::Get( bostream & o_rPath ) const
{
    if (NOT IsValid())
        return;

    pRoot->Get( o_rPath );
    aPath.Get( o_rPath, pRoot->OwnDelimiter() );

    if ( sFile.length() > 0 )
        o_rPath.write( sFile );
}




} // namespace ploc
} // namespace csv
