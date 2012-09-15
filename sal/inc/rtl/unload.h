/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _RTL_UNLOAD_H_
#define _RTL_UNLOAD_H_

#include "sal/config.h"

#include "osl/interlck.h"
#include "osl/module.h"
#include "osl/time.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

/** @file
The API enables an effective way of unloading libraries in a centralized way.
The mechanism ensures that used libraries are not unloaded. This prevents
crashes if library code is being used after unloading the library.
The unloading mechanism currently only works with libraries which contain
UNO services. A library cannot be unloaded if one of the following conditions
apply

<ul>
<li>An instance is still referenced </li>
<li>A module has been loaded without registering it </li>
<li>The service manager has created a one instance service </li>
<li>A service instance has been added to an UNO context </li>
</ul>

<b>Notification Mechanism</b>
The API provides a notification mechanism. Clients can use it to do clean up,
such as releasing cached references, in order to allow modules to be unloaded.
As long as someone holds a reference to an object whose housing module
supports unloading the module cannot be unloaded.<p>

Because of the inherent danger of crashing the application by using this API
all instances which control threads should be registered listeners. On
notification they have to ensure that their threads assume a safe state, that
is, they run outside of modules which could be unloaded and do not jump
back into module code as a result of a finished function call. In other words,
there must not be an address of the module on the thread's stack.
<p>
Since current operating systems lack APIs in respect to controlling the
position of threads within libraries, it would be a major effort to comply with
that recommendation. The best and most efficient way of handling the unloading
scenario is to let all threads, except for the main thread, die in case of a
notification.
<p>
Use this API with great care because it might crash the application. See the
respective documentation (Library Unloading) on the udk.openoffice.org web site.
*/


/**
A library which supports unloading has to implement and export a function
called <code>component_canUnload</code>. <p>
If the function returns <code>sal_True</code> then the module can be safely unloaded.
That is the case when there are no external references to code within the
library. In case a module houses UNO components then the function must return
<code>sal_False</code> after the first factory has been handed out. The function then
continues to return <code>sal_False</code> as long as there is at least one object (factory
or service instance)  which originated from the module.<p>

Libraries which not only contain UNO components (or none at all) have to
provide a means to control whether they can be unloaded or not, e.g. However,
there is no concept yet. <p>

The argument <code>pTime</code> is an optional out-parameter. If the return value is
<code>sal_True</code> then <code>pTime</code> reflects a point in time since
when the module could have
been unloaded. Since that time the function would have continually returned
<code>sal_True</code> up to the present. The value of <code>pTime</code> is
important for the decision
as to a module will be unloaded. When someone initiates the unloading of
modules by calling <code>rtl_unloadUnusedModules</code> then the caller can specify a time
span with the effect that only those modules are unloaded which are unused at
least for that amount of time. If <code>component_canUnload</code> does not
fill in <code>pTime</code>
then the module is unloaded immediately.<p>

<code>component_canUnload</code> is implicitly called by <code>rtl_unloadUnusedModules
</code>. There is no need to call the function directly.
*/
#define COMPONENT_CANUNLOAD         "component_canUnload"
typedef sal_Bool (SAL_CALL * component_canUnloadFunc)( TimeValue* pTime);


/** C-interface for a module reference counting
 */
#ifdef __cplusplus
extern "C"
{
#endif

/**
By registering a module, one declares that a module supports the
unloading mechanism. One registers a module by calling this function.<p>

A module can only be unloaded from memory when it has been registered
as many times as it has been loaded. The reason is that a library can
be &quot;loaded&quot; several times by <code>osl_loadModule</code>
within the same process. The
function will then return the same module handle because the library will
effectively only be loaded once. To remove the library from memory it is
necessary to call <code>osl_unloadModule</code> as often as <code>
osl_loadModule</code> was called. The
function <code>rtl_unloadUnusedModules</code> calls <code>osl_unloadModule</code>
for a module as many
times as it was registered. If, for example, a module has been registered one
time less then <code>osl_loadModule</code> has been called and the module can be unloaded
then it needs a call to <code>rtl_unloadUnusedModules</code> and an explicit call to
<code>osl_unloadModule</code> to remove the module from memory. <p>

A module must be registered every time it has been loaded otherwise the
unloading mechanism is not effective.<p>

Before a module is registered, one has to make sure that the module is in a
state that prevents it from being unloaded. In other words,
<code>component_canUnload</code> must return <code>sal_False</code>. Assuming that
<code>component_canUnload</code>
returns <code>sal_True</code> and it is registered regardless, then a call to
<code>rtl_unloadUnusedModules</code> causes the module to be unloaded. This unloading can
be set off by a different thread and the thread which registered the module is
&quot;unaware&quot; of this. Then when the first thread tries to obtain a factory or
calls another function in the module, the application will crash, because the
module has been unloaded before. Therefore one has to ensure that the module
cannot be unloaded before it is registered. This is simply done by obtaining a
factory from the module. As long as a factory or some other object, which has
been created by the factory, is alive, the <code>component_canUnload</code> function will
return <code>sal_False</code>.<p>
Loading and registering have to be in this order:<br>
<ul>
<li>load a library (<code>osl_loadModule</code>)</li>
<li>get the <code>component_getFactory</code> function and get a factory</li>
<li>register the module (rtl_registerModuleForUnloading</li>
</ul>
Usually the service manager is used to obtain an instance of a service.
The service manager registers all modules which support the unloading mechanism.
When the service manager is used to get service instances than one does not
have to bother about registering.

@param module a module handle as is obtained by osl_loadModule
@return sal_True - the module could be registered for unloading, sal_False otherwise
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL rtl_registerModuleForUnloading( oslModule module);

/**
The function revokes the registration of a module. By calling the function for
a previously registered module one prevents the module from being unloaded by
this unloading mechanism. However, in order to completely unregister the module
it is necessary to call the function as often as the module has been registered.
<p>
<code>rtl_unloadUnusedModules</code> unregisters the modules which it unloads. Therefore
there is no need to call this function unless one means to prevent the unloading of a module.

@param module a module handle as is obtained by osl_loadModule
*/
SAL_DLLPUBLIC void SAL_CALL rtl_unregisterModuleForUnloading( oslModule module);
/**
This function sets off the unloading mechanism. At first it notifies the
unloading listeners in order to give them a chance to do cleanup and get
their threads in a safe state. Then all registered modules are asked if they
can be unloaded. That is, the function calls component_canUnload on every
registered module. If <code>sal_True</code> is returned then <code>osl_unloadModule</code>
is called for the belonging module as often as it is registered.
<p>
A call to <code>osl_unloadModule</code> does not guarantee that the module is unloaded even
if its <code>component_canUnload</code> function returns <code>sal_True</code>.
<p>
The optional in-parameter <code>libUnused</code> specifies a period of time which a library
must be unused in order to qualify for being unloaded. By using this argument
one can counter the multithreading problem as described further above. It is in
the responsibility of the user of this function to provide a timespan big enough
to ensure that all threads are out of modules (see <code>component_canUnload</code>).
<p>
The service managers which have been created by functions such as
<code>createRegistryServiceFactory</code> (declared in cppuhelper/servicefactory.hxx) are
registered listeners and release the references to factories on notification.


@param libUnused span of time that a module must be unused to be unloaded. the
argument is optional.
*/
SAL_DLLPUBLIC void SAL_CALL rtl_unloadUnusedModules( TimeValue* libUnused);

/**
rtl_addUnloadingListener takes an argument of this type.

@param id - The value that has been passed as second argument to rtl_addUnloadingListener
*/
typedef void (SAL_CALL *rtl_unloadingListenerFunc)(void* id);
/**
The function registered an unloading listener. The callback argument is a
function which is called  when the unloading procedure has been initiated by a call to
<code>rtl_unloadUnusedLibraries</code>. The second argument is used to distinguish between different
listener instances and may be <code>NULL</code>. It will be passed as argument when the callback
function is being called. The return value identifies the registered listener and will
be used for removing the listener later on. If the same listener is added more then
once then every registration is treated as if made for a different listener. That is,
a different cookie is returned and the callback function will be called as many times
as it has been registered.
@param callback - a function that is called to notify listeners.
@param _this  - a value to distinguish different listener instances
@return identifier which is used in rtl_removeUnloadingListener
*/
SAL_DLLPUBLIC sal_Int32 SAL_CALL rtl_addUnloadingListener( rtl_unloadingListenerFunc callback, void* _this);

/**
Listeners (the callback functions) must be unregistered before the listener code
becomes invalid. That is, if a module contains listener code, namely callback
functions of type <code>rtl_unloadingListenerFunc</code>, then those functions must not be
registered when <code>component_canUnload</code> returns <code>sal_True</code>.

@param cookie is an identifier as returned by <code>rtl_addUnloadingListener</code> function.
*/
SAL_DLLPUBLIC void SAL_CALL rtl_removeUnloadingListener( sal_Int32 cookie );


/**
Pointers to <code>rtl_ModuleCount</code> are passed as arguments to the default factory creator
functions: <code>createSingleComponentFactory</code>, <code>createSingleFactory</code>,
<code>createOneInstanceFactory</code>.
The factory implementation is calling <code>rtl_ModuleCount.acquire</code> when it is being
constructed and it is calling <code>rtl_ModuleCount.release</code>. The implementations of
<code>acquire</code>
and <code>release</code> should influence the return value of <code>component_canUnload</code>
in a way that it
returns <code>sal_False</code> after <code>acquire</code> has been called. That is the module will not be unloaded
once a default factory has been created. A call to <code>release</code> may cause
<code>component_canUnload</code> to return <code>sal_False</code>, but only if there are
no object alive which
originated from the module. These objects are factory instances and the service instances
which have been created by these factories.
<p>
It is not necessary to synchronize <code>acquire</code> and <code>release</code> as a whole.
Simply sychronize the
access to a counter variable, e.g. the <code>rtl_moduleCount_release</code> implementation:
<pre>
extern "C" void rtl_moduleCount_acquire(rtl_ModuleCount * that )
{
    rtl_StandardModuleCount* pMod= (rtl_StandardModuleCount*)that;
    osl_incrementInterlockedCount( &pMod->counter);
}
</pre>
The SAL library offers functions that can be used for <code>acquire</code> and <code>release</code>. See struct
<code>_rtl_StandardModuleCount</code>.
*/
typedef struct _rtl_ModuleCount
{
    void ( SAL_CALL * acquire ) ( struct _rtl_ModuleCount * that );
    void ( SAL_CALL * release ) ( struct _rtl_ModuleCount * that );
}rtl_ModuleCount;


#define MODULE_COUNT_INIT \
{ {rtl_moduleCount_acquire,rtl_moduleCount_release}, rtl_moduleCount_canUnload, 0, {0, 0}}

/**
This struct can be used to implement the unloading mechanism. To make a UNO library
unloadable create one global instance of this struct in the module. To initialize it one
uses the MODULE_COUNT_INIT macro.

<pre>rtl_StandardModuleCount globalModuleCount= MODULE_COUNT_INIT</pre>;
*/
typedef struct _rtl_StandardModuleCount
{
    rtl_ModuleCount modCnt;
     sal_Bool ( *canUnload ) ( struct _rtl_StandardModuleCount* a, TimeValue* libUnused);
    oslInterlockedCount counter;
    TimeValue unusedSince;
} rtl_StandardModuleCount;

/** Default implementation for <code>rtl_ModuleCount.acquire</code>. Use this function along with
<code>rtl_StandardModuleCount</code>.
*/
SAL_DLLPUBLIC void rtl_moduleCount_acquire(rtl_ModuleCount * that );
/** Default implementation for <code>rtl_ModuleCount.release</code>.
Use this function along with
<code>rtl_StandardModuleCount</code>.
*/
SAL_DLLPUBLIC void rtl_moduleCount_release( rtl_ModuleCount * that );

/** Default implementation for <code>component_canUnload</code>. Use this function along with
<code>rtl_StandardModuleCount</code>.
*/
SAL_DLLPUBLIC sal_Bool rtl_moduleCount_canUnload( rtl_StandardModuleCount * that, TimeValue* libUnused);


#ifdef __cplusplus
}
#endif


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
