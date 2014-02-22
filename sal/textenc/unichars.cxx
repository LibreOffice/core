/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "sal/config.h"
#include "sal/types.h"
#include "unichars.hxx"

bool ImplIsControlOrFormat(sal_uInt32 nUtf32)
{
    
    
    
    
    return nUtf32 <= 0x001F
           || (nUtf32 >= 0x007F && nUtf32 <= 0x009F)
           || nUtf32 == 0x070F 
           || nUtf32 == 0x180B 
           || nUtf32 == 0x180C 
           || nUtf32 == 0x180D 
           || nUtf32 == 0x180E 
           || nUtf32 == 0x200C 
           || nUtf32 == 0x200D 
           || nUtf32 == 0x200E 
           || nUtf32 == 0x200F 
           || nUtf32 == 0x202A 
           || nUtf32 == 0x202B 
           || nUtf32 == 0x202C 
           || nUtf32 == 0x202D 
           || nUtf32 == 0x202E 
           || nUtf32 == 0x206A 
           || nUtf32 == 0x206B 
           || nUtf32 == 0x206C 
           || nUtf32 == 0x206D 
           || nUtf32 == 0x206E 
           || nUtf32 == 0x206F 
           || nUtf32 == 0xFEFF 
           || nUtf32 == 0xFFF9 
           || nUtf32 == 0xFFFA 
           || nUtf32 == 0xFFFB 
           || nUtf32 == 0x1D173 
           || nUtf32 == 0x1D174 
           || nUtf32 == 0x1D175 
           || nUtf32 == 0x1D176 
           || nUtf32 == 0x1D177 
           || nUtf32 == 0x1D178 
           || nUtf32 == 0x1D179 
           || nUtf32 == 0x1D17A 
           || nUtf32 == 0xE0001 
           || (nUtf32 >= 0xE0020 && nUtf32 <= 0xE007F);
}

bool ImplIsPrivateUse(sal_uInt32 nUtf32)
{
    
    
    
    return (nUtf32 >= 0xE000 && nUtf32 <= 0xF8FF)
           || (nUtf32 >= 0xF0000 && nUtf32 <= 0xFFFFD)
           || (nUtf32 >= 0x100000 && nUtf32 <= 0x10FFFD);
}

bool ImplIsZeroWidth(sal_uInt32 nUtf32)
{
    
    
    
    return nUtf32 == 0x200B 
           || nUtf32 == 0x200C 
           || nUtf32 == 0x200D 
           || nUtf32 == 0xFEFF; 
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
