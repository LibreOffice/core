/*************************************************************************
 *
 *  $RCSfile: ploc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:25:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <precomp.h>
#include <cosv/ploc.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/bstream.hxx>
// #include <ctype.h>


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

    const char * pRestPath = 0;
    pRoot = Root::Create_( pRestPath, i_sPath, i_sDelimiter );
    if (pRestPath == 0)
        return;

    aPath.Set(pRestPath, i_bPathIsAlwaysDir, i_sDelimiter);

    if (NOT i_bPathIsAlwaysDir)
    {
        pRestPath = strrchr( pRestPath, *i_sDelimiter );
        if ( NOT no_str(pRestPath) )
            sFile = pRestPath + 1;
    }
}

void
Path::SetFile( const String & i_sName )
{
    sFile = i_sName;
}

const char *
Path::FileEnding() const
{
    const char * pEnd = strrchr(sFile, '.');
    if (pEnd != 0)
        ++pEnd;
    else
        pEnd = "";
    return pEnd;
}

bool
Path::IsValid() const         { return RootDir().OwnDelimiter() != 0; }

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



