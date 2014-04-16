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

#ifndef _CONNECTIVITY_MAB_NS_DECLARES_HXX_
#define _CONNECTIVITY_MAB_NS_DECLARES_HXX_

#include <sal/types.h>


const   sal_Int32 RowStates_Normal = 0;
const   sal_Int32 RowStates_Inserted = 1;
const   sal_Int32 RowStates_Updated = 2;
const   sal_Int32 RowStates_Deleted  = 4;
const   sal_Int32 RowStates_Error  = 32;

namespace connectivity{
    namespace mozab{
        class OConnection;
    }
}

bool isProfileLocked(connectivity::mozab::OConnection* _pCon);

class nsIAbDirectory;
sal_Int32 getDirectoryType(const nsIAbDirectory*  directory);


#endif // _CONNECTIVITY_MAB_NS_DECLARES_HXX_ 1

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
