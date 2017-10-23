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
#ifndef INCLUDED_UNO_CURRENT_CONTEXT_H
#define INCLUDED_UNO_CURRENT_CONTEXT_H

#include "cppu/cppudllapi.h"
#include "rtl/ustring.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** Gets the current task's context.
    @attention
    Don't spread the returned interface around to other threads.  Every thread has its own
    current context.

    @param ppCurrentContext  inout param current context of type com.sun.star.uno.XCurrentContext
    @param pEnvDcp           descriptor of returned interface's environment
    @param pEnvContext       context of returned interface's environment (commonly 0)
    @return                  true, if context ref was transferred (even if null ref)
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL uno_getCurrentContext(
    void ** ppCurrentContext,
    rtl_uString * pEnvDcp, void * pEnvContext )
    SAL_THROW_EXTERN_C();

/** Sets the current task's context.

    @param pCurrentContext  in param current context of type com.sun.star.uno.XCurrentContext
    @param pEnvDcp          descriptor of interface's environment
    @param pEnvContext      context of interface's environment (commonly 0)
    @return                 true, if context ref was transferred (even if null ref)
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL uno_setCurrentContext(
    void * pCurrentContext,
    rtl_uString * pEnvDcp, void * pEnvContext )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
