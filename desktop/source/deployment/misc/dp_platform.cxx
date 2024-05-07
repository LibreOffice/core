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


#include <dp_platform.hxx>
#include <rtl/ustring.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/diagnose.h>
#include <o3tl/string_view.hxx>

constexpr OUStringLiteral PLATFORM_ALL = u"all";


namespace dp_misc
{
namespace
{
    OUString StrOperatingSystemInit()
    {
        OUString os( u"$_OS"_ustr );
        ::rtl::Bootstrap::expandMacros( os );
        return os;
    }

    const OUString & StrOperatingSystem()
    {
        static const OUString theOS = StrOperatingSystemInit();
        return theOS;
    };

    OUString StrCPUInit()
    {
        OUString arch( u"$_ARCH"_ustr );
        ::rtl::Bootstrap::expandMacros( arch );
        return arch;
    }

    const OUString & StrCPU()
    {
        static const OUString theCPU = StrCPUInit();
        return theCPU;
    };


    const OUString & StrPlatform()
    {
        static const OUString thePlatform = StrOperatingSystem() + "_" + StrCPU();
        return thePlatform;
    };

    bool checkOSandCPU(std::u16string_view os, std::u16string_view cpu)
    {
        return (os == StrOperatingSystem())
            && (cpu == StrCPU());
    }

    bool isPlatformSupported( std::u16string_view token )
    {
        bool ret = false;
        if (token == PLATFORM_ALL)
            ret = true;
        else if (token == u"windows_x86")
            ret = checkOSandCPU(u"Windows", u"x86");
        else if (token == u"windows_x86_64")
            ret = checkOSandCPU(u"Windows", u"X86_64");
        else if (token == u"windows_aarch64")
            ret = checkOSandCPU(u"Windows", u"AARCH64");
        else if (token == u"solaris_sparc")
            ret = checkOSandCPU(u"Solaris", u"SPARC");
        else if (token == u"solaris_sparc64")
            ret = checkOSandCPU(u"Solaris", u"SPARC64");
        else if (token == u"solaris_x86")
            ret = checkOSandCPU(u"Solaris", u"x86");
        else if (token == u"macosx_aarch64")
            ret = checkOSandCPU(u"MacOSX", u"AARCH64");
        else if (token == u"macosx_x86_64")
            ret = checkOSandCPU(u"MacOSX", u"X86_64");
        else if (token == u"linux_x86")
            ret = checkOSandCPU(u"Linux", u"x86");
        else if (token == u"linux_x86_64")
            ret = checkOSandCPU(u"Linux", u"X86_64");
        else if (token == u"linux_sparc")
            ret = checkOSandCPU(u"Linux", u"SPARC");
        else if (token == u"linux_sparc64")
            ret = checkOSandCPU(u"Linux", u"SPARC64");
        else if (token == u"linux_powerpc")
            ret = checkOSandCPU(u"Linux", u"PowerPC");
        else if (token == u"linux_powerpc64")
            ret = checkOSandCPU(u"Linux", u"PowerPC_64");
        else if (token == u"linux_powerpc64_le")
            ret = checkOSandCPU(u"Linux", u"PowerPC_64_LE");
        else if (token == u"linux_arm_eabi")
            ret = checkOSandCPU(u"Linux", u"ARM_EABI");
        else if (token == u"linux_arm_oabi")
            ret = checkOSandCPU(u"Linux", u"ARM_OABI");
        else if (token == u"linux_mips_el")
            ret = checkOSandCPU(u"Linux", u"MIPS_EL");
        else if (token == u"linux_mips64_el")
            ret = checkOSandCPU(u"Linux", u"MIPS64_EL");
        else if (token == u"linux_mips_eb")
            ret = checkOSandCPU(u"Linux", u"MIPS_EB");
        else if (token == u"linux_mips64_eb")
            ret = checkOSandCPU(u"Linux", u"MIPS64_EB");
        else if (token == u"linux_ia64")
            ret = checkOSandCPU(u"Linux", u"IA64");
        else if (token == u"linux_m68k")
            ret = checkOSandCPU(u"Linux", u"M68K");
        else if (token == u"linux_s390x")
            ret = checkOSandCPU(u"Linux", u"S390x");
        else if (token == u"linux_hppa")
            ret = checkOSandCPU(u"Linux", u"HPPA");
        else if (token == u"linux_alpha")
            ret = checkOSandCPU(u"Linux", u"ALPHA");
        else if (token == u"linux_aarch64")
            ret = checkOSandCPU(u"Linux", u"AARCH64");
        else if (token == u"linux_riscv64")
            ret = checkOSandCPU(u"Linux", u"RISCV64");
        else if (token == u"linux_loongarch64")
            ret = checkOSandCPU(u"Linux", u"LOONGARCH64");
        else if (token == u"freebsd_x86")
            ret = checkOSandCPU(u"FreeBSD", u"x86");
        else if (token == u"freebsd_x86_64")
            ret = checkOSandCPU(u"FreeBSD", u"X86_64");
        else if (token == u"freebsd_powerpc")
            ret = checkOSandCPU(u"FreeBSD", u"PowerPC");
        else if (token == u"freebsd_powerpc64")
            ret = checkOSandCPU(u"FreeBSD", u"PowerPC64");
        else if (token == u"kfreebsd_x86")
            ret = checkOSandCPU(u"kFreeBSD", u"x86");
        else if (token == u"kfreebsd_x86_64")
            ret = checkOSandCPU(u"kFreeBSD", u"X86_64");
        else if (token == u"netbsd_x86")
            ret = checkOSandCPU(u"NetBSD", u"x86");
        else if (token == u"netbsd_x86_64")
            ret = checkOSandCPU(u"NetBSD", u"X86_64");
        else if (token == u"openbsd_x86")
            ret = checkOSandCPU(u"OpenBSD", u"x86");
        else if (token == u"openbsd_x86_64")
            ret = checkOSandCPU(u"OpenBSD", u"X86_64");
        else if (token == u"dragonfly_x86")
            ret = checkOSandCPU(u"DragonFly", u"x86");
        else if (token == u"dragonfly_x86_64")
            ret = checkOSandCPU(u"DragonFly", u"X86_64");
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
    return StrPlatform();
}

bool platform_fits( std::u16string_view platform_string )
{
    sal_Int32 index = 0;
    for (;;)
    {
        const std::u16string_view token(
            o3tl::trim(o3tl::getToken(platform_string, 0, ',', index )) );
        // check if this platform:
        if (o3tl::equalsIgnoreAsciiCase( token, StrPlatform() ) ||
            (token.find( '_' ) == std::u16string_view::npos && /* check OS part only */
             o3tl::equalsIgnoreAsciiCase( token, StrOperatingSystem() )))
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
    for (const OUString& s : platformStrings)
    {
        if ( isPlatformSupported( s ) )
        {
            ret = true;
            break;
        }
    }
    return ret;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
