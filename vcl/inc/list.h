/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
