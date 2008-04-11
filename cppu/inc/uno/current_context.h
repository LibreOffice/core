/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: current_context.h,v $
 * $Revision: 1.8 $
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
#ifndef _UNO_CURRENT_CONTEXT_H_
#define _UNO_CURRENT_CONTEXT_H_

#include <rtl/ustring.h>

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
sal_Bool SAL_CALL uno_getCurrentContext(
    void ** ppCurrentContext,
    rtl_uString * pEnvDcp, void * pEnvContext )
    SAL_THROW_EXTERN_C();

/** Sets the current task's context.

    @param pCurrentContext  in param current context of type com.sun.star.uno.XCurrentContext
    @param pEnvDcp          descriptor of interface's environment
    @param pEnvContext      context of interface's environment (commonly 0)
    @return                 true, if context ref was transferred (even if null ref)
*/
sal_Bool SAL_CALL uno_setCurrentContext(
    void * pCurrentContext,
    rtl_uString * pEnvDcp, void * pEnvContext )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
