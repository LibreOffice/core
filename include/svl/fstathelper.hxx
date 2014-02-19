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

#ifndef INCLUDED_SVL_FSTATHELPER_HXX
#define INCLUDED_SVL_FSTATHELPER_HXX

#include <rtl/ustring.hxx>
#include <svl/svldllapi.h>

class Date;
class Time;

namespace FStatHelper {

/** Return the modified time and date stamp for this URL.

    @param URL  the asking URL

    @param pDate if unequal 0, the function set the date stamp

    @param pTime if unequal 0, the function set the time stamp

    @return     it was be able to get the date/time stamp
*/
SVL_DLLPUBLIC bool GetModifiedDateTimeOfFile( const OUString& rURL,
                                    Date* pDate, Time* pTime );

/** Return if under the URL a document exist. This is only a wrapper for the
    UCB.IsContent.
*/
SVL_DLLPUBLIC bool IsDocument( const OUString& rURL );

/** Return if under the URL a folder exist. This is only a wrapper for the
    UCB.isFolder.
*/
SVL_DLLPUBLIC bool IsFolder( const OUString& rURL );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
