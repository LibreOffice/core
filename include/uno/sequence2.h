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
#ifndef INCLUDED_UNO_SEQUENCE2_H
#define INCLUDED_UNO_SEQUENCE2_H

#include "cppu/cppudllapi.h"
#include "uno/data.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct _typelib_TypeDescriptionReference;
struct _typelib_TypeDescription;
typedef sal_Sequence uno_Sequence;

/** Assigns a sequence.

    @param ppDest       destinstaion sequence
    @param pSource      source sequence
    @param pTypeDescr   type description of the sequence and NOT of an element
    @param release      function called each time an interface needs to
                        be released; defaults (0) to uno
*/
CPPU_DLLPUBLIC void SAL_CALL uno_sequence_assign(
    uno_Sequence ** ppDest,
    uno_Sequence * pSource,
    struct _typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();
/** Assigns a sequence.

    @param ppDest       destinstaion sequence
    @param pSource      source sequence
    @param pType        type of the sequence and NOT of an element
    @param release      function called each time an interface needs to
                        be released; defaults (0) to uno
*/
CPPU_DLLPUBLIC void SAL_CALL uno_type_sequence_assign(
    uno_Sequence ** ppDest,
    uno_Sequence * pSource,
    struct _typelib_TypeDescriptionReference * pType,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

/** Constructs a new sequence with given elements.

    @param ppSequence         out parameter sequence;
                              0 if memory allocation has failed
    @param pTypeDescr         type description of the sequence and NOT of an
                              element
    @param pElements          if 0, then all elements are default constructed
    @param len                number of elements
    @param acquire            function called each time an interface needs to
                              be acquired; defaults (0) to uno
    @return                   false, if memoray allocation has failed
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL uno_sequence_construct(
    uno_Sequence ** ppSequence,
    struct _typelib_TypeDescription * pTypeDescr,
    void * pElements, sal_Int32 len,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C();
/** Constructs a new sequence with given elements.

    @param ppSequence         out parameter sequence;
                              0 if memory allocation has failed
    @param pType              type of the sequence and NOT of an element
    @param pElements          if 0, then all elements are default constructed
    @param len                number of elements
    @param acquire            function called each time an interface needs to
                              be acquired; defaults (0) to uno
    @return                   false, if memoray allocation has failed
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL uno_type_sequence_construct(
    uno_Sequence ** ppSequence,
    struct _typelib_TypeDescriptionReference * pType,
    void * pElements, sal_Int32 len,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C();

/** Assures that the reference count of the given sequence is one.
    Otherwise a new copy of the sequence is created with a reference count
    of one.

    @param ppSequence       inout sequence
    @param pTypeDescr       type description of sequence
    @param acquire          function called each time an interface needs to
                            be acquired; defaults (0) to uno
    @param release          function called each time an interface needs to
                            be released; defaults (0) to uno
    @return                 false, if memoray allocation has failed
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL uno_sequence_reference2One(
    uno_Sequence ** ppSequence,
    struct _typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();
/** Assures that the reference count of the given sequence is one.
    Otherwise a new copy of the sequence is created with a reference count
    of one.

    @param ppSequence       inout sequence
    @param pType            type of sequence
    @param acquire          function called each time an interface needs to
                            be acquired; defaults (0) to uno
    @param release          function called each time an interface needs to
                            be released; defaults (0) to uno
    @return                 false, if memoray allocation has failed
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL uno_type_sequence_reference2One(
    uno_Sequence ** ppSequence,
    struct _typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

/** Reallocates length of a sequence. This truncates a sequence or enlarges
    it default constructing appended elements.

    @param ppSequence       inout sequence
    @param pTypeDescr       type description of sequence
    @param nSize            new size of sequence
    @param acquire          function called each time an interface needs to
                            be acquired; defaults (0) to uno
    @param release          function called each time an interface needs to
                            be released; defaults (0) to uno
    @return                 false, if memoray allocation has failed
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL uno_sequence_realloc(
    uno_Sequence ** ppSequence,
    struct _typelib_TypeDescription * pTypeDescr,
    sal_Int32 nSize,
    uno_AcquireFunc acquire,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();
/** Reallocates length of a sequence. This truncates a sequence or enlarges
    it default constructing appended elements.

    @param ppSequence       inout sequence
    @param pType            type of sequence
    @param nSize            new size of sequence
    @param acquire          function called each time an interface needs to
                            be acquired; defaults (0) to uno
    @param release          function called each time an interface needs to
                            be released; defaults (0) to uno
    @return                 false, if memoray allocation has failed
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL uno_type_sequence_realloc(
    uno_Sequence ** ppSequence,
    struct _typelib_TypeDescriptionReference * pType,
    sal_Int32 nSize,
    uno_AcquireFunc acquire,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

/** Destroy a sequence whose reference count has dropped to zero.

    @param sequence must be non-null, sequence->nRefCount must be zero
    @param type the type of the sequence, must be non-null
    @param release function called each time an interface needs to be release,
        must be non-null

    @since LibreOffice 4.4
*/
CPPU_DLLPUBLIC void SAL_CALL uno_type_sequence_destroy(
    uno_Sequence * sequence, struct _typelib_TypeDescriptionReference * type,
    uno_ReleaseFunc release)
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
