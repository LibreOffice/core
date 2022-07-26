/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/genfunc.hxx>
#include <uno/data.h>
#include <typelib/typedescription.hxx>
#include "types.hxx"
#include "abi.hxx"
#include <stdio.h>
#include <cstring>

//#define BRIDGE_DEBUG

namespace abi_riscv64
{
void countnGreg(sal_Int32& nGreg, sal_Int32& nFreg,
                const typelib_CompoundTypeDescription* pTypeDescr)
{
    for (int i = 0; i < pTypeDescr->nMembers; i++)
    {
        typelib_TypeDescriptionReference* pTypeInStruct = pTypeDescr->ppTypeRefs[i];
        switch (pTypeInStruct->eTypeClass)
        {
            case typelib_TypeClass_STRUCT:
            case typelib_TypeClass_EXCEPTION:
            {
                typelib_TypeDescription* childTypeDescr = nullptr;
                TYPELIB_DANGER_GET(&childTypeDescr, pTypeInStruct);
                countnGreg(
                    nGreg, nFreg,
                    reinterpret_cast<typelib_CompoundTypeDescription const*>(childTypeDescr));
                TYPELIB_DANGER_RELEASE(childTypeDescr);
            }
            break;
            case typelib_TypeClass_FLOAT:
            case typelib_TypeClass_DOUBLE:
                nFreg++;
                break;
            default:
                nGreg++;
                break;
        }
    }
}

void fillStruct(const typelib_TypeDescription* pTypeDescr, sal_Int64* gret, double* fret,
                void* pRegisterReturn)
{
#ifdef BRIDGE_DEBUG
    printf("In fillStruct, pTypeDescr = %p, gret = %p, fret = %p, pRegisterReturn = %p\n",
           pTypeDescr, gret, fret, pRegisterReturn);
#endif
    sal_Int32 nGreg = 0;
    sal_Int32 nFreg = 0;
    countnGreg(nGreg, nFreg, reinterpret_cast<typelib_CompoundTypeDescription const*>(pTypeDescr));
    char* pAdjust = reinterpret_cast<char*>(pRegisterReturn);
    if (nGreg == 0 && nFreg <= 2)
    {
        if (pTypeDescr->nSize <= 8 && nFreg == 2)
        {
            std::memcpy(pAdjust, fret, 4);
            std::memcpy(pAdjust + 4, fret + 1, 4);
        }
        else
        {
            std::memcpy(pAdjust, fret, 16);
        }
    }
    else if (nFreg == 1 && nGreg == 1)
    {
        if (pTypeDescr->nSize > 8)
        {
            std::memcpy(pAdjust, gret, 8);
            std::memcpy(pAdjust + 8, fret, 8);
        }
        else
        {
            std::memcpy(pAdjust, gret, 4);
            std::memcpy(pAdjust + 4, fret, 4);
        }
    }
    else
    {
        std::memcpy(pAdjust, gret, 16);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
