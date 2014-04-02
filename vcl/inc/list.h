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

/*[]---------------------------------------------------[]*/
/*|                                                     |*/
/*|  Implementation of the list data type               |*/
/*|                                                     |*/
/*|                                                     |*/
/*|  Author: Alexander Gelfenbain                       |*/
/*[]---------------------------------------------------[]*/

#ifndef INCLUDED_VCL_INC_LIST_H
#define INCLUDED_VCL_INC_LIST_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * List of void * pointers
 */

    typedef struct _list *list;
    typedef void (*list_destructor)(void *);

/*- constructors and a destructor */
    list listNewEmpty(void);
#ifdef TEST
    list listNewCopy(list);
#endif
    void listDispose(list);
    void listSetElementDtor(list, list_destructor);                     /*- this function will be executed when the element is removed via listRemove() or listClear() */

/*- queries */
    void * listCurrent(list);
    int    listCount(list);
    int    listIsEmpty(list);
#ifdef TEST
    int    listAtFirst(list);
    int    listAtLast(list);
    int    listPosition(list);                        /* Expensive! */
#endif

/*- positioning functions */
/*- return the number of elements by which the current position in the list changes */
    int    listNext(list);
    int    listSkipForward(list, int n);
    int    listToFirst(list);
    int    listToLast(list);

/*- adding and removing elements */
    list   listAppend(list, void *);
#ifdef TEST
    list   listPrepend(list, void *);
    list   listInsertAfter(list, void *);
    list   listInsertBefore(list, void *);
#endif
    list   listRemove(list);                          /* removes the current element */
    list   listClear(list);                           /* removes all elements */

#ifdef TEST
/*- forall */
    void   listForAll(list, void (*f)(void *));
#endif

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_VCL_INC_LIST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
