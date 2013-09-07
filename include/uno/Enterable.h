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

#ifndef INCLUDED_uno_Enterable_h
#define INCLUDED_uno_Enterable_h

#include "uno/environment.h"


#ifdef __cplusplus
extern "C"
{
#endif


/** Generic function type declaration for entering an Environment.
    (http://wiki.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

    @param context
    @since UDK 3.2.7
*/
typedef void uno_Enterable_enter     (void * context);


/** Generic function type declaration for levaing an Environment.
    (http://wiki.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

    @param context
    @since UDK 3.2.7
*/
typedef void uno_Enterable_leave     (void * context);


/** Generic function type declaration for calling into an Environment.
    (http://wiki.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

    @param context
    @param pCallee  the function to be called
    @param pParam   the parameter pointer to be passed to the function
    @since UDK 3.2.7
*/
typedef void uno_Enterable_callInto_v(void * context, uno_EnvCallee * pCallee, va_list * pParam);


/** Generic function type declaration for calling out of an Environment.
    (http://wiki.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

    @param context
    @param pCallee  the function to be called
    @param pParam   the parameter pointer to be passed to the function
    @since UDK 3.2.7
*/
typedef void uno_Enterable_callOut_v (void * context, uno_EnvCallee * pCallee, va_list * pParam);


/** Generic function type declaration for checking if calling on managed object is
    valid.
    (http://wiki.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

    @param  context
    @param  ppReason  the reason, in case calling is not valid
    @return           0 == calling is not valid, 1 == calling is valid
    @since UDK 3.2.7
*/
typedef int  uno_Enterable_isValid_v (void * context, rtl_uString ** ppReason);


/** A struct pReserved needs to point to, if implementing a purpose environment.
    (http://wiki.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

    @since UDK 3.2.7
*/
typedef struct
{
    uno_Enterable_enter      * m_enter;
    uno_Enterable_leave      * m_leave;
    uno_Enterable_callInto_v * m_callInto_v;
    uno_Enterable_callOut_v  * m_callOut_v;
    uno_Enterable_isValid_v  * m_isValid;
}
uno_Enterable;


#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
