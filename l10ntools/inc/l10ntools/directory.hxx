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

#ifndef L10NTOOLS_DIRECTORY_HXX
#define L10NTOOLS_DIRECTORY_HXX

#include <vector>
#include <algorithm>
#include <rtl/ustring.hxx>
#include <tools/string.hxx>

#ifdef WNT
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <stdio.h>

#include <l10ntools/file.hxx>

namespace transex{

class Directory
{
    private:
    rtl::OUString sDirectoryName;
    rtl::OUString sFullName;
    bool bSkipLinks;

    std::vector<Directory>  aDirVec;
    std::vector<File>       aFileVec;

    public:
    std::vector<Directory>  getSubDirectories()  { return aDirVec;        }
    std::vector<File>       getFiles()           { return aFileVec;       }

    void readDirectory();
    void readDirectory( const rtl::OUString& sFullpath );
    void scanSubDir( int nLevels = 0 );

    rtl::OUString getDirectoryName()            { return sDirectoryName; }
    rtl::OUString getFullName()                 { return sFullName ;     }
    void setSkipLinks( bool is_skipped );

    void dump();
    Directory(){};

    Directory( const rtl::OUString sFullPath );
    Directory( const rtl::OUString sFullPath , const rtl::OUString sEntry ) ;
    Directory( const ByteString sFullPath );

    static bool lessDir ( const Directory& rKey1, const Directory& rKey2 ) ;
};

}

#endif // L10NTOOLS_DIRECTORY_HXX

