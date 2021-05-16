/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sal/config.h>
#include <tools/toolsdllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <rtl/ustring.hxx>

namespace cpuid {

enum class InstructionSetFlags
{
    NONE  = 0x00,
    HYPER = 0x01,
    SSE2  = 0x02,
    SSSE3 = 0x04,
    SSE41 = 0x08,
    SSE42 = 0x10,
    AVX   = 0x20,
    AVX2  = 0x40,
    AVX512F = 0x100
};

} // end cpuid

namespace o3tl {
    template<> struct typed_flags<cpuid::InstructionSetFlags> : is_typed_flags<cpuid::InstructionSetFlags, 0x07f> {};
}

namespace cpuid {

/** Get supported instruction set flags determined at runtime by probing the CPU.
 */
TOOLS_DLLPUBLIC InstructionSetFlags getCpuInstructionSetFlags();

/** Check if a certain instruction set is supported by the CPU at runtime.
 */
TOOLS_DLLPUBLIC bool isCpuInstructionSetSupported(InstructionSetFlags eInstructions);

/** Returns a string of supported instructions.
 */
TOOLS_DLLPUBLIC OUString instructionSetSupportedString();

/** Check if SSE2 is supported by the CPU
 */
inline bool hasSSE2()
{
    return isCpuInstructionSetSupported(InstructionSetFlags::SSE2);
}

/** Check if SSSE3 is supported by the CPU
 */
inline bool hasSSSE3()
{
    return isCpuInstructionSetSupported(InstructionSetFlags::SSSE3);
}

/** Check if AVX is supported by the CPU
 */
inline bool hasAVX()
{
    return isCpuInstructionSetSupported(InstructionSetFlags::AVX);
}

/** Check if AVX2 is supported by the CPU
 */
inline bool hasAVX2()
{
    return isCpuInstructionSetSupported(InstructionSetFlags::AVX2);
}

/** Check if AVX512F is supported by the CPU
 */
inline bool hasAVX512F()
{
    return isCpuInstructionSetSupported(InstructionSetFlags::AVX512F);
}

/** Check if Hyper Threading is supported
 */
inline bool hasHyperThreading()
{
    return isCpuInstructionSetSupported(InstructionSetFlags::HYPER);
}

} // end cpuid

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
