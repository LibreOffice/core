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
#ifndef INCLUDED_CPPU_SOURCE_UNO_CONSTR_HXX
#define INCLUDED_CPPU_SOURCE_UNO_CONSTR_HXX

#include "prim.hxx"
#include <osl/diagnose.h>

namespace cppu
{


//#### construction ################################################################################


void defaultConstructStruct(
    void * pMem,
    typelib_CompoundTypeDescription * pCompType );

inline void _defaultConstructStruct(
    void * pMem,
    typelib_CompoundTypeDescription * pTypeDescr )
{
    if (pTypeDescr->pBaseTypeDescription)
    {
        defaultConstructStruct( pMem, pTypeDescr->pBaseTypeDescription );
    }

    typelib_TypeDescriptionReference ** ppTypeRefs = pTypeDescr->ppTypeRefs;
    sal_Int32 * pMemberOffsets = pTypeDescr->pMemberOffsets;
    sal_Int32 nDescr = pTypeDescr->nMembers;

    while (nDescr--)
    {
        ::uno_type_constructData( static_cast<char *>(pMem) + pMemberOffsets[nDescr], ppTypeRefs[nDescr] );
    }
}


inline void _defaultConstructData(
    void * pMem,
    typelib_TypeDescriptionReference * pType,
    typelib_TypeDescription * pTypeDescr )
{
    switch (pType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        *static_cast<sal_Unicode *>(pMem) = '\0';
        break;
    case typelib_TypeClass_BOOLEAN:
        *static_cast<sal_Bool *>(pMem) = false;
        break;
    case typelib_TypeClass_BYTE:
        *static_cast<sal_Int8 *>(pMem) = 0;
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        *static_cast<sal_Int16 *>(pMem) = 0;
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        *static_cast<sal_Int32 *>(pMem) = 0;
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        *static_cast<sal_Int64 *>(pMem) = 0;
        break;
    case typelib_TypeClass_FLOAT:
        *static_cast<float *>(pMem) = 0.0;
        break;
    case typelib_TypeClass_DOUBLE:
        *static_cast<double *>(pMem) = 0.0;
        break;
    case typelib_TypeClass_STRING:
        *static_cast<rtl_uString **>(pMem) = nullptr;
        ::rtl_uString_new( static_cast<rtl_uString **>(pMem) );
        break;
    case typelib_TypeClass_TYPE:
        *static_cast<typelib_TypeDescriptionReference **>(pMem) = _getVoidType();
        break;
    case typelib_TypeClass_ANY:
        CONSTRUCT_EMPTY_ANY( static_cast<uno_Any *>(pMem) );
        break;
    case typelib_TypeClass_ENUM:
        if (pTypeDescr)
        {
            *static_cast<sal_Int32 *>(pMem) = reinterpret_cast<typelib_EnumTypeDescription *>(pTypeDescr)->nDefaultEnumValue;
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            *static_cast<sal_Int32 *>(pMem) = reinterpret_cast<typelib_EnumTypeDescription *>(pTypeDescr)->nDefaultEnumValue;
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (pTypeDescr)
        {
            _defaultConstructStruct( pMem, reinterpret_cast<typelib_CompoundTypeDescription *>(pTypeDescr) );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            _defaultConstructStruct( pMem, reinterpret_cast<typelib_CompoundTypeDescription *>(pTypeDescr) );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_SEQUENCE:
        *static_cast<uno_Sequence **>(pMem) = createEmptySequence();
        break;
    case typelib_TypeClass_INTERFACE:
        *static_cast<void **>(pMem) = nullptr; // either cpp or c-uno interface
        break;
    default:
        OSL_ASSERT(false);
        break;
    }
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
