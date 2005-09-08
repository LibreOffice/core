/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: current_context.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:38:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _UNO_CURRENT_CONTEXT_H_
#define _UNO_CURRENT_CONTEXT_H_

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/** Gets the current task's context.
    @attention
    Don't spread the returned interface around to other threads.  Every thread has its own
    current context.

    @param ppCurrentContext inout param current context of type com.sun.star.uno.XCurrentContext
    @param pEnvTypeName type name of returned interface's environment
    @param pEnvContext context of returned interface's environment (commonly 0)
    @return true, if context ref was transferred (even if null ref)
*/
sal_Bool SAL_CALL uno_getCurrentContext(
    void ** ppCurrentContext,
    rtl_uString * pEnvTypeName, void * pEnvContext )
    SAL_THROW_EXTERN_C();

/** Sets the current task's context.

    @param pCurrentContext in param current context of type com.sun.star.uno.XCurrentContext
    @param pEnvTypeName type name of interface's environment
    @param pEnvContext context of interface's environment (commonly 0)
    @return true, if context ref was transferred (even if null ref)
*/
sal_Bool SAL_CALL uno_setCurrentContext(
    void * pCurrentContext,
    rtl_uString * pEnvTypeName, void * pEnvContext )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
