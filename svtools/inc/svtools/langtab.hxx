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


#ifndef _SVTOOLS_LANGTAB_HXX_
#define _SVTOOLS_LANGTAB_HXX_


#include <tools/resary.hxx>
#include <i18npool/lang.h>
#include <svtools/svtdllapi.h>

//========================================================================
//  class SvtLanguageTable
//========================================================================

class SVT_DLLPUBLIC SvtLanguageTable : public ResStringArray
{
public:
    SvtLanguageTable();
    ~SvtLanguageTable();

    const String&   GetString( const LanguageType eType ) const;
    LanguageType    GetType( const String& rStr ) const;

    sal_uInt32      GetEntryCount() const;
    LanguageType    GetTypeAtIndex( sal_uInt32 nIndex ) const;
    static String   GetLanguageString( const LanguageType eType );
};

// Add LRE or RLE embedding characters to the string based on the
// String content (see #i78466#, #i32179#)
SVT_DLLPUBLIC const String ApplyLreOrRleEmbedding( const String &rText );

#endif

