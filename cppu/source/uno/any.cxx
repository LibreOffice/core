/*************************************************************************
 *
 *  $RCSfile: any.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:52 $
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

#include "copy.hxx"
#include "destr.hxx"

using namespace cppu;


extern "C"
{
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_type_any_construct(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire )
{
    if (pType)
    {
        __copyConstructAny( pDest, pSource, pType, 0, acquire, 0 );
    }
    else
    {
        __CONSTRUCT_EMPTY_ANY( (uno_Any *)pDest );
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_any_construct(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire )
{
    if (pTypeDescr)
    {
        __copyConstructAny( pDest, pSource, pTypeDescr->pWeakRef, pTypeDescr, acquire, 0 );
    }
    else
    {
        __CONSTRUCT_EMPTY_ANY( pDest );
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_type_any_constructAndConvert(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_Mapping * mapping )
{
    if (pType)
    {
        __copyConstructAny( pDest, pSource, pType, 0, 0, mapping );
    }
    else
    {
        __CONSTRUCT_EMPTY_ANY( pDest );
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_any_constructAndConvert(
    uno_Any * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_Mapping * mapping )
{
    if (pTypeDescr)
    {
        __copyConstructAny( pDest, pSource, pTypeDescr->pWeakRef, pTypeDescr, 0, mapping );
    }
    else
    {
        __CONSTRUCT_EMPTY_ANY( pDest );
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_any_destruct( uno_Any * pValue, uno_ReleaseFunc release )
{
    __destructAny( pValue, release );
}
}
