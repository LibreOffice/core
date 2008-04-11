/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: any.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"

#include "copy.hxx"
#include "destr.hxx"

using namespace cppu;


extern "C"
{
//##################################################################################################
void SAL_CALL uno_type_any_assign(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    _destructAny( pDest, release );
    if (pType)
    {
        _copyConstructAny( pDest, pSource, pType, 0, acquire, 0 );
    }
    else
    {
        CONSTRUCT_EMPTY_ANY( pDest );
    }
}
//##################################################################################################
void SAL_CALL uno_any_assign(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    _destructAny( pDest, release );
    if (pTypeDescr)
    {
        _copyConstructAny( pDest, pSource, pTypeDescr->pWeakRef, pTypeDescr, acquire, 0 );
    }
    else
    {
        CONSTRUCT_EMPTY_ANY( pDest );
    }
}
//##################################################################################################
void SAL_CALL uno_type_any_construct(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    if (pType)
    {
        _copyConstructAny( pDest, pSource, pType, 0, acquire, 0 );
    }
    else
    {
        CONSTRUCT_EMPTY_ANY( pDest );
    }
}
//##################################################################################################
void SAL_CALL uno_any_construct(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    if (pTypeDescr)
    {
        _copyConstructAny( pDest, pSource, pTypeDescr->pWeakRef, pTypeDescr, acquire, 0 );
    }
    else
    {
        CONSTRUCT_EMPTY_ANY( pDest );
    }
}
//##################################################################################################
void SAL_CALL uno_type_any_constructAndConvert(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    if (pType)
    {
        _copyConstructAny( pDest, pSource, pType, 0, 0, mapping );
    }
    else
    {
        CONSTRUCT_EMPTY_ANY( pDest );
    }
}
//##################################################################################################
void SAL_CALL uno_any_constructAndConvert(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    if (pTypeDescr)
    {
        _copyConstructAny( pDest, pSource, pTypeDescr->pWeakRef, pTypeDescr, 0, mapping );
    }
    else
    {
        CONSTRUCT_EMPTY_ANY( pDest );
    }
}
//##################################################################################################
void SAL_CALL uno_any_destruct( uno_Any * pValue, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    _destructAny( pValue, release );
}
//##################################################################################################
void SAL_CALL uno_any_clear( uno_Any * pValue, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    _destructAny( pValue, release );
    CONSTRUCT_EMPTY_ANY( pValue );
}
}
