/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Enterable.h,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_uno_Enterable_h
#define INCLUDED_uno_Enterable_h

#include "uno/environment.h"


#ifdef __cplusplus
extern "C"
{
#endif


/** Generic function type declaration for entering an Environment.
    (http://wiki.services.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

    @param context
    @since UDK 3.2.7
*/
typedef void uno_Enterable_enter     (void * context);


/** Generic function type declaration for levaing an Environment.
    (http://wiki.services.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

    @param context
    @since UDK 3.2.7
*/
typedef void uno_Enterable_leave     (void * context);


/** Generic function type declaration for calling into an Environment.
    (http://wiki.services.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

    @param context
    @param pCallee  the function to be called
    @param pParam   the parameter pointer to be passed to the function
    @since UDK 3.2.7
*/
typedef void uno_Enterable_callInto_v(void * context, uno_EnvCallee * pCallee, va_list * pParam);


/** Generic function type declaration for calling out of an Environment.
    (http://wiki.services.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

    @param context
    @param pCallee  the function to be called
    @param pParam   the parameter pointer to be passed to the function
    @since UDK 3.2.7
*/
typedef void uno_Enterable_callOut_v (void * context, uno_EnvCallee * pCallee, va_list * pParam);


/** Generic function type declaration for checking if calling on managed object is
    valid.
    (http://wiki.services.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

    @param  context
    @param  ppReason  the reason, in case calling is not valid
    @return           0 == calling is not valid, 1 == calling is valid
    @since UDK 3.2.7
*/
typedef int  uno_Enterable_isValid_v (void * context, rtl_uString ** ppReason);


/** A struct pReserved needs to point to, if implementing a purpose environment.
    (http://wiki.services.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

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
