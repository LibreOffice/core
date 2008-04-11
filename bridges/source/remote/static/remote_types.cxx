/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: remote_types.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_bridges.hxx"
#include "remote_types.hxx"

namespace bridges_remote {

sal_Bool SAL_CALL remote_relatesToInterface2( typelib_TypeDescription * pTypeDescr )
{
    switch (pTypeDescr->eTypeClass)
    {
    case typelib_TypeClass_SEQUENCE:
    {
        switch (((typelib_IndirectTypeDescription *)pTypeDescr)->pType->eTypeClass)
        {
        case typelib_TypeClass_SEQUENCE:
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, ((typelib_IndirectTypeDescription *)pTypeDescr)->pType );
            sal_Bool bRel = remote_relatesToInterface( pTD );
            TYPELIB_DANGER_RELEASE( pTD );
            return bRel;
        }
        default:
            break;
        }
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        // ...optimized... to avoid getDescription() calls!
        typelib_CompoundTypeDescription * pComp    = (typelib_CompoundTypeDescription *)pTypeDescr;
        typelib_TypeDescriptionReference ** pTypes = pComp->ppTypeRefs;
        for ( sal_Int32 nPos = pComp->nMembers; nPos--; )
        {
            switch (pTypes[nPos]->eTypeClass)
            {
            case typelib_TypeClass_INTERFACE:
            case typelib_TypeClass_UNION: // might relate to interface
            case typelib_TypeClass_ANY: // might relate to interface
                return sal_True;
            case typelib_TypeClass_SEQUENCE:
            case typelib_TypeClass_STRUCT:
            case typelib_TypeClass_EXCEPTION:
            {
                typelib_TypeDescription * pTD = 0;
                TYPELIB_DANGER_GET( &pTD, pTypes[nPos] );
                sal_Bool bRel = remote_relatesToInterface( pTD );
                TYPELIB_DANGER_RELEASE( pTD );
                if (bRel)
                    return sal_True;
                break;
            }
            default:
                break;
            }
        }
        if (pComp->pBaseTypeDescription)
            return remote_relatesToInterface( (typelib_TypeDescription *)pComp->pBaseTypeDescription );
        break;
    }
    default:
        OSL_ASSERT( 0 );
    }
    return sal_False;
}

}
