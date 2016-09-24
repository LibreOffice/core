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

#ifndef INCLUDED_VCL_OSX_A11YTABLEWRAPPER_H
#define INCLUDED_VCL_OSX_A11YTABLEWRAPPER_H

#include "osx/a11ywrapper.h"

#define MAXIMUM_ACCESSIBLE_TABLE_CELLS 1000

@interface AquaA11yTableWrapper : AquaA11yWrapper
{
}
+(id)childrenAttributeForElement:(AquaA11yTableWrapper *)wrapper;
+(void)addAttributeNamesTo: (NSMutableArray *)attributeNames object: (AquaA11yWrapper*)pObject;

-(id)rowsAttribute;
-(id)columnsAttribute;
@end
#endif // INCLUDED_VCL_OSX_A11YTABLEWRAPPER_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
