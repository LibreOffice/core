/*************************************************************************
 *
 *  $RCSfile: list.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: pl $ $Date: 2001-05-08 11:45:36 $
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

/* $Id: list.h,v 1.1.1.1 2001-05-08 11:45:36 pl Exp $ */

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
