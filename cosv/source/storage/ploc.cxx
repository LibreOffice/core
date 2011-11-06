/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
