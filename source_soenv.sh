#!/bin/sh
###############################################################
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
###############################################################

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
        amd64-*-freebsd*|x86_64-*-freebsd*)
            . ./FreeBSDAMDEnv.Set.sh
        ;;
        arm*-*-freebsd*)
            . ./FreeBSDARMEnv.Set.sh
        ;;
        powerpc-*-freebsd*)
            . ./FreeBSDPPCEnv.Set.sh
        ;;
        powerpc64-*-freebsd*)
            . ./FreeBSDPPC64Env.Set.sh
        ;;
        # the result here (whether you get x86_64) seems to depend on $CC...
        i[3456]86-*-darwin*|x86_64-*-darwin*)
            . ./MacOSXX64Env.Set.sh
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

