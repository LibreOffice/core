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


#include "dp_misc.h"
#include "dp_platform.hxx"
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/diagnose.h>

#define PLATFORM_ALL                "all"


namespace dp_misc
{
namespace
{
    struct StrOperatingSystem :
        public rtl::StaticWithInit<OUString, StrOperatingSystem> {
            const OUString operator () () {
                OUString os( "$_OS" );
                ::rtl::Bootstrap::expandMacros( os );
                return os;
            }
    };

    struct StrCPU :
        public rtl::StaticWithInit<OUString, StrCPU> {
            const OUString operator () () {
                OUString arch( "$_ARCH" );
                ::rtl::Bootstrap::expandMacros( arch );
                return arch;
            }
    };


    struct StrPlatform : public rtl::StaticWithInit<
        OUString, StrPlatform> {
            const OUString operator () () {
                OUStringBuffer buf;
                buf.append( StrOperatingSystem::get() );
                buf.append( '_' );
                buf.append( StrCPU::get() );
                return buf.makeStringAndClear();
            }
    };

    bool checkOSandCPU(OUString const & os, OUString const & cpu)
    {
        return (os == StrOperatingSystem::get())
            && (cpu == StrCPU::get());
    }

    bool isPlatformSupported( OUString const &  token )
    {
        bool ret = false;
        if (token == PLATFORM_ALL)
            ret = true;
        else if (token == "windows_x86")
            ret = checkOSandCPU("Windows", "x86");
        else if (token == "windows_x86_64")
            ret = checkOSandCPU("Windows", "x86_64");
        else if (token == "solaris_sparc")
            ret = checkOSandCPU("Solaris", "SPARC");
        else if (token == "solaris_sparc64")
            ret = checkOSandCPU("Solaris", "SPARC64");
        else if (token == "solaris_x86")
            ret = checkOSandCPU("Solaris", "x86");
        else if (token == "aix_powerpc")
            ret = checkOSandCPU("AIX", "PowerPC");
        else if (token == "macosx_powerpc")
            ret = checkOSandCPU("MacOSX", "PowerPC");
        else if (token == "macosx_x86")
            ret = checkOSandCPU("MacOSX", "x86");
        else if (token == "macosx_x86_64")
            ret = checkOSandCPU("MacOSX", "X86_64");
        else if (token == "linux_x86")
            ret = checkOSandCPU("Linux", "x86");
        else if (token == "linux_x86_64")
            ret = checkOSandCPU("Linux", "X86_64");
        else if (token == "linux_sparc")
            ret = checkOSandCPU("Linux", "SPARC");
        else if (token == "linux_sparc64")
            ret = checkOSandCPU("Linux", "SPARC64");
        else if (token == "linux_powerpc")
            ret = checkOSandCPU("Linux", "PowerPC");
        else if (token == "linux_powerpc64")
            ret = checkOSandCPU("Linux", "PowerPC_64");
        else if (token == "linux_powerpc64_le")
            ret = checkOSandCPU("Linux", "PowerPC_64_LE");
        else if (token == "linux_arm_eabi")
            ret = checkOSandCPU("Linux", "ARM_EABI");
        else if (token == "linux_arm_oabi")
            ret = checkOSandCPU("Linux", "ARM_OABI");
        else if (token == "linux_mips_el")
            ret = checkOSandCPU("Linux", "MIPS_EL");
        else if (token == "linux_mips64_el")
            ret = checkOSandCPU("Linux", "MIPS64_EL");
        else if (token == "linux_mips_eb")
            ret = checkOSandCPU("Linux", "MIPS_EB");
        else if (token == "linux_mips64_eb")
            ret = checkOSandCPU("Linux", "MIPS64_EB");
        else if (token == "linux_ia64")
            ret = checkOSandCPU("Linux", "IA64");
        else if (token == "linux_m68k")
            ret = checkOSandCPU("Linux", "M68K");
        else if (token == "linux_s390")
            ret = checkOSandCPU("Linux", "S390");
        else if (token == "linux_s390x")
            ret = checkOSandCPU("Linux", "S390x");
        else if (token == "linux_hppa")
            ret = checkOSandCPU("Linux", "HPPA");
        else if (token == "linux_alpha")
            ret = checkOSandCPU("Linux", "ALPHA");
        else if (token == "linux_aarch64")
            ret = checkOSandCPU("Linux", "AARCH64");
        else if (token == "freebsd_x86")
            ret = checkOSandCPU("FreeBSD", "x86");
        else if (token == "freebsd_x86_64")
            ret = checkOSandCPU("FreeBSD", "X86_64");
        else if (token == "freebsd_powerpc")
            ret = checkOSandCPU("FreeBSD", "PowerPC");
        else if (token == "kfreebsd_x86")
            ret = checkOSandCPU("kFreeBSD", "x86");
        else if (token == "kfreebsd_x86_64")
            ret = checkOSandCPU("kFreeBSD", "X86_64");
        else if (token == "netbsd_x86")
            ret = checkOSandCPU("NetBSD", "x86");
        else if (token == "netbsd_x86_64")
            ret = checkOSandCPU("NetBSD", "X86_64");
        else if (token == "openbsd_x86")
            ret = checkOSandCPU("OpenBSD", "x86");
        else if (token == "openbsd_x86_64")
            ret = checkOSandCPU("OpenBSD", "X86_64");
        else if (token == "dragonfly_x86")
            ret = checkOSandCPU("DragonFly", "x86");
        else if (token == "dragonfly_x86_64")
            ret = checkOSandCPU("DragonFly", "X86_64");
        else
        {
            OSL_FAIL("Extension Manager: The extension supports an unknown platform. "
            "Check the platform in the description.xml");
            ret = false;
        }
        return ret;
    }

} // anon namespace


OUString const & getPlatformString()
{
    return StrPlatform::get();
}

bool platform_fits( OUString const & platform_string )
{
    sal_Int32 index = 0;
    for (;;)
    {
        const OUString token(
            platform_string.getToken( 0, ',', index ).trim() );
        // check if this platform:
        if (token.equalsIgnoreAsciiCase( StrPlatform::get() ) ||
            (token.indexOf( '_' ) < 0 && /* check OS part only */
             token.equalsIgnoreAsciiCase( StrOperatingSystem::get() )))
        {
            return true;
        }
        if (index < 0)
            break;
    }
    return false;
}

bool hasValidPlatform( css::uno::Sequence<OUString> const & platformStrings)
{
    bool ret = false;
    for (sal_Int32 i  = 0; i < platformStrings.getLength(); i++)
    {
        if ( isPlatformSupported( platformStrings[i] ))
        {
            ret = true;
            break;
        }
    }
    return ret;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
