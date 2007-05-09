/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Enterable.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:33:57 $
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
    @param param    the parameter pointer to be passed to the function
    @since UDK 3.2.7
*/
typedef void uno_Enterable_callInto_v(void * context, uno_EnvCallee * pCallee, va_list param);


/** Generic function type declaration for calling out of an Environment.
    (http://wiki.services.openoffice.org/wiki/Uno/Binary/Spec/Environment_Stack)

    @param context
    @param pCallee  the function to be called
    @param param    the parameter pointer to be passed to the function
    @since UDK 3.2.7
*/
typedef void uno_Enterable_callOut_v (void * context, uno_EnvCallee * pCallee, va_list param);


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
