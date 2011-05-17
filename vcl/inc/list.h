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

/*[]---------------------------------------------------[]*/
/*|                                                     |*/
/*|  Implementation of the list data type               |*/
/*|                                                     |*/
/*|                                                     |*/
/*|  Author: Alexander Gelfenbain                       |*/
/*[]---------------------------------------------------[]*/

#ifndef __CLIST_H
#define __CLIST_H

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
/*- search */
    int    listFind(list, void *);                    /* Returns true/false */

/*- positioning functions */
/*- return the number of elements by which the current position in the list changes */
    int    listNext(list);
    int    listSkipForward(list, int n);
    int    listToFirst(list);
    int    listToLast(list);
    int    listPositionAt(list, int n);               /* Expensive! */

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


#endif /* __CLIST_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
