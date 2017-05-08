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

#if defined(_MSC_VER)
#include <intrin.h>
void getCpuId(uint32_t array[4], uint32_t nInfoType)
{
    __cpuid(reinterpret_cast<int*>(array), nInfoType);
}
#else
#include <cpuid.h>
void getCpuId(uint32_t array[4], uint32_t nInfoType)
{
    __cpuid_count(nInfoType, 0, *(array + 0), *(array + 1), *(array + 2), *(array + 3));
}
#endif

// For AVX we need to check if OS has support for ymm registers
bool checkAVXSupportInOS()
{
    uint32_t xcr0;
#if defined(_MSC_VER)
    xcr0 = uint32_t(_xgetbv(0));
#else
    __asm__("xgetbv" : "=a" (xcr0) : "c" (0) : "%edx");
#endif
    return ((xcr0 & 6) == 6); /* checking if xmm and ymm state are enabled in XCR0 */
}

} // end anonymous namespace

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
        uint32_t abcd[] = {0, 0, 0, 0};
        getCpuId(abcd, 1);

        if ((abcd[3] & SSE2_bit) != 0)
            eInstructions |= InstructionSetFlags::SSE2;

        if ((abcd[2] & SSSE3_bit) != 0)
            eInstructions |= InstructionSetFlags::SSSE3;

        if ((abcd[2] & SSE41_bit ) != 0)
            eInstructions |= InstructionSetFlags::SSE41;

        if ((abcd[2] & SSE42_bit) != 0)
            eInstructions |= InstructionSetFlags::SSE42;

        if (((abcd[2] & AVX_bit)   != 0) &&
            ((abcd[2] & XSAVE_bit) != 0))
        {
            if (checkAVXSupportInOS())
            {
                eInstructions |= InstructionSetFlags::AVX;

                if (nLevel >= 7)
                {
                    uint32_t extendedInfo[] = {0, 0, 0, 0};
                    getCpuId(extendedInfo, 7);

                    if ((extendedInfo[1] & AVX2_bit) != 0)
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
        aString += OUString("SSE2 ");
    if (isCpuInstructionSetSupported(InstructionSetFlags::SSSE3))
        aString += OUString("SSSE3 ");
    if (isCpuInstructionSetSupported(InstructionSetFlags::SSE41))
        aString += OUString("SSE4.1 ");
    if (isCpuInstructionSetSupported(InstructionSetFlags::SSE42))
        aString += OUString("SSE4.2 ");
    if (isCpuInstructionSetSupported(InstructionSetFlags::AVX))
        aString += OUString("AVX ");
    if (isCpuInstructionSetSupported(InstructionSetFlags::AVX2))
        aString += OUString("AVX2 ");
    return aString;
}

} // end cpuid

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
