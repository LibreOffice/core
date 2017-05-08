/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_TOOLS_CPUID_HXX
#define INCLUDED_TOOLS_CPUID_HXX

#include <sal/config.h>
#include <tools/toolsdllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <rtl/ustring.hxx>

namespace cpuid {

enum class InstructionSetFlags
{
    NONE  = 0x00,
    SSE2  = 0x01,
    SSSE3 = 0x02,
    SSE41 = 0x04,
    SSE42 = 0x08,
    AVX   = 0x10,
    AVX2  = 0x20,
};

} // end cpuid

namespace o3tl {
    template<> struct typed_flags<cpuid::InstructionSetFlags> : is_typed_flags<cpuid::InstructionSetFlags, 0x03f> {};
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

/** Check if SSE2 can be used in code
 */
inline bool hasSSE2()
{
    return isCpuInstructionSetSupported(InstructionSetFlags::SSE2);
}

/** Check if SSSE3 can be used in code
 */
inline bool hasSSSE3()
{
    return isCpuInstructionSetSupported(InstructionSetFlags::SSSE3);
}

/** Check if AVX2 can be used in code
 */
inline bool hasAVX2()
{
    return isCpuInstructionSetSupported(InstructionSetFlags::AVX2);
}

} // end cpuid

#endif // INCLUDED_TOOLS_CPUID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
