/*
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <skia_compiler.hxx>

// Get the type of compiler that Skia is compiled with.
const char* skia_compiler_name()
{
#if defined __clang__
    return "Clang";
#elif defined __GNUC__
    return "GCC";
#elif defined _MSC_VER
    return "MSVC";
#else
    return "?";
#endif
}
