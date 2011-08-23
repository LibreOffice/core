/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <tools/filecoll.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/ploc_dir.hxx>

#include <stdio.h>


FileCollector::FileCollector( uintt i_nRoughNrOfFiles )
    // :    aFoundFiles
{
    if (i_nRoughNrOfFiles > 0)
        aFoundFiles.reserve(i_nRoughNrOfFiles);
}

uintt
FileCollector::AddFilesFrom( const char *                 i_sRootDir,
                             const char *				  i_sFilter,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
