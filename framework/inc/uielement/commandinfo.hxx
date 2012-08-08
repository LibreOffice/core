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

#ifndef __FRAMEWORK_UIELEMENT_COMMANDINFO_HXX_
#define __FRAMEWORK_UIELEMENT_COMMANDINFO_HXX_

#include <macros/generic.hxx>
#include <stdtypes.h>

#include <tools/solar.h>

namespace framework
{

struct CommandInfo
{
    CommandInfo() : nId( 0 ),
                    nWidth( 0 ),
                    nImageInfo( 0 ),
                    bMirrored( false ),
                    bRotated( false ) {}

    sal_uInt16                  nId;
    sal_uInt16                  nWidth;
    ::std::vector< sal_uInt16 > aIds;
    sal_Int16               nImageInfo;
    sal_Bool                bMirrored : 1,
                            bRotated  : 1;
};

typedef BaseHash< CommandInfo > CommandToInfoMap;

} // namespace framework

#endif // __FRAMEWORK_UIELEMENT_COMMANDINFO_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
