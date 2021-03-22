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
#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/diagnose.h>

#define PLATFORM_ALL                u"all"


namespace dp_misc
{
namespace
{
    struct StrOperatingSystem :
        public rtl::StaticWithInit<OUString, StrOperatingSystem> {
             OUString operator () () {
                OUString os( "$_OS" );
                ::rtl::Bootstrap::expandMacros( os );
                return os;
            }
    };

    struct StrCPU :
        public rtl::StaticWithInit<OUString, StrCPU> {
            OUString operator () () {
                OUString arch( "$_ARCH" );
                ::rtl::Bootstrap::expandMacros( arch );
                return arch;
            }
    };


    struct StrPlatform : public rtl::StaticWithInit<
        OUString, StrPlatform> {
            OUString operator () () {
                OUStringBuffer buf;
                buf.append( StrOperatingSystem::get() );
                buf.append( '_' );
                buf.append( StrCPU::get() );
                return buf.makeStringAndClear();
            }
    };

    bool checkOSandCPU(std::u16string_view os, std::u16string_view cpu)
    {
        return (os == StrOperatingSystem::get())
            && (cpu == StrCPU::get());
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
        else if (token == u"solaris_sparc")
            ret = checkOSandCPU(u"Solaris", u"SPARC");
        else if (token == u"solaris_sparc64")
            ret = checkOSandCPU(u"Solaris", u"SPARC64");
        else if (token == u"solaris_x86")
            ret = checkOSandCPU(u"Solaris", u"x86");
        else if (token == u"aix_powerpc")
            ret = checkOSandCPU(u"AIX", u"PowerPC");
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
        else if (token == u"linux_s390")
            ret = checkOSandCPU(u"Linux", u"S390");
        else if (token == u"linux_s390x")
            ret = checkOSandCPU(u"Linux", u"S390x");
        else if (token == u"linux_hppa")
            ret = checkOSandCPU(u"Linux", u"HPPA");
        else if (token == u"linux_alpha")
            ret = checkOSandCPU(u"Linux", u"ALPHA");
        else if (token == u"linux_aarch64")
            ret = checkOSandCPU(u"Linux", u"AARCH64");
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
