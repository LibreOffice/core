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
/*|     list.c - bidirectional list class               |*/
/*|                                                     |*/
/*|                                                     |*/
/*|  Author: Alexander Gelfenbain                       |*/
/*[]---------------------------------------------------[]*/

#include <stdlib.h>
#include <assert.h>

#ifdef MALLOC_TRACE
#include <stdio.h>
#include </usr/local/include/malloc.h>
#endif
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
    lnode *ptr = malloc(sizeof(lnode));
    assert(ptr != 0);

    ptr->value = el;

    return ptr;
}

static lnode *appendPrim(list this, void *el)
{
    lnode *ptr = newNode(el);
    lnode **flink, *blink;

    if (this->tail != 0) {
        flink = &(this->tail->next);
        blink = this->tail;
    } else {
        flink = &this->head;
        blink = 0;
        this->cptr = ptr;                         /*- list was empty - set current to this element */
    }

    *flink  = ptr;
    this->tail = ptr;

    ptr->prev = blink;
    ptr->next = 0;

    this->aCount++;
    return ptr;
}

/*- public methods  */
list listNewEmpty(void)                           /*- default ctor */
{
    list this = malloc(sizeof(struct _list));
    assert(this != 0);

    this->aCount = 0;
    this->eDtor = 0;
    this->head = this->tail = this->cptr = 0;

    return this;
}

void listDispose(list this)                       /*- dtor */
{
    assert(this != 0);
    listClear(this);
    free(this);
}

void listSetElementDtor(list this, list_destructor f)
{
    assert(this != 0);
    this->eDtor = f;
}

/* calling this function on an empty list is a run-time error */
void *listCurrent(list this)
{
    assert(this != 0);
    assert(this->cptr != 0);
    return this->cptr->value;
}

int   listCount(list this)
{
    assert(this != 0);
    return this->aCount;
}

int   listIsEmpty(list this)
{
    assert(this != 0);
    return this->aCount == 0;
}

int    listFind(list this, void *el)
{
    lnode *ptr;
    assert(this != 0);

    ptr = this->head;

    while (ptr) {
        if (ptr->value == el) {
            this->cptr = ptr;
            return 1;
        }
        ptr = ptr->next;
    }

    return 0;
}

int    listNext(list this)
{
    return listSkipForward(this, 1);
}

int    listSkipForward(list this, int n)
{
    int m = 0;
    assert(this != 0);

    if (this->cptr == 0) return 0;

    while (n != 0) {
        if (this->cptr->next == 0) break;
        this->cptr = this->cptr->next;
        n--;
        m++;
    }
    return m;
}

int    listToFirst(list this)
{
    assert(this != 0);

    if (this->cptr != this->head) {
        this->cptr = this->head;
        return 1;
    }
    return 0;
}

int    listToLast(list this)
{
    assert(this != 0);

    if (this->cptr != this->tail) {
        this->cptr = this->tail;
        return 1;
    }
    return 0;
}

int    listPositionAt(list this, int n)                     /*- returns the actual position number */
{
    int m = 0;
    assert(this != 0);

    this->cptr = this->head;
    while (n != 0) {
        if (this->cptr->next == 0) break;
        this->cptr = this->cptr->next;
        n--;
        m++;
    }
    return m;
}

list   listAppend(list this, void *el)
{
    assert(this != 0);

    appendPrim(this, el);
    return this;
}

list   listRemove(list this)
{
    lnode *ptr = 0;
    if (this->cptr == 0) return this;

    if (this->cptr->next != 0) {
        ptr  = this->cptr->next;
        this->cptr->next->prev = this->cptr->prev;
    } else {
        this->tail = this->cptr->prev;
    }

    if (this->cptr->prev != 0) {
        if (ptr == 0) ptr = this->cptr->prev;
        this->cptr->prev->next = this->cptr->next;
    } else {
        this->head = this->cptr->next;
    }

    if (this->eDtor) this->eDtor(this->cptr->value);        /* call the dtor callback */

    free(this->cptr);
    this->aCount--;
    this->cptr = ptr;
    return this;
}

list   listClear(list this)
{
    lnode *node = this->head, *ptr;

    while (node) {
        ptr = node->next;
        if (this->eDtor) this->eDtor(node->value);           /* call the dtor callback */
        free(node);
        this->aCount--;
        node = ptr;
    }

    this->head = this->tail = this->cptr = 0;
    assert(this->aCount == 0);
    return this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
