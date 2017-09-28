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

#include <stdio.h>

#include "codemaker/global.hxx"
#include "rtl/ustring.hxx"
#include "rtl/process.h"
#include "options.hxx"

namespace unodevtools {


bool readOption( OUString * pValue, const sal_Char * pOpt,
                     sal_uInt32 * pnIndex, const OUString & aArg)
{
    const OUString dash = "-";
    if(aArg.indexOf(dash) != 0)
        return false;

    OUString aOpt = OUString::createFromAscii( pOpt );

    if (aArg.getLength() < aOpt.getLength())
        return false;

    if (aOpt.equalsIgnoreAsciiCase( aArg.copy(1) )) {
        // take next argument
        ++(*pnIndex);

        rtl_getAppCommandArg(*pnIndex, &pValue->pData);
        if (*pnIndex >= rtl_getAppCommandArgCount() ||
            pValue->copy(1) == dash)
        {
            throw CannotDumpException(
                "incomplete option \"-" + aOpt + "\" given!");
        }
        SAL_INFO("unodevtools", "identified option -" << pOpt << " = " << *pValue);
        ++(*pnIndex);
        return true;
    } else if (aArg.indexOf(aOpt) == 1) {
        *pValue = aArg.copy(1 + aOpt.getLength());
        SAL_INFO("unodevtools", "identified option -" << pOpt << " = " << *pValue);
        ++(*pnIndex);

        return true;
    }
    return false;
}


bool readOption( const sal_Char * pOpt,
                     sal_uInt32 * pnIndex, const OUString & aArg)
{
    OUString aOpt = OUString::createFromAscii(pOpt);

    if((aArg.startsWith("-") && aOpt.equalsIgnoreAsciiCase(aArg.copy(1))) ||
       (aArg.startsWith("--") && aOpt.equalsIgnoreAsciiCase(aArg.copy(2))) )
    {
        ++(*pnIndex);
        SAL_INFO("unodevtools", "identified option --" << pOpt);
        return true;
    }
    return false;
}

} // end of namespace unodevtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
