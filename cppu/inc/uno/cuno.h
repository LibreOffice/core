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
#ifndef _UNO_CUNO_H_
#define _UNO_CUNO_H_

#include <sal/types.h>

#define CUNO_ERROR_NONE                 0
#define CUNO_ERROR_CALL_FAILED          (1 << 31)
#define CUNO_ERROR_EXCEPTION            (1 | CUNO_ERROR_CALL_FAILED)

/** macro to call on a C interface

    @param interface_pointer interface pointer
*/
#define CUNO_CALL( interface_pointer ) (*interface_pointer)
/** macro to test if an exception was signalled.

    @param return_code return code of call
*/
#define CUNO_EXCEPTION_OCCURRED( return_code ) (0 != ((return_code) & CUNO_ERROR_EXCEPTION))

typedef sal_Int32 cuno_ErrorCode;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
