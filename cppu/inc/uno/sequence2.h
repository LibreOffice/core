/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sequence2.h,v $
 * $Revision: 1.9 $
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
#ifndef _UNO_SEQUENCE2_H_
#define _UNO_SEQUENCE2_H_

#include <sal/types.h>
#include <uno/data.h>

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
void SAL_CALL uno_sequence_assign(
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
void SAL_CALL uno_type_sequence_assign(
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
sal_Bool SAL_CALL uno_sequence_construct(
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
sal_Bool SAL_CALL uno_type_sequence_construct(
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
sal_Bool SAL_CALL uno_sequence_reference2One(
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
sal_Bool SAL_CALL uno_type_sequence_reference2One(
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
sal_Bool SAL_CALL uno_sequence_realloc(
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
sal_Bool SAL_CALL uno_type_sequence_realloc(
    uno_Sequence ** ppSequence,
    struct _typelib_TypeDescriptionReference * pType,
    sal_Int32 nSize,
    uno_AcquireFunc acquire,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
