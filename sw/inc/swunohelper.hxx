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


#ifndef _SWUNOHELPER_HXX
#define _SWUNOHELPER_HXX

#include <tools/solar.h>
#include <sal/types.h>
#include "swdllapi.h"

namespace com { namespace sun { namespace star {
    namespace uno {
        class Any;
    }
}}}

class String;
class SvStrings;
class SvPtrarr;

//UUUU
class SfxItemSet;

namespace SWUnoHelper {

// calls over the compherl the getEnumAsInt32 function and handle the
// exceptions.
sal_Int32 GetEnumAsInt32( const com::sun::star::uno::Any& rVal );


// methods for UCB actions:
    // delete the file under this URL
SW_DLLPUBLIC sal_Bool UCB_DeleteFile( const String& rURL );

    // copy/move the file to a new location
sal_Bool UCB_CopyFile( const String& rURL, const String& rNewURL,
                    sal_Bool bCopyIsMove = sal_False );

    // is the URL on the current system case sentive?
SW_DLLPUBLIC sal_Bool UCB_IsCaseSensitiveFileName( const String& rURL );

    // is the URL readonly?
SW_DLLPUBLIC sal_Bool UCB_IsReadOnlyFileName( const String& rURL );

    // get a list of files from the folder of the URL
    // options: pExtension = 0 -> all, else this specific extension
    //          pDateTime != 0 -> returns also the modified date/time of
    //                       the files in a SvPtrarr -->
    //                       !! objects must be deleted from the caller!!
sal_Bool UCB_GetFileListOfFolder( const String& rURL, SvStrings& rList,
                                const String* pExtension = 0,
                                SvPtrarr* pDateTimeList = 0 );

    // is the URL an existing file?
SW_DLLPUBLIC sal_Bool UCB_IsFile( const String& rURL );

    // is the URL a existing directory?
sal_Bool UCB_IsDirectory( const String& rURL );

///UUUU helper to check if fill style is set to color or bitmap
/// and thus formally used SvxBrushItem parts need to be mapped
/// for backwards compatibility
bool needToMapFillItemsToSvxBrushItemTypes(const SfxItemSet& rSet);

}

#endif
