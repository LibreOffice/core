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


#ifndef _TOOLS_TEMPFILE_HXX
#define _TOOLS_TEMPFILE_HXX

#include <tools/string.hxx>
#include "tools/toolsdllapi.h"

struct TempFile_Impl;
class TOOLS_DLLPUBLIC TempFile
{
    TempFile_Impl*  pImp;
    sal_Bool        bKillingFileEnabled;

public:
                    // Create a temporary file or directory in a given folder or the default tempfile folder
                    TempFile( const String* pParent=NULL, sal_Bool bDirectory=sal_False );

                    // Create a temporary file or directory in a given folder or the default tempfile folder; its name starts
                    // with some given characters followed by a counter ( example: rLeadingChars="abc" means "abc0","abc1"
                    // and so on, depending on existing files in that folder ).
                    // The extension string may be f.e. ".txt" or "", if no extension string is given, ".tmp" is used
                    TempFile( const String& rLeadingChars, const String* pExtension=NULL, const String* pParent=NULL, sal_Bool bDirectory=sal_False );

                    // TempFile will be removed from disk in dtor if EnableKillingTempFile was called before.
                    // TempDirs will be removed recursively in that case.
                    ~TempFile();

    sal_Bool        IsValid() const;

                    // Returns the real name of the tempfile in file URL scheme.
    String          GetName() const;

                    // If enabled the file will be removed from disk when the dtor is called ( default is not enabled )
    void            EnableKillingFile( sal_Bool bEnable=sal_True )
                    { bKillingFileEnabled = bEnable; }

    sal_Bool        IsKillingFileEnabled() const
                    { return bKillingFileEnabled; }

                    // Only create a name for a temporary file that would be valid at that moment.
    static String   CreateTempName( const String* pParent=NULL );

                    // The TempNameBase is a folder in the default ( system ) tempfile folder.
                    // This subfolder will be used if a TempFile or TempName is created without a parent name.
                    // The caller of the SetTempNameBase is responsible for deleting this folder and all temporary files in it.
                    // The argument must be a simple name, not a complete URL.
                    // The return value of both methods is the complete URL of the tempname base folder.
    static String   SetTempNameBaseDirectory( const String &rBaseName );
    static String   GetTempNameBaseDirectory();
};

#endif
