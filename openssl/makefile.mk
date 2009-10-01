#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.26 $
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

PRJ=.

PRJNAME=openssl
TARGET=openssl

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_OPENSSL)" == "YES"
@all:
    @echo "Using system openssl...."
.ENDIF

.IF "$(DISABLE_OPENSSL)" == "TRUE"
@all:
    @echo "openssl disabled...."
.ENDIF

OPENSSL_NAME=openssl-0.9.8k

TARFILE_NAME=$(OPENSSL_NAME)

CONFIGURE_DIR=.
CONFIGURE_ACTION=config
CONFIGURE_FLAGS=-I$(SYSBASE)$/usr$/include -L$(SYSBASE)$/usr$/lib shared 

BUILD_DIR=.
BUILD_ACTION=make CC='$(CC)'

OUT2LIB = libssl.*
OUT2LIB += libcrypto.*
OUT2INC += include/openssl/*

.IF "$(OS)" == "LINUX" || "$(OS)" == "FREEBSD"
    PATCH_FILES=openssllnx.patch
    ADDITIONAL_FILES:= \
        libcrypto_OOo_0_9_8k.map \
        libssl_OOo_0_9_8k.map
    .IF "$(CPU)" == "I"
        CONFIGURE_ACTION=Configure linux-elf
    .ELIF "$(CPU)" == "X"
        CONFIGURE_ACTION=Configure linux-x86_64
    .ELIF "$(BUILD64)" == "1"
        CONFIGURE_ACTION=Configure linux-generic64
    .ELSE
        CONFIGURE_ACTION=Configure linux-generic32
    .ENDIF
    # if you build openssl as shared library you have to patch the Makefile.Shared "LD_LIBRARY_PATH=$$LD_LIBRARY_PATH \"
    #BUILD_ACTION=make 'SHARED_LDFLAGS=-Wl,--version-script=./lib$$(SHLIBDIRS)_OOo_0_9_8e.map'
.ENDIF

.IF "$(OS)" == "SOLARIS"
    PATCH_FILES=opensslsol.patch
    ADDITIONAL_FILES:= \
        libcrypto_OOo_0_9_8k.map \
        libssl_OOo_0_9_8k.map
    #BUILD_ACTION=make 'SHARED_LDFLAGS=-G -dy -z text -M./lib$$$$$$$$(SHLIBDIRS)_OOo_0_9_8e.map'

    # Use BUILD64 when 1 to select new specific 64bit Configurations if necessary

    .IF "$(CPUNAME)" == "INTEL" # Solaris INTEL
        .IF "$(CPU)" == "X"
           CONFIGURE_ACTION=Configure solaris64-x86_64-cc
        .ELSE
           CONFIGURE_ACTION=Configure solaris-x86-cc
        .ENDIF
    .ELIF "$(CPU)" == "U" # Solaris SPARC
       CONFIGURE_ACTION=Configure solaris64-sparcv9-cc
    .ELSE
       CONFIGURE_ACTION=Configure solaris-sparcv9-cc
    .ENDIF
.ENDIF

.IF "$(OS)" == "WNT"

.IF "$(COM)"=="GCC"
PATCH_FILES=opensslmingw.patch
.IF "$(USE_MINGW)" == "cygwin"
CONFIGURE_ACTION=$(PERL) configure
CONFIGURE_FLAGS=mingw shared 
INSTALL_ACTION=mv libcrypto.a libcrypto_static.a && mv libcrypto.dll.a libcrypto.a && mv libssl.a libssl_static.a && mv libssl.dll.a libssl.a
OUT2LIB = libcrypto_static.*
OUT2LIB += libssl_static.*
OUT2LIB += libcrypto.*
OUT2LIB += libssl.*
OUT2BIN = ssleay32.dll
OUT2BIN += libeay32.dll
.ELSE
CONFIGURE_ACTION=
BUILD_ACTION=cmd /c "ms\mingw32"
OUT2LIB = out/libcrypto_static.*
OUT2LIB += out/libssl_static.*
OUT2LIB += out/libcrypto.*
OUT2LIB += out/libssl.*
OUT2BIN = out/ssleay32.dll
OUT2BIN += out/libeay32.dll
.ENDIF
.ELSE

        PATCH_FILES=openssl.patch
        .IF "$(MAKETARGETS)" == ""
            # The env. vars CC and PERL are used by nmake, and nmake insists on '\'s
            # If WRAPCMD is set it is prepended before the compiler, don't touch that.
            .IF "$(WRAPCMD)"==""
                CC!:=$(subst,/,\ $(normpath,1 $(CC)))
                .EXPORT : CC
            .ENDIF
            PERL_bak:=$(PERL)
            PERL!:=$(subst,/,\ $(normpath,1 $(PERL)))
            .EXPORT : PERL
            PERL!:=$(PERL_bak)
        .ENDIF

        #CONFIGURE_ACTION=cmd /c $(PERL:s!\!/!) configure
        CONFIGURE_ACTION=$(PERL) configure
        CONFIGURE_FLAGS=VC-WIN32
        BUILD_ACTION=cmd /c "ms$(EMQ)\do_ms.bat $(subst,/,\ $(normpath,1 $(PERL)))" && nmake -f ms/ntdll.mak

        OUT2LIB = out32dll$/ssleay32.lib
        OUT2LIB += out32dll$/libeay32.lib
        OUT2BIN = out32dll$/ssleay32.dll
        OUT2BIN += out32dll$/libeay32.dll
        OUT2INC = inc32$/openssl$/*
    .ENDIF
.ENDIF

#set INCLUDE=D:\sol_temp\n\msvc7net3\PlatformSDK\include;D:\sol_temp\n\msvc7net3\include\ && set path=%path%;D:\sol_temp\r\btw\SRC680\perl\bin &&

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

