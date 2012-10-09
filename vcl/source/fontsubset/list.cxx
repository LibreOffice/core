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
/*|     list.c - bidirectional list class               |*/
/*|                                                     |*/
/*|                                                     |*/
/*|  Author: Alexander Gelfenbain                       |*/
/*[]---------------------------------------------------[]*/

#include <rtl/alloc.h>
#include <assert.h>

#include "list.h"

/*- private data types */
typedef struct _lnode {
    struct _lnode *next;
    struct _lnode *prev;

    void *value;

} lnode;

struct _list {
    lnode *head, *tail, *cptr;
    size_t aCount;
    list_destructor eDtor;
};

/*- private methods */

static lnode *newNode(void *el)
{
    lnode *ptr = (lnode *)rtl_allocateMemory(sizeof(lnode));
    assert(ptr != 0);

    ptr->value = el;

    return ptr;
}

static lnode *appendPrim(list pThis, void *el)
{
    lnode *ptr = newNode(el);
    lnode **flink, *blink;

    if (pThis->tail != 0) {
        flink = &(pThis->tail->next);
        blink = pThis->tail;
    } else {
        flink = &pThis->head;
        blink = 0;
        pThis->cptr = ptr;                         /*- list was empty - set current to this element */
    }

    *flink  = ptr;
    pThis->tail = ptr;

    ptr->prev = blink;
    ptr->next = 0;

    pThis->aCount++;
    return ptr;
}

/*- public methods  */
list listNewEmpty(void)                           /*- default ctor */
{
    list pThis = (list)rtl_allocateMemory(sizeof(struct _list));
    assert(pThis != 0);

    pThis->aCount = 0;
    pThis->eDtor = 0;
    pThis->head = pThis->tail = pThis->cptr = 0;

    return pThis;
}

void listDispose(list pThis)                       /*- dtor */
{
    assert(pThis != 0);
    listClear(pThis);
    rtl_freeMemory(pThis);
}

void listSetElementDtor(list pThis, list_destructor f)
{
    assert(pThis != 0);
    pThis->eDtor = f;
}

/* calling this function on an empty list is a run-time error */
void *listCurrent(list pThis)
{
    assert(pThis != 0);
    assert(pThis->cptr != 0);
    return pThis->cptr->value;
}

int   listCount(list pThis)
{
    assert(pThis != 0);
    return pThis->aCount;
}

int   listIsEmpty(list pThis)
{
    assert(pThis != 0);
    return pThis->aCount == 0;
}

int    listNext(list pThis)
{
    return listSkipForward(pThis, 1);
}

int    listSkipForward(list pThis, int n)
{
    int m = 0;
    assert(pThis != 0);

    if (pThis->cptr == 0) return 0;

    while (n != 0) {
        if (pThis->cptr->next == 0) break;
        pThis->cptr = pThis->cptr->next;
        n--;
        m++;
    }
    return m;
}

int    listToFirst(list pThis)
{
    assert(pThis != 0);

    if (pThis->cptr != pThis->head) {
        pThis->cptr = pThis->head;
        return 1;
    }
    return 0;
}

int    listToLast(list pThis)
{
    assert(pThis != 0);

    if (pThis->cptr != pThis->tail) {
        pThis->cptr = pThis->tail;
        return 1;
    }
    return 0;
}

list   listAppend(list pThis, void *el)
{
    assert(pThis != 0);

    appendPrim(pThis, el);
    return pThis;
}

list   listRemove(list pThis)
{
    lnode *ptr = 0;
    if (pThis->cptr == 0) return pThis;

    if (pThis->cptr->next != 0) {
        ptr  = pThis->cptr->next;
        pThis->cptr->next->prev = pThis->cptr->prev;
    } else {
        pThis->tail = pThis->cptr->prev;
    }

    if (pThis->cptr->prev != 0) {
        if (ptr == 0) ptr = pThis->cptr->prev;
        pThis->cptr->prev->next = pThis->cptr->next;
    } else {
        pThis->head = pThis->cptr->next;
    }

    if (pThis->eDtor) pThis->eDtor(pThis->cptr->value);        /* call the dtor callback */

    rtl_freeMemory(pThis->cptr);
    pThis->aCount--;
    pThis->cptr = ptr;
    return pThis;
}

list   listClear(list pThis)
{
    lnode *node = pThis->head, *ptr;

    while (node) {
        ptr = node->next;
        if (pThis->eDtor) pThis->eDtor(node->value);           /* call the dtor callback */
        rtl_freeMemory(node);
        pThis->aCount--;
        node = ptr;
    }

    pThis->head = pThis->tail = pThis->cptr = 0;
    assert(pThis->aCount == 0);
    return pThis;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
