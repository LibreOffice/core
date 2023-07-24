/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include "abi.hxx"

int loongarch64::flatten_struct(typelib_TypeDescription* pTypeDescr, Registers& regs)
{
    const typelib_CompoundTypeDescription* p
        = reinterpret_cast<const typelib_CompoundTypeDescription*>(pTypeDescr);
    int sum = p->nMembers;
    for (sal_Int32 i = 0; i < p->nMembers; ++i)
    {
        typelib_TypeDescriptionReference* pTypeInStruct = p->ppTypeRefs[i];

        switch (pTypeInStruct->eTypeClass)
        {
            case typelib_TypeClass_STRUCT:
            {
                typelib_TypeDescription* t = 0;
                TYPELIB_DANGER_GET(&t, pTypeInStruct);
                sum--;
                sum += flatten_struct(t, regs);
                TYPELIB_DANGER_RELEASE(t);
            }
            break;
            case typelib_TypeClass_CHAR:
            case typelib_TypeClass_BOOLEAN:
            case typelib_TypeClass_BYTE:
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_ENUM:
                regs.nr_int++;
                if (!regs.priorInt && !regs.priorFp)
                    regs.priorInt = true;
                break;
            case typelib_TypeClass_FLOAT:
                regs.complex_float = true;
                [[fallthrough]];
            case typelib_TypeClass_DOUBLE:
                regs.nr_fp++;
                if (!regs.priorInt && !regs.priorFp)
                    regs.priorFp = true;
                break;
            default:
                break;
        }
    }
    return sum;
}

loongarch64::ReturnKind loongarch64::getReturnKind(typelib_TypeDescriptionReference* pTypeRef)
{
    switch (pTypeRef->eTypeClass)
    {
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
        case typelib_TypeClass_ENUM:
            return ReturnKind::RegistersInt;
        case typelib_TypeClass_FLOAT:
        case typelib_TypeClass_DOUBLE:
            return ReturnKind::RegistersFp;
        case typelib_TypeClass_STRUCT:
        {
            Registers regs = { false, false, false, 0, 0 };
            typelib_TypeDescription* pTypeDescr = nullptr;
            TYPELIB_DANGER_GET(&pTypeDescr, pTypeRef);
            int sum = flatten_struct(pTypeDescr, regs);
            TYPELIB_DANGER_RELEASE(pTypeDescr);
            if ((sum == 1 || sum == 2) && sum == regs.nr_fp)
            {
                if (regs.complex_float && pTypeRef->pType->nSize == 8)
                    return ReturnKind::RegistersTwoFloat;
                return ReturnKind::RegistersFp;
            }
            if (sum == 2 && regs.nr_fp == regs.nr_int)
            {
                if (regs.priorInt)
                {
                    if (regs.complex_float && pTypeRef->pType->nSize == 8)
                        return ReturnKind::RegistersIntFloat;
                    return ReturnKind::RegistersIntFp;
                }
                if (regs.priorFp)
                {
                    if (regs.complex_float && pTypeRef->pType->nSize == 8)
                        return ReturnKind::RegistersFloatInt;
                    return ReturnKind::RegistersFpInt;
                }
            }
        }
            [[fallthrough]];
        default:
            return ReturnKind::RegistersInt;
    }
}

void loongarch64::fillReturn(typelib_TypeDescriptionReference* pTypeRef, sal_Int64* gret,
                             double* fret, void* pRegisterReturn)
{
    ReturnKind returnKind = getReturnKind(pTypeRef);
    switch (returnKind)
    {
        case ReturnKind::RegistersFp:
            reinterpret_cast<double*>(pRegisterReturn)[0] = fret[0];
            reinterpret_cast<double*>(pRegisterReturn)[1] = fret[1];
            break;
        case ReturnKind::RegistersTwoFloat:
            memcpy(reinterpret_cast<char*>(pRegisterReturn), &(fret[0]), 4);
            memcpy(reinterpret_cast<char*>(pRegisterReturn) + 4, &(fret[1]), 4);
            break;
        case ReturnKind::RegistersFpInt:
            reinterpret_cast<double*>(pRegisterReturn)[0] = fret[0];
            reinterpret_cast<sal_Int64*>(pRegisterReturn)[1] = gret[0];
            break;
        case ReturnKind::RegistersFloatInt:
            memcpy(reinterpret_cast<char*>(pRegisterReturn), fret, 4);
            memcpy(reinterpret_cast<char*>(pRegisterReturn) + 4, gret, 4);
            break;
        case ReturnKind::RegistersIntFp:
            reinterpret_cast<sal_Int64*>(pRegisterReturn)[0] = gret[0];
            reinterpret_cast<double*>(pRegisterReturn)[1] = fret[0];
            break;
        case ReturnKind::RegistersIntFloat:
            memcpy(reinterpret_cast<char*>(pRegisterReturn), gret, 4);
            memcpy(reinterpret_cast<char*>(pRegisterReturn) + 4, fret, 4);
            break;
        default:
            reinterpret_cast<sal_Int64*>(pRegisterReturn)[0] = gret[0];
            reinterpret_cast<sal_Int64*>(pRegisterReturn)[1] = gret[1];
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
