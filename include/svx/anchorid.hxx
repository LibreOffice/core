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
#ifndef _SVX_ANCHORID_HXX
#define _SVX_ANCHORID_HXX


enum SvxAnchorIds
{
    SVX_OBJ_AT_CNTNT    = 0x01,     // Frame bound to paragraph
    SVX_OBJ_IN_CNTNT    = 0x02,     // Frame bound to character
    SVX_OBJ_PAGE        = 0x04,     // Frame bound to page
    SVX_OBJ_AT_FLY      = 0x08      // Frame bound to other frame
//  SVX_OBJ_AUTO_CNTNT  = 0x10      // Automatically positioned frame bound to paragraph
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
