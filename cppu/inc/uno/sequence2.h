/*************************************************************************
 *
 *  $RCSfile: sequence2.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:10:56 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _UNO_SEQUENCE2_H_
#define _UNO_SEQUENCE2_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _UNO_DATA_H_
#include <uno/data.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef sal_Sequence uno_Sequence;

/** Assign a sequence.
    <br>
    @param ppDest       destinstaion sequence
    @param pSource      source sequence
    @param pTypeDescr   type description of the <B>sequence</B> and <b>not</b> of an element
    @param release      function called each time an interface needs to be released; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_sequence_assign(
    uno_Sequence ** ppDest,
    uno_Sequence * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();
/** Assign a sequence.
    <br>
    @param ppDest       destinstaion sequence
    @param pSource      source sequence
    @param pType        type of the <B>sequence</B> and <b>not</b> of an element
    @param release      function called each time an interface needs to be released; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_type_sequence_assign(
    uno_Sequence ** ppDest,
    uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

/** Constructs a new sequence with given elements.
    <br>
    @param ppSequence         <b>out</b> parameter sequence
    @param pTypeDescr         type description of the <B>sequence</B> and <b>not</b> of an element
    @param pElements          if 0, then all elements are default constructed
    @param len                number of elements
    @param acquire            function called each time an interface needs to be acquired; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_sequence_construct(
    uno_Sequence ** ppSequence,
    typelib_TypeDescription * pTypeDescr,
    void * pElements, sal_Int32 len,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C();
/** Constructs a new sequence with given elements.
    <br>
    @param ppSequence         <b>out</b> parameter sequence
    @param pType               type of the <B>sequence</B> and <b>not</b> of an element
    @param pElements          if 0, then all elements are default constructed
    @param len                number of elements
    @param acquire            function called each time an interface needs to be acquired; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_type_sequence_construct(
    uno_Sequence ** ppSequence,
    typelib_TypeDescriptionReference * pType,
    void * pElements, sal_Int32 len,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C();

/** Assures that the reference count of the given sequence is one.
    Otherwise a new copy of the sequence is created with a reference count of one.
    <br>
    @param ppSequence       sequence
    @param pTypeDescr       type description of sequence
    @param acquire          function called each time an interface needs to be acquired; defaults (0) to uno
    @param release          function called each time an interface needs to be released; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_sequence_reference2One(
    uno_Sequence ** ppSequence,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();
/** Assures that the reference count of the given sequence is one.
    Otherwise a new copy of the sequence is created with a reference count of one.
    <br>
    @param ppSequence       sequence
    @param pType            type of sequence
    @param acquire          function called each time an interface needs to be acquired; defaults (0) to uno
    @param release          function called each time an interface needs to be released; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_type_sequence_reference2One(
    uno_Sequence ** ppSequence,
    typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

/** Reallocates length of a sequence. This truncates a sequence or enlarges it
    default constructing appended elements.
    <br>
    @param ppSequence       sequence
    @param pTypeDescr       type description of sequence
    @param nSize            new size of sequence
    @param acquire          function called each time an interface needs to be acquired; defaults (0) to uno
    @param release          function called each time an interface needs to be released; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_sequence_realloc(
    uno_Sequence ** ppSequence,
    typelib_TypeDescription * pTypeDescr,
    sal_Int32 nSize,
    uno_AcquireFunc acquire,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();
/** Reallocates length of a sequence. This truncates a sequence or enlarges it
    default constructing appended elements.
    <br>
    @param ppSequence       sequence
    @param pType            type of sequence
    @param nSize            new size of sequence
    @param acquire          function called each time an interface needs to be acquired; defaults (0) to uno
    @param release          function called each time an interface needs to be released; defaults (0) to uno
*/
SAL_DLLEXPORT void SAL_CALL uno_type_sequence_realloc(
    uno_Sequence ** ppSequence,
    typelib_TypeDescriptionReference * pType,
    sal_Int32 nSize,
    uno_AcquireFunc acquire,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
