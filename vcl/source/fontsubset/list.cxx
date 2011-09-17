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

#include <rtl/alloc.h>

#if OSL_DEBUG_LEVEL == 0
#  ifndef NDEBUG
#    define NDEBUG
#  endif
#endif

#include <assert.h>

/* #define TEST */
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
    lnode *ptr = (lnode*)rtl_allocateMemory(sizeof(lnode));
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
        pThis->cptr = ptr;                         /*- list was empty - set current to pThis element */
    }

    *flink  = ptr;
    pThis->tail = ptr;

    ptr->prev = blink;
    ptr->next = 0;

    pThis->aCount++;
    return ptr;
}
#ifdef TEST
static lnode *prependPrim(list pThis, void *el)
{
    lnode *ptr = newNode(el);
    lnode *flink, **blink;

    if (pThis->head != 0) {
        blink = &(pThis->head->prev);
        flink = pThis->head;
    } else {
        blink = &pThis->tail;
        flink = 0;
        pThis->cptr  = ptr;                        /*- list was empty - set current to pThis element */
    }

    *blink = ptr;
    pThis->head   = ptr;

    ptr->next = flink;
    ptr->prev = 0;

    pThis->aCount++;
    return ptr;
}
#endif

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

#ifdef TEST
list listNewCopy(list l)                          /*- copy ctor */
{
    lnode *ptr, *c;
    list pThis;
    assert(l != 0);

    pThis = rtl_allocateMemory(sizeof(struct _list));
    assert(pThis != 0);

    ptr = l->head;

    pThis->aCount = 0;
    pThis->eDtor = 0;
    pThis->head = pThis->tail = pThis->cptr = 0;

    while (ptr) {
        c = appendPrim(pThis, ptr->value);
        if (ptr == l->cptr) pThis->cptr = c;
        ptr = ptr->next;
    }

    return pThis;
}
#endif

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


#ifdef TEST
int   listAtFirst(list pThis)
{
    assert(pThis != 0);
    return pThis->cptr == pThis->head;
}

int   listAtLast(list pThis)
{
    assert(pThis != 0);
    return pThis->cptr == pThis->tail;
}

int   listPosition(list pThis)
{
    int res = 0;
    lnode *ptr;
    assert(pThis != 0);

    ptr = pThis->head;

    while (ptr != pThis->cptr) {
        ptr = ptr->next;
        res++;
    }

    return res;
}
#endif
int    listFind(list pThis, void *el)
{
    lnode *ptr;
    assert(pThis != 0);

    ptr = pThis->head;

    while (ptr) {
        if (ptr->value == el) {
            pThis->cptr = ptr;
            return 1;
        }
        ptr = ptr->next;
    }

    return 0;
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

int    listPositionAt(list pThis, int n)                     /*- returns the actual position number */
{
    int m = 0;
    assert(pThis != 0);

    pThis->cptr = pThis->head;
    while (n != 0) {
        if (pThis->cptr->next == 0) break;
        pThis->cptr = pThis->cptr->next;
        n--;
        m++;
    }
    return m;
}

list   listAppend(list pThis, void *el)
{
    assert(pThis != 0);

    appendPrim(pThis, el);
    return pThis;
}
#ifdef TEST
list   listPrepend(list pThis, void *el)
{
    assert(pThis != 0);

    prependPrim(pThis, el);
    return pThis;
}

list   listInsertAfter(list pThis, void *el)
{
    lnode *ptr;
    assert(pThis != 0);

    if (pThis->cptr == 0) return listAppend(pThis, el);

    ptr = newNode(el);

    ptr->prev  = pThis->cptr;
    ptr->next  = pThis->cptr->next;
    pThis->cptr->next = ptr;

    if (ptr->next != 0) {
        ptr->next->prev = ptr;
    } else {
        pThis->tail = ptr;
    }
    pThis->aCount++;
    return pThis;
}

list   listInsertBefore(list pThis, void *el)
{
    lnode *ptr;
    assert(pThis != 0);

    if (pThis->cptr == 0) return listAppend(pThis, el);

    ptr = newNode(el);

    ptr->prev  = pThis->cptr->prev;
    ptr->next  = pThis->cptr;
    pThis->cptr->prev = ptr;

    if (ptr->prev != 0) {
        ptr->prev->next = ptr;
    } else {
        pThis->head = ptr;
    }
    pThis->aCount++;
    return pThis;
}
#endif
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

#ifdef TEST

void   listForAll(list pThis, void (*f)(void *))
{
    lnode *ptr = pThis->head;
    while (ptr) {
        f(ptr->value);
        ptr = ptr->next;
    }
}


#include <stdio.h>

void printlist(list l)
{
    int saved;
    assert(l != 0);
    saved = listPosition(l);

    printf("[ ");

    if (!listIsEmpty(l)) {
        listToFirst(l);
        do {
            printf("%d ", (int) listCurrent(l));
        } while (listNext(l));
    }

    printf("]\n");

    listPositionAt(l, saved);
}

void printstringlist(list l)
{
    int saved;
    assert(l != 0);
    saved = listPosition(l);

    printf("[ ");

    if (!listIsEmpty(l)) {
        listToFirst(l);
        do {
            printf("'%s' ", (char *) listCurrent(l));
        } while (listNext(l));
    }

    printf("]\n");

    listPositionAt(l, saved);
}

void printstat(list l)
{
    printf("count: %d, position: %d, isEmpty: %d, atFirst: %d, atLast: %d.\n",
           listCount(l), listPosition(l), listIsEmpty(l), listAtFirst(l), listAtLast(l));
}

void allfunc(void *e)
{
    printf("%d ", e);
}

void edtor(void *ptr)
{
    printf("element dtor: 0x%08x\n", ptr);
    rtl_freeMemory(ptr);
}

int main()
{
    list l1, l2;
    char *ptr;
    int i;

    l1 = listNewEmpty();
    printstat(l1);

    listAppend(l1, 1);
    printstat(l1);

    listAppend(l1, 2);
    printstat(l1);

    listAppend(l1, 3);
    printstat(l1);

    printlist(l1);

    listToFirst(l1);
    listInsertBefore(l1, -5);
    printlist(l1);

    l2 = listNewCopy(l1);
    printlist(l2);

    listForAll(l2, allfunc);
    printf("\n");

    listClear(l1);
    listSetElementDtor(l1, edtor);

    for(i=0; i<10; i++) {
        ptr = rtl_allocateMemory(20);
        snprintf(ptr, 20, "element # %d", i);
        listAppend(l1, ptr);
    }

    printstringlist(l1);


    listDispose(l1);
    listDispose(l2);


    return 0;
}
#endif


