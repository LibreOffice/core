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
#ifndef _SWUNOHELPER_HXX
#define _SWUNOHELPER_HXX

#include <tools/solar.h>
#include <sal/types.h>
#include "swdllapi.h"

#include <vector>

namespace com { namespace sun { namespace star {
    namespace uno {
        class Any;
    }
}}}

namespace rtl {class OUString;}

class String;
class DateTime;

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
    //                       the files in a vector -->
    //                       !! objects must be deleted from the caller!!
bool UCB_GetFileListOfFolder( const OUString& rURL,
                                std::vector<OUString*>& rList,
                                const OUString* pExtension = 0,
                                std::vector<DateTime*>* pDateTimeList = 0 );

    // is the URL an existing file?
SW_DLLPUBLIC sal_Bool UCB_IsFile( const String& rURL );

    // is the URL a existing directory?
sal_Bool UCB_IsDirectory( const String& rURL );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
