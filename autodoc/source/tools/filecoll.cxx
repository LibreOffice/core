/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filecoll.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:12:43 $
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
#include <tools/filecoll.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/ploc_dir.hxx>


FileCollector::FileCollector( uintt i_nRoughNrOfFiles )
    // :    aFoundFiles
{
    if (i_nRoughNrOfFiles > 0)
        aFoundFiles.reserve(i_nRoughNrOfFiles);
}

uintt
FileCollector::AddFilesFrom( const char *                 i_sRootDir,
                             const char *                 i_sFilter,
                             E_SearchMode                 i_eSearchMode )
{
    uintt nSizeAtStart = aFoundFiles.size();

    if (csv::no_str(i_sFilter) OR csv::no_str(i_sRootDir))
    {
        Cout() << "Warning: The filter contains no files." <<  Endl();
        return 0;
    }

    csv::ploc::Directory aDir(i_sRootDir);
    if (NOT aDir.Exists())
    {
        Cerr() << "Warning: The path for the files to be parsed could not be found:\n"
               << i_sRootDir
               << Endl();
        return 0;
    }

    Cout() << "." << Flush();
    aDir.GetContainedFiles(aFoundFiles, i_sFilter);

    if (i_eSearchMode == recursive)
    {
        StreamStr aPath(1020);
        aPath << i_sRootDir << csv::ploc::Delimiter();
        uintt nSubDirStart = aPath.tellp();

        StringVector aSubDirs;
        aDir.GetContainedDirectories(aSubDirs);

        for ( const_iterator iter = aSubDirs.begin();
              iter != aSubDirs.end();
              ++iter )
        {
            aPath.seekp(nSubDirStart);
            aPath << (*iter);
            AddFilesFrom( aPath.c_str(), i_sFilter, i_eSearchMode );
        }
    }

    return aFoundFiles.size() - nSizeAtStart;
}

uintt
FileCollector::AddFile( const char * i_sFilePath )
{
    FILE * pFile = fopen( i_sFilePath, "r" );
    if ( pFile == 0 )
    {
        Cerr() << "Warning: The path for the file to be parsed could not be found:\n"
               << i_sFilePath
               << Endl();
        return 0;
    }

    fclose(pFile);
    aFoundFiles.push_back(i_sFilePath);
    return 1;
}

void
FileCollector::EraseAll()
{
    csv::erase_container(aFoundFiles);
}

FileCollector::const_iterator
FileCollector::Begin() const
{
    return aFoundFiles.begin();
}

FileCollector::const_iterator
FileCollector::End() const
{
    return aFoundFiles.end();
}

uintt
FileCollector::Size() const
{
    return aFoundFiles.size();
}

