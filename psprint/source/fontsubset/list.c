/*************************************************************************
 *
 *  $RCSfile: list.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:14:04 $
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
 *  Contributor(s): Alexander Gelfenbain
 *
 *
 ************************************************************************/

/* $Id: list.c,v 1.2 2003-04-15 16:14:04 vg Exp $ */

/*[]---------------------------------------------------[]*/
/*|                                                     |*/
/*|     list.c - bidirectional list class               |*/
/*|                                                     |*/
/*|                                                     |*/
/*|  Author: Alexander Gelfenbain                       |*/
/*[]---------------------------------------------------[]*/

#include <stdlib.h>

#if OSL_DEBUG_LEVEL == 0
#define NDEBUG
#endif
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
    void (*eDtor)(void *);
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

static lnode *prependPrim(list this, void *el)
{
    lnode *ptr = newNode(el);
    lnode *flink, **blink;

    if (this->head != 0) {
        blink = &(this->head->prev);
        flink = this->head;
    } else {
        blink = &this->tail;
        flink = 0;
        this->cptr  = ptr;                        /*- list was empty - set current to this element */
    }

    *blink = ptr;
    this->head   = ptr;

    ptr->next = flink;
    ptr->prev = 0;

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

list listNewCopy(list l)                          /*- copy ctor */
{
    lnode *ptr, *c;
    list this;
    assert(l != 0);

    this = malloc(sizeof(struct _list));
    assert(this != 0);

    ptr = l->head;

    this->aCount = 0;
    this->eDtor = 0;
    this->head = this->tail = this->cptr = 0;

    while (ptr) {
        c = appendPrim(this, ptr->value);
        if (ptr == l->cptr) this->cptr = c;
        ptr = ptr->next;
    }

    return this;
}

void listDispose(list this)                       /*- dtor */
{
    assert(this != 0);
    listClear(this);
    free(this);
}

void listSetElementDtor(list this, void (*f)(void *))
{
    assert(this != 0);
    this->eDtor = f;
}


list listCopy(list to, list from)                 /*- assignment */
{
    lnode *ptr, *c;
    assert(to != 0);
    assert(from != 0);

    listClear(to);
    ptr = from->head;

    while (ptr) {
        c = appendPrim(to, ptr->value);
        if (ptr == from->cptr) to->cptr = c;
        ptr = ptr->next;
    }

    return to;
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


int   listAtFirst(list this)
{
    assert(this != 0);
    return this->cptr == this->head;
}

int   listAtLast(list this)
{
    assert(this != 0);
    return this->cptr == this->tail;
}

int   listPosition(list this)
{
    int res = 0;
    lnode *ptr;
    assert(this != 0);

    ptr = this->head;

    while (ptr != this->cptr) {
        ptr = ptr->next;
        res++;
    }

    return res;
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

int    listPrev(list this)
{
    return listSkipBackward(this, 1);
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

int    listSkipBackward(list this, int n)
{
    int m = 0;
    assert(this != 0);

    if (this->cptr == 0) return 0;

    while (n != 0) {
        if (this->cptr->prev == 0) break;
        this->cptr = this->cptr->prev;
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

list   listPrepend(list this, void *el)
{
    assert(this != 0);

    prependPrim(this, el);
    return this;
}

list   listInsertAfter(list this, void *el)
{
    lnode *ptr;
    assert(this != 0);

    if (this->cptr == 0) return listAppend(this, el);

    ptr = newNode(el);

    ptr->prev  = this->cptr;
    ptr->next  = this->cptr->next;
    this->cptr->next = ptr;

    if (ptr->next != 0) {
        ptr->next->prev = ptr;
    } else {
        this->tail = ptr;
    }
    this->aCount++;
    return this;
}

list   listInsertBefore(list this, void *el)
{
    lnode *ptr;
    assert(this != 0);

    if (this->cptr == 0) return listAppend(this, el);

    ptr = newNode(el);

    ptr->prev  = this->cptr->prev;
    ptr->next  = this->cptr;
    this->cptr->prev = ptr;

    if (ptr->prev != 0) {
        ptr->prev->next = ptr;
    } else {
        this->head = ptr;
    }
    this->aCount++;
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

void   listForAll(list this, void (*f)(void *))
{
    lnode *ptr = this->head;
    while (ptr) {
        f(ptr->value);
        ptr = ptr->next;
    }
}

void **listToArray(list this)
{
    void **res;
    lnode *ptr = this->head;
    int i = 0;

    assert(this->aCount != 0);
    res = calloc(this->aCount, sizeof(void *));
    assert(res != 0);

    while (ptr) {
        res[i++] = ptr->value;
        ptr = ptr->next;
    }
    return res;
}


/* #define TEST */
#ifdef TEST
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
    free(ptr);
}

int main()
{
    list l1, l2;
    char *ptr;
    int i;

#ifdef MALLOC_TRACE
    mal_leaktrace(1);
    mal_debug(2);
#endif

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
        ptr = malloc(20);
        sprintf(ptr, "element # %d", i);
        listAppend(l1, ptr);
    }

    printstringlist(l1);


    listDispose(l1);
    listDispose(l2);

#ifdef MALLOC_TRACE
    mal_dumpleaktrace(stdout);
#endif


    return 0;
}
#endif


