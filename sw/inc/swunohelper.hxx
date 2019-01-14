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
#ifndef INCLUDED_SW_INC_SWUNOHELPER_HXX
#define INCLUDED_SW_INC_SWUNOHELPER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include "swdllapi.h"

#include <vector>

namespace com { namespace sun { namespace star {
    namespace uno {
        class Any;
    }
}}}
class DateTime;
class SfxItemSet;

namespace SWUnoHelper {

// calls over the compherl the getEnumAsInt32 function and handle the
// exceptions.
sal_Int32 GetEnumAsInt32( const css::uno::Any& rVal );

// methods for UCB actions:
    // delete the file under this URL
SW_DLLPUBLIC bool UCB_DeleteFile( const OUString& rURL );

    // move the file to a new location
bool UCB_MoveFile( const OUString& rURL, const OUString& rNewURL );

    // is the URL on the current system case sensitive?
SW_DLLPUBLIC bool UCB_IsCaseSensitiveFileName( const OUString& rURL );

    // is the URL readonly?
SW_DLLPUBLIC bool UCB_IsReadOnlyFileName( const OUString& rURL );

    // get a list of files from the folder of the URL
    // options: pExtension = 0 -> all, else this specific extension
    //          pDateTime != 0 -> returns also the modified date/time of
    //                       the files in a vector -->
bool UCB_GetFileListOfFolder( const OUString& rURL,
                                std::vector<OUString>& rList,
                                const OUString* pExtension,
                                std::vector<DateTime>* pDateTimeList = nullptr );

    // is the URL an existing file?
SW_DLLPUBLIC bool UCB_IsFile( const OUString& rURL );

    // is the URL a existing directory?
bool UCB_IsDirectory( const OUString& rURL );

/// helper to check if fill style is set to color or bitmap
/// and thus formerly used SvxBrushItem parts need to be mapped
/// for backwards compatibility
bool needToMapFillItemsToSvxBrushItemTypes(const SfxItemSet& rSet, sal_uInt16 const nMemberId);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
