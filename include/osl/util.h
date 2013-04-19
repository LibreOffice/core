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


#ifndef _OSL_UTIL_H_
#define _OSL_UTIL_H_

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
    @param pEthernetAddr 6 bytes of memory

    @return sal_True  if the ethernetaddress could be retrieved. <br>
            sal_False if no address could be found. This may be either because
           there is no ethernet card or there is no appropriate algorithm
           implemented on the platform. In this case, pEthernetAddr is
           unchanged.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getEthernetAddress( sal_uInt8 *pEthernetAddr );

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
