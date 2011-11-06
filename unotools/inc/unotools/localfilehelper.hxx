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


#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#define _UNOTOOLS_LOCALFILEHELPER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include "unotools/unotoolsdllapi.h"

#include <rtl/ustring.hxx>
#include <tools/string.hxx>

namespace utl
{

class UNOTOOLS_DLLPUBLIC LocalFileHelper
{
public:
                    /**
                    Converts a "physical" file name into a "UCB compatible" URL ( if possible ).
                    If no UCP is available for the local file system, sal_False and an empty URL is returned.
                    Returning sal_True and an empty URL means that the URL doesn't point to a local file.
                    */
    static sal_Bool ConvertPhysicalNameToURL( const String& rName, String& rReturn );
    static sal_Bool ConvertSystemPathToURL( const String& rName, const String& rBaseURL, String& rReturn );

                    /**
                    Converts a "UCB compatible" URL into a "physical" file name.
                    If no UCP is available for the local file system, sal_False and an empty file name is returned,
                    otherwise sal_True and a valid URL, because a file name can always be converted if a UCP for the local
                    file system is present ( watch: this doesn't mean that this file really exists! )
                    */
    static sal_Bool ConvertURLToPhysicalName( const String& rName, String& rReturn );
    static sal_Bool ConvertURLToSystemPath( const String& rName, String& rReturn );

    static sal_Bool IsLocalFile( const String& rName );
    static sal_Bool IsFileContent( const String& rName );

    static          ::com::sun::star::uno::Sequence< ::rtl::OUString >
                            GetFolderContents( const ::rtl::OUString& rFolder, sal_Bool bFolder );
};

}

#endif
