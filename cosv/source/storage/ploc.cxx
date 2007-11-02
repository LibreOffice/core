/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ploc.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:46:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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
