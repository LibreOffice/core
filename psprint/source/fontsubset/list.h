/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: list.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:39:20 $
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

/* $Id: list.h,v 1.2 2005-09-08 16:39:20 rt Exp $ */

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

/*- constructors and a destructor */
    list listNewEmpty(void);
    list listNewCopy(list);
    void listDispose(list);
    void listSetElementDtor(list, void (*f)(void *));                     /*- this function will be executed when the element is removed via listRemove() or listClear() */

/*- assignment */
    list listCopy(list to, list from);

/*- queries */
    void * listCurrent(list);
    int    listCount(list);
    int    listIsEmpty(list);
    int    listAtFirst(list);
    int    listAtLast(list);
    int    listPosition(list);                        /* Expensive! */

/*- search */
    int    listFind(list, void *);                    /* Returns true/false */

/*- positioning functions */
/*- return the number of elements by which the current position in the list changes */
    int    listNext(list);
    int    listPrev(list);
    int    listSkipForward(list, int n);
    int    listSkipBackward(list, int n);
    int    listToFirst(list);
    int    listToLast(list);
    int    listPositionAt(list, int n);               /* Expensive! */

/*- adding and removing elements */
    list   listAppend(list, void *);
    list   listPrepend(list, void *);
    list   listInsertAfter(list, void *);
    list   listInsertBefore(list, void *);
    list   listRemove(list);                          /* removes the current element */
    list   listClear(list);                           /* removes all elements */

/*- forall */
    void   listForAll(list, void (*f)(void *));

/*- conversion */
    void **listToArray(list);

#ifdef __cplusplus
}
#endif


#endif /* __CLIST_H */
