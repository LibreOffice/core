/*************************************************************************
 *
 *  $RCSfile: unload.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-06-07 09:18:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _RTL_UNLOAD_H_
#define _RTL_UNLOAD_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif
#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif


#define COMPONENT_CANUNLOAD         "component_canUnload"
typedef sal_Bool (SAL_CALL * component_canUnloadFunc)( TimeValue* pTime);


/** C-interface for a module reference counting
 */
#ifdef __cplusplus
extern "C"
{
#endif

sal_Bool SAL_CALL rtl_registerModuleForUnloading( oslModule module);
void SAL_CALL rtl_unregisterModuleForUnloading( oslModule module);
void SAL_CALL rtl_unloadUnusedModules( TimeValue* libUnused);

typedef void (SAL_CALL *rtl_unloadingListenerFunc)(void* id);
sal_Int32 SAL_CALL rtl_addUnloadingListener( rtl_unloadingListenerFunc callback, void* _this);
void SAL_CALL rtl_removeUnloadingListener( sal_Int32 cookie );



typedef struct _rtl_ModuleCount
{
    void ( SAL_CALL * acquire ) ( struct _rtl_ModuleCount * that );
    void ( SAL_CALL * release ) ( struct _rtl_ModuleCount * that );
}rtl_ModuleCount;


#define MODULE_COUNT_INIT \
{ {rtl_moduleCount_acquire,rtl_moduleCount_release}, rtl_moduleCount_canUnload, 0, {0, 0}};

typedef struct _rtl_StandardModuleCount
{
    rtl_ModuleCount modCnt;
     sal_Bool ( *canUnload ) ( struct _rtl_StandardModuleCount* a, TimeValue* libUnused);
    oslInterlockedCount counter;
    TimeValue unusedSince;
} rtl_StandardModuleCount;


void rtl_moduleCount_acquire(rtl_ModuleCount * that );
void rtl_moduleCount_release( rtl_ModuleCount * that );
sal_Bool rtl_moduleCount_canUnload( rtl_StandardModuleCount * that, TimeValue* libUnused);


#ifdef __cplusplus
}
#endif


#endif
