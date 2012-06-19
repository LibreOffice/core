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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
