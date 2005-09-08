/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: any.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:50:06 $
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
