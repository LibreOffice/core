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

#include <vector>
#include <algorithm>
#include <rtl/ustring.hxx>

#ifdef WNT
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <stdio.h>

#ifndef L10NTOOLS_FILE_HXX
#define L10NTOOLS_FILE_HXX
#include <l10ntools/file.hxx>
#endif

namespace transex{

class Directory
{
    private:
    rtl::OUString sDirectoryName;
    rtl::OUString sFullName;

    std::vector<Directory>  aDirVec;
    std::vector<File>       aFileVec;

    public:
    std::vector<Directory>  getSubDirectories()  { return aDirVec;        }
    std::vector<File>       getFiles()           { return aFileVec;       }

    void readDirectory( const rtl::OUString& sFullpath );
    void scanSubDir( int nLevels = 0 );

    rtl::OUString getDirectoryName()            { return sDirectoryName; }
    rtl::OUString getFullName()                 { return sFullName ;     }

    void dump();
    Directory(){}

    Directory( const rtl::OUString sFullPath , const rtl::OUString sEntry ) ;

    static bool lessDir ( const Directory& rKey1, const Directory& rKey2 ) ;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
