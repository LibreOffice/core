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




// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_l10ntools.hxx"
#include <l10ntools/file.hxx>

namespace transex
{

File::File( const rtl::OUString sFile )
{
    sFileName = sFile;
}

File::File( const rtl::OUString sFullPath , const rtl::OUString sFile )
{
    sFileName = sFile;
    sFullName = sFullPath;
}

bool File::lessFile ( const File& rKey1, const File& rKey2 )
{
    rtl::OUString sName1( ( static_cast< File >( rKey1 ) ).getFileName() );
    rtl::OUString sName2( ( static_cast< File >( rKey2 ) ).getFileName() );

    return sName1.compareTo( sName2 ) < 0 ;
}

}
