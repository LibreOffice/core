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
#ifndef INCLUDED_RSC_RSCSFX_HXX
#define INCLUDED_RSC_RSCSFX_HXX

//========== S F X =======================================
// This is used as a flags enum in sw/, but only there,
// so I don't pull in o3tl::typed_flags here
enum class SfxStyleFamily {
    None    = 0x00,
    Char    = 0x01,
    Para    = 0x02,
    Frame   = 0x04,
    Page    = 0x08,
    Pseudo  = 0x10,
    Table   = 0x20,
    Cell    = 0x40,
    All     = 0x7fff
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
