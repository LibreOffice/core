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

#include "oox/token/namespacemap.hxx"

namespace oox {



NamespaceMap::NamespaceMap()
{
    static const struct NamespaceUrl { sal_Int32 mnId; const sal_Char* mpcUrl; } spNamespaceUrls[] =
    {
// include auto-generated C array with namespace URLs as C strings
#include "namespacenames.inc"
        { -1, "" }
    };

    static const struct NamespaceStrictUrl { sal_Int32 mnId; const sal_Char* mpcUrl; } spNamespaceStrictUrls[] =
    {
// include auto-generated C array with namespace URLs as C strings
#include "namespace-strictnames.inc"
        { -1, "" }
    };

    for( const NamespaceUrl* pNamespaceUrl = spNamespaceUrls; pNamespaceUrl->mnId != -1; ++pNamespaceUrl )
        maTransitionalNamespaceMap[ pNamespaceUrl->mnId ] = OUString::createFromAscii( pNamespaceUrl->mpcUrl );

    for( const NamespaceStrictUrl* pNamespaceUrl = spNamespaceStrictUrls; pNamespaceUrl->mnId != -1; ++pNamespaceUrl )
        maStrictNamespaceMap[ pNamespaceUrl->mnId ] = OUString::createFromAscii( pNamespaceUrl->mpcUrl );
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
