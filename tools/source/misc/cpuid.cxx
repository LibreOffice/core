/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/cpuid.hxx>
#include <cstdint>

namespace cpuid {

namespace {

#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_AMD64))
#include <intrin.h>
void getCpuId(uint32_t array[4], uint32_t nInfoType)
{
    __cpuid(reinterpret_cast<int*>(array), nInfoType);
}
#elif (defined(__i386__) || defined(__x86_64__))
#include <cpuid.h>
void getCpuId(uint32_t array[4], uint32_t nInfoType)
{
    __cpuid_count(nInfoType, 0, *(array + 0), *(array + 1), *(array + 2), *(array + 3));
}
#else
void getCpuId(uint32_t array[4], uint32_t /*nInfoType*/)
{
   array[0] = array[1] =  array[2] = array[3] = 0;
}
#endif

// For AVX we need to check if OS has support for ymm registers
bool checkAVXSupportInOS()
{
    uint32_t xcr0 = 0;
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_AMD64))
    xcr0 = uint32_t(_xgetbv(0));
#elif (defined(__i386__) || defined(__x86_64__))
    __asm__("xgetbv" : "=a" (xcr0) : "c" (0) : "%edx");
#endif
    return ((xcr0 & 6) == 6); /* checking if xmm and ymm state are enabled in XCR0 */
}

} // end anonymous namespace

#define HYPER_bit (1 << 28)
#define SSE2_bit  (1 << 26)
#define SSSE3_bit (1 <<  9)
#define SSE41_bit (1 << 19)
#define SSE42_bit (1 << 20)
#define XSAVE_bit (1 << 27)
#define AVX_bit   (1 << 28)
#define AVX2_bit  (1 << 5)

InstructionSetFlags getCpuInstructionSetFlags()
{
    InstructionSetFlags eInstructions = InstructionSetFlags::NONE;

    uint32_t info[] = {0, 0, 0, 0};
    getCpuId(info, 0);
    int nLevel = info[0];

    if (nLevel >= 1)
    {
        uint32_t aCpuInfoArray[] = {0, 0, 0, 0};
        getCpuId(aCpuInfoArray, 1);

        if ((aCpuInfoArray[3] & HYPER_bit) != 0)
            eInstructions |= InstructionSetFlags::HYPER;

        if ((aCpuInfoArray[3] & SSE2_bit) != 0)
            eInstructions |= InstructionSetFlags::SSE2;

        if ((aCpuInfoArray[2] & SSSE3_bit) != 0)
            eInstructions |= InstructionSetFlags::SSSE3;

        if ((aCpuInfoArray[2] & SSE41_bit ) != 0)
            eInstructions |= InstructionSetFlags::SSE41;

        if ((aCpuInfoArray[2] & SSE42_bit) != 0)
            eInstructions |= InstructionSetFlags::SSE42;

        if (((aCpuInfoArray[2] & AVX_bit)   != 0) &&
            ((aCpuInfoArray[2] & XSAVE_bit) != 0))
        {
            if (checkAVXSupportInOS())
            {
                eInstructions |= InstructionSetFlags::AVX;

                if (nLevel >= 7)
                {
                    uint32_t aExtendedInfo[] = {0, 0, 0, 0};
                    getCpuId(aExtendedInfo, 7);

                    if ((aExtendedInfo[1] & AVX2_bit) != 0)
                        eInstructions |= InstructionSetFlags::AVX2;
                }
            }
        }
    }

    return eInstructions;
}

bool isCpuInstructionSetSupported(InstructionSetFlags eInstructions)
{
    static InstructionSetFlags eCPUFlags = getCpuInstructionSetFlags();
    return (eCPUFlags & eInstructions) == eInstructions;
}

OUString instructionSetSupportedString()
{
    OUString aString;
    if (isCpuInstructionSetSupported(InstructionSetFlags::SSE2))
        aString += "SSE2 ";
    if (isCpuInstructionSetSupported(InstructionSetFlags::SSSE3))
        aString += "SSSE3 ";
    if (isCpuInstructionSetSupported(InstructionSetFlags::SSE41))
        aString += "SSE4.1 ";
    if (isCpuInstructionSetSupported(InstructionSetFlags::SSE42))
        aString += "SSE4.2 ";
    if (isCpuInstructionSetSupported(InstructionSetFlags::AVX))
        aString += "AVX ";
    if (isCpuInstructionSetSupported(InstructionSetFlags::AVX2))
        aString += "AVX2 ";
    return aString;
}

} // end cpuid

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
