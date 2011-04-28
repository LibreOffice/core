#!/bin/sh
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2009 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# if no environment is set, try to find one, and source it
# user can override auto-detection via SOLAR_SHELL_ENVIRONMENT
# it is not currently possible to handle corner cases like you have an
# AMD64 linux box, create configure environments for both x86 and x86_64,
# and then expect to auto-detect both.
# use SOLAR_SHELL_ENVIRONMENT in such cases.

me=source_soenv.sh

if test -n "${SOLARENV}"; then
    echo "$me: error: only call this if you do not have an environment set!"
    exit 1
fi

if test -n "${SOLAR_SHELL_ENVIRONMENT}"; then
    if test -f "${SOLAR_SHELL_ENVIRONMENT}"; then
        . "${SOLAR_SHELL_ENVIRONMENT}"
    else
        echo "$me: error: \$SOLAR_SHELL_ENVIRONMENT set, but the file does not exist"
        exit 1
    fi
else
    config_guess_result=`./config.guess`
    case "${config_guess_result}" in
        i[3456]86-*-solaris*)
            . ./SolarisX86Env.Set.sh
        ;;
        sparc-*-solaris*)
            . ./SolarisSparcEnv.Set.sh
        ;;
        i[3456]86-*-linux*)
            . ./LinuxX86Env.Set.sh
        ;;
        x86_64-*-linux*)
            . ./LinuxX86-64Env.Set.sh
        ;;
        ia64-*-linux*)
            . ./LinuxIA64Env.Set.sh
        ;;
        sparc-*-linux*)
            . ./LinuxSparcEnv.Set.sh
        ;;
        powerpc-*-linux*)
            . ./LinuxPPCEnv.Set.sh
        ;;
        powerpc64-*-linux*)
            . ./LinuxPPC64Env.Set.sh
        ;;
        s390-*-linux*)
            . ./LinuxS390Env.Set.sh
        ;;
        s390x-*-linux*)
            . ./LinuxS390XEnv.Set.sh
        ;;
        m68k-*-linux*)
            . ./LinuxM68KEnv.Set.sh
        ;;
        hppa-*-linux*)
            . ./LinuxHPPAEnv.Set.sh
        ;;
        alpha-*-linux*)
            . ./LinuxAlphaEnv.Set.sh
        ;;
        arm*-*-linux*)
            . ./LinuxARMEnv.Set.sh
        ;;
        mips-*-linux*)
            . ./LinuxMIPSEnv.Set.sh
        ;;
        i386-*-freebsd*)
            . ./FreeBSDX86Env.Set.sh
        ;;
        amd64-*-freebsd*)
            . ./FreeBSDAMDEnv.Set.sh
        ;;
        # the result here (whether you get x86_64) seems to depend on $CC...
        i[3456]86-*-darwin*|x86_64-*-darwin*)
            . ./MacOSXX86Env.Set.sh
        ;;
        powerpc-*-darwin*)
            . ./MacOSXPPCEnv.Set.sh
        ;;
        *-cygwin*)
            . ./winenv.set.sh || . ./winmingw.set.sh
        ;;
        *)
            echo "$me: error: unknown *Env.Set.sh for: ${config_guess_result}"
            exit 1
        ;;
    esac
fi
if test -z "${SOLARENV}"; then
    echo "$me: error: No environment set!"
    exit 1
fi

