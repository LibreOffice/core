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
#include <cstring>

namespace abi_riscv64
{
/*
   F: floating point reg
   G: general purpose reg
*/
enum class ReturnKind
{
    FF_Align4,
    FF_Align8,
    FG_Align4,
    FG_Align8,
    GF_Align4,
    GF_Align8,
    DEFAULT
};

void countnGreg(sal_Int32& nGreg, sal_Int32& nFreg, bool& firstIsGreg, sal_Int32& align,
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
                    nGreg, nFreg, firstIsGreg, align,
                    reinterpret_cast<typelib_CompoundTypeDescription const*>(childTypeDescr));
                TYPELIB_DANGER_RELEASE(childTypeDescr);
            }
            break;
            case typelib_TypeClass_DOUBLE:
                // Align to the larger type
                align = 8;
                [[fallthrough]];
            case typelib_TypeClass_FLOAT:
                nFreg++;
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                align = 8;
                [[fallthrough]];
            default:
                if (nFreg > 0)
                {
                    firstIsGreg = false;
                }
                nGreg++;
                break;
        }
    }
}

ReturnKind getReturnKind(const typelib_TypeDescription* pTypeDescr)
{
    sal_Int32 nGreg = 0;
    sal_Int32 nFreg = 0;
    sal_Int32 align = 4;
    bool firstIsGreg = true;
    countnGreg(nGreg, nFreg, firstIsGreg, align,
               reinterpret_cast<typelib_CompoundTypeDescription const*>(pTypeDescr));
    if (nGreg == 0 && nFreg == 2)
    {
        if (align == 4)
            return ReturnKind::FF_Align4;
        else
            return ReturnKind::FF_Align8;
    }
    else if (nGreg == 1 && nFreg == 1)
    {
        if (firstIsGreg)
        {
            if (align == 4)
                return ReturnKind::GF_Align4;
            else
                return ReturnKind::GF_Align8;
        }
        else
        {
            if (align == 4)
                return ReturnKind::FG_Align4;
            else
                return ReturnKind::FG_Align8;
        }
    }
    else
    {
        return ReturnKind::DEFAULT;
    }
}

/*
    Transform the returned cpp data to uno.
    This happens at the end of uno2cpp, when callee cpp func finished.

                  |    returned data saved in
    default cases |     gret[0] and gret[1]
    2 float       |     fret[0] and fret[1]
    1 float 1 int |     gret[0] and fret[0]

    There is a complex problem -- alignment. For example, 4 byte float and 8 byte
    integer take 16 bytes rather than 12 bytes.

    There is also another complex problem. e.g. Two 4 byte integer is compacted
    in a0, but two 4 byte float is seperately set in fa0 and fa1. However, return
    size is 8 bytes. We need to cut the lower 32bit of fa0 and fa1 seperately and
    combine them in 8 bytes.
*/
void fillUNOStruct(const typelib_TypeDescription* pTypeDescr, sal_Int64* gret, double* fret,
                   void* pRegisterReturn)
{
    BRIDGE_LOG("In fillStruct, pTypeDescr = %p, gret = %p, fret = %p, pRegisterReturn = %p\n",
               pTypeDescr, gret, fret, pRegisterReturn);
    ReturnKind returnKind = getReturnKind(pTypeDescr);
    switch (returnKind)
    {
        case ReturnKind::FF_Align4:
            memcpy(reinterpret_cast<char*>(pRegisterReturn), &(fret[0]), 4);
            memcpy(reinterpret_cast<char*>(pRegisterReturn) + 4, &(fret[1]), 4);
            break;
        case ReturnKind::FF_Align8:
            reinterpret_cast<double*>(pRegisterReturn)[0] = fret[0];
            reinterpret_cast<double*>(pRegisterReturn)[1] = fret[1];
            break;
        case ReturnKind::FG_Align4:
            memcpy(reinterpret_cast<char*>(pRegisterReturn), &(fret[0]), 4);
            memcpy(reinterpret_cast<char*>(pRegisterReturn) + 4, &(gret[0]), 4);
            break;
        case ReturnKind::FG_Align8:
            reinterpret_cast<double*>(pRegisterReturn)[0] = fret[0];
            reinterpret_cast<sal_Int64*>(pRegisterReturn)[1] = gret[0];
            break;
        case ReturnKind::GF_Align4:
            memcpy(reinterpret_cast<char*>(pRegisterReturn), &(gret[0]), 4);
            memcpy(reinterpret_cast<char*>(pRegisterReturn) + 4, &(fret[0]), 4);
            break;
        case ReturnKind::GF_Align8:
            reinterpret_cast<sal_Int64*>(pRegisterReturn)[0] = gret[0];
            reinterpret_cast<double*>(pRegisterReturn)[1] = fret[0];
            break;
        default:
            reinterpret_cast<sal_Int64*>(pRegisterReturn)[0] = gret[0];
            reinterpret_cast<sal_Int64*>(pRegisterReturn)[1] = gret[1];
            break;
    }
}

/*
    Split zipped unoreturn to cpp func. This happens at the end of cpp2uno.

    The data in pTarget will be extrat to return regs in privateSnippetExecutor:
                  |  pTarget[0]  |  pTarget[1]  | return type |
    default cases |     $a0      |     $a1      |      0      |
    2 float       |     $fa0     |     $fa1     |      0      |
    1 float 1 int |     $a0      |     $fa0     |      1      |

    This looks like a reverse version of fillUNOStruct. The reason for such
    "meaningless" effort is that java return a compact struct, but cpp not.
*/
void splitUNOStruct(const typelib_TypeDescription* pTypeDescr, sal_uInt64* pTarget,
                    sal_uInt64* pSource, sal_Int32& returnType)
{
    BRIDGE_LOG("In splitUNOStruct, pTypeDescr = %p, pTarget = %p, pSource = %p\n", pTypeDescr,
               pTarget, pSource);
    sal_uInt64* pTemp = (sal_uInt64*)calloc(2, sizeof(sal_uInt64));
    ReturnKind returnKind = getReturnKind(pTypeDescr);
    switch (returnKind)
    {
        case ReturnKind::FF_Align4:
            memcpy(reinterpret_cast<char*>(pTemp), reinterpret_cast<char*>(pSource), 4);
            memset(reinterpret_cast<char*>(pTemp) + 4, 0xFF, 4);
            memcpy(reinterpret_cast<char*>(pTemp) + 8, reinterpret_cast<char*>(pSource) + 4, 4);
            memset(reinterpret_cast<char*>(pTemp) + 12, 0xFF, 4);
            returnType = 0;
            break;
        case ReturnKind::FF_Align8:
            pTemp[0] = pSource[0];
            pTemp[1] = pSource[1];
            returnType = 0;
            break;
        case ReturnKind::FG_Align4:
            memcpy(reinterpret_cast<char*>(pTemp), reinterpret_cast<char*>(pSource) + 4, 4);
            memcpy(reinterpret_cast<char*>(pTemp) + 8, reinterpret_cast<char*>(pSource), 4);
            memset(reinterpret_cast<char*>(pTemp) + 12, 0xFF, 4);
            returnType = 1;
            break;
        case ReturnKind::FG_Align8:
            pTemp[0] = pSource[1];
            pTemp[1] = pSource[0];
            returnType = 1;
            break;
        case ReturnKind::GF_Align4:
            memcpy(reinterpret_cast<char*>(pTemp), reinterpret_cast<char*>(pSource), 4);
            memcpy(reinterpret_cast<char*>(pTemp) + 8, reinterpret_cast<char*>(pSource) + 4, 4);
            memset(reinterpret_cast<char*>(pTemp) + 12, 0xFF, 4);
            returnType = 1;
            break;
        case ReturnKind::GF_Align8:
            pTemp[0] = pSource[0];
            pTemp[1] = pSource[1];
            returnType = 1;
            break;
        default:
            pTemp[0] = pSource[0];
            pTemp[1] = pSource[1];
            returnType = 0;
            break;
    }
    pTarget[0] = pTemp[0];
    pTarget[1] = pTemp[1];
    free(pTemp);
}

/*
    Extend higher bits for integer types.

    According to
    https://github.com/riscv-non-isa/riscv-elf-psabi-doc/blob/master/riscv-cc.adoc#integer-calling-convention

    > When passed in registers or on the stack, integer scalars narrower than XLEN bits
    > are widened according to the sign of their type up to 32 bits, then sign-extended
    > to XLEN bits.
*/
void extIntBits(sal_uInt64* outData, const sal_uInt64* inData, bool isSigned, sal_uInt32 dataBytes)
{
    if (dataBytes > 8)
    {
        //SAL_WARN("bridges", "illegal dataBytes in dataBytes, please check the bridge.");
        return;
    }

    sal_uInt64 data = *inData;
    char* dataPointer = reinterpret_cast<char*>(&data);

    // Clear bits which are not data
    sal_uInt64* dataMask = (sal_uInt64*)calloc(1, 8);
    memset(dataMask, 0xFF, dataBytes);
    data = data & *dataMask;
    free(dataMask);

    // extend to 32 bit
    if (dataBytes < 4)
    {
        if (isSigned)
        {
            // Detect the highest bit of the data.
            // For example, if a one-byte integer data passed in, we need to detect the 8th bit(8 x 1)
            // So left shift 1 three-times(8-1) we can get mask 1000 0000
            sal_uInt64 detectMask = 1 << (dataBytes * 8 - 1);

            if (detectMask & data)
                // Is negative
                memset(dataPointer + dataBytes, 0xFF, 4 - dataBytes);
            else
                // Is positive
                memset(dataPointer + dataBytes, 0x0, 4 - dataBytes);
        }
        else
            memset(dataPointer + dataBytes, 0x0, 4 - dataBytes);

        // The highest data bit turns into 8 * 4 = 32 bit
        dataBytes = 4;
    }

    // Sign extend to 64 bit
    sal_uInt64 detectMask = 1 << (dataBytes * 8 - 1);
    if (detectMask & data)
        memset(dataPointer + dataBytes, 0xFF, 8 - dataBytes);
    else
        memset(dataPointer + dataBytes, 0x00, 8 - dataBytes);

    *outData = data;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
