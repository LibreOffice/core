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

#include <types.hxx>
#include <osl/diagnose.h>


sal_Unicode ConvertMathPrivateUseAreaToUnicode( sal_Unicode cChar )
{
    sal_Unicode cRes = cChar;
    if (IsInPrivateUseArea( cChar ))
    {
        OSL_FAIL( "Error: private use area characters should no longer be in use!" );
        cRes = (sal_Unicode) '@'; // just some character that should easily be notice as odd in the context
    }
    return cRes;
}


sal_Unicode ConvertMathToMathML( sal_Unicode cChar )
{
    sal_Unicode cRes = ConvertMathPrivateUseAreaToUnicode( cChar );
    return cRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
