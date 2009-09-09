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
# $Revision: 1.15 $
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

PRJNAME=ooo_mozab
TARGET=ooo_mozab

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------
# ----- pkg-config start -------
.INCLUDE .IGNORE : pkgroot.mk
.IF "$(PKGCONFIG_ROOT)"!=""
PKG_CONFIG:=$(PKGCONFIG_ROOT)$/bin$/pkg-config
PKG_CONFIG_PATH:=$(PKGCONFIG_ROOT)$/lib$/pkgconfig
.IF "$(LD_LIBRARY_PATH)"=="" # assume empty implies unset
LD_LIBRARY_PATH!:=$(PKGCONFIG_ROOT)$/lib
.ELSE
LD_LIBRARY_PATH!:=$(LD_LIBRARY_PATH)$(PATH_SEPERATOR)$(PKGCONFIG_ROOT)$/lib
.ENDIF
.EXPORT : PKG_CONFIG_PATH PKG_CONFIG LD_LIBRARY_PATH
.ENDIF          # "$(PKGCONFIG_ROOT)"!=""

# reduce prerequisites by disabling mozilla binary
DISABLE_MOZ_EXECUTABLE=TRUE
.EXPORT : DISABLE_MOZ_EXECUTABLE

.IF "$(SYSBASE)"!="" && "$(OS)" == "LINUX"
## hmm... rather gcc specific switches...
CFLAGS:=-isystem $(SYSBASE)/usr/include -B$(SYSBASE)/usr/lib
LDFLAGS:= -Wl,--unresolved-symbols=ignore-in-shared-libs -L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -L$(SYSBASE)/usr/X11R6/lib
SYSBASE_X11:=--x-includes=$(SYSBASE)/usr/include/X11 --x-libraries=$(SYSBASE)/usr/X11R6/lib
MOZ_ENABLE_COREXFONTS=1
MOZILLA_CONFIGURE_FLAGS +=--disable-xft
.EXPORT : CFLAGS LDFLAGS MOZ_ENABLE_COREXFONTS
.ENDIF			# "$(SYSBASE)"!="" && "$(OS)" == "LINUX"

.IF "$(SYSBASE)"!="" && "$(OS)" == "MACOSX"
PKGCONFIG_ROOT!:=$(ENV_ROOT)$/macports-1.7.0
PKG_CONFIG:=$(PKGCONFIG_ROOT)$/bin$/pkg-config
PKG_CONFIG_PATH:=$(PKGCONFIG_ROOT)$/lib$/pkgconfig
.EXPORT : PKG_CONFIG_PATH PKG_CONFIG
# hmm... rather gcc specific switches...
CFLAGS:=-isystem $(SYSBASE)/usr/include -B$(SYSBASE)/usr/lib -B$(SYSBASE)/usr/lib/system -L$(ENV_ROOT)/macports-1.7.0/lib -lmathCommon
LDFLAGS:=-L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -L$(SYSBASE)/usr/lib/system
XLDOPTS:= -B$(SYSBASE)/usr/lib -B$(SYSBASE)/usr/lib/system -lmathCommon
.EXPORT : CFLAGS LDFLAGS XLDOPTS
.ENDIF			# "$(SYSBASE)"!="" && "$(OS)" == "MACOSX"

# ----- pkg-config end -------

MOZILLA_VERSION=1.1.14
TARFILE_NAME=seamonkey-$(MOZILLA_VERSION).source

TARFILE_ROOTDIR=mozilla
PATCH_FILES=seamonkey-source-$(MOZILLA_VERSION).patch

# This file is needed for the W32 build when BUILD_MOZAB is set
# (currently only vc8/vs2005 is supported when BUILD_MOZAB is set)
.IF "$(COM)"=="GCC"
LIBIDL_VC71_ZIPFILE_NAME*=vc71-libIDL-0.6.8-bin.zip
LIBGLIB_VC71_ZIPFILE_NAME*=vc71-glib-1.2.10-bin.zip
MOZTOOLS_ZIPFILE_NAME*=wintools.zip
.ELSE
MOZTOOLS_ZIPFILE_NAME*=vc8-moztools.zip
.ENDIF

ADDITIONAL_FILES=mailnews$/addrbook$/src$/nsAbMD5sum.cpp

CONFIGURE_DIR=
.IF "$(GUIBASE)"!="aqua"
MOZILLA_CONFIGURE_FLAGS += $(SYSBASE_X11)
.ENDIF

MOZILLA_CONFIGURE_FLAGS +=  --disable-tests \
                --enable-application=suite \
                --enable-ldap \
                --enable-crypto \
                --enable-optimize \
                --enable-strip \
                --disable-profilelocking \
                --disable-activex \
                --disable-activex-scripting \
                --disable-gnomevfs \
                --disable-debug \
                --disable-xprint \
                --disable-postscript \
                --without-system-zlib \
                --disable-installer \
                --disable-accessibility \
                --disable-xpfe-components \
                --disable-mathml \
                --disable-oji \
                --disable-profilesharing \
                --disable-boehm \
                --disable-jsloader \
                --disable-canvas \
                --disable-freetype2 \
                --disable-gnomeui \
                --disable-image-encoders \
                --disable-plugins \
                --disable-printing \
                --enable-extensions="pref"

#.IF "$(GUI)"!="WNT"
#MOZILLA_CONFIGURE_FLAGS += --enable-system-cairo
#.ENDIF

#disable profilelocking to share profile with mozilla
#disable activex and activex-scripting to remove the dependence of Microsoft_SDK\src\mfc\atlbase.h
#disable gnomevfs to remove the needed of gnome develop files
#disable others to save build times

.IF "$(GUI)"=="UNX"
.IF "$(GUIBASE)"=="aqua"
MACDEVSDK*=/Developer/SDKs/MacOSX10.4u.sdk
MOZILLA_CONFIGURE_FLAGS+= \
    --with-macos-sdk=$(MACDEVSDK) \
    --disable-glibtest \
    --enable-macos-target=10.4 \
    --disable-libxul
DEFAULT_MOZILLA_TOOLKIT=mac
.ELSE
#We do not need mozilla ui, but libIDL version are decided by default toolkit.
#default-toolkit=xlib need libIDL < 0.68
#default-toolkit=gtk2 need libIDL > 0.8 (know as libIDL2)
DEFAULT_MOZILLA_TOOLKIT*=gtk2
.ENDIF # "$(GUIBASE)"=="aqua"
MOZILLA_CONFIGURE_FLAGS+= --enable-default-toolkit=$(DEFAULT_MOZILLA_TOOLKIT)
.ENDIF

CONFIGURE_ACTION=sh -c "./configure $(MOZILLA_CONFIGURE_FLAGS)"

BUILD_DIR=
.IF "$(USE_SHELL)"!="4nt"
BUILD_ACTION:=$(GNUMAKE) -j$(EXTMAXPROCESS)
.ELSE
# This construct is needed because unitools.mk defines GNUMAKE using $ENV_TOOLS.
# $ENV_TOOLS doesn't exist for OOo builds and the cygwin make is needed.
BUILD_ACTION:=make
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(COMNAME)"=="sunpro5"
CXXFLAGS:=-features=tmplife
.IF "$(CPU)"=="U"
CXXFLAGS+=-m64
.ENDIF
.EXPORT : CXXFLAGS
.ENDIF          # "$(COMNAME)"=="sunpro5"
.ENDIF

.IF "$(OS)"=="SOLARIS" && "$(CPUNAME)"=="SPARC" && "$(CPU)"=="U"
PKG_CONFIG_PATH=/usr/lib/64/pkgconfig
.EXPORT: PKG_CONFIG_PATH
MAKE=/usr/sfw/bin/gmake
.EXPORT: MAKE
CFLAGS=-I/usr/sfw/include
.EXPORT: CFLAGS
.ENDIF
.IF "$(COM)"=="C52" && "$(CPUNAME)"=="SPARC" && "$(CPU)"=="U"
CFLAGS=-m64
ASFLAGS=-m64
.EXPORT: CFLAGS ASFLAGS
.ENDIF

MOZDIR=$(MISC)$/build$/seamonkey
MOZTARGET=$(OS)$(COM)$(CPU)

.IF "$(GUI)"=="WNT"
# "Our" build environment uses "NO" for the environment variable that
# indicate if system libraries should be used, the mozilla build uses
# "" in this case. This conflicts (at least for W32) with mozilla (1.7b)
# because it disables the library checks for msvc so that
# --without-system-* is not evaluated. To build the included libraries
# the affected variables have to be empty and not NO.
.IF "$(SYSTEM_ZLIB)"=="NO"
SYSTEM_ZLIB:=
.EXPORT : SYSTEM_ZLIB
.ENDIF
.IF "$(SYSTEM_JPEG)"=="NO"
SYSTEM_JPEG:=
.EXPORT : SYSTEM_JPEG
.ENDIF

# For W32-tcsh CC and CXX must not contain the wrapper, and W32-4nt ( in
# some cases ) doesn't work with DOS path notation when building mozilla.
.IF "$(COM)"=="GCC"
CC:=$(CC:s/guw.exe //:s/ -mno-cygwin//)
CXX:=$(CXX:s/guw.exe //:s/ -mno-cygwin//)
CPP:=$(CC) -E
LD:=ld
LIBS:=-lsupc++
.EXPORT : CPP LIBS
.ELSE
CC:=cl.exe
CXX:=cl.exe
.ENDIF

# Variables to install/use our own wintools
.IF "$(COM)"=="GCC"
MOZTOOLSUNPACK:=$(MISC)$/build$/moztoolsunpack
MOZTOOLSINST:=$(MISC)$/build$/moztoolsinst
.ELSE
MOZTOOLS_EXTRACT:=$(MISC)$/build$/moztools
.ENDIF
.IF "$(USE_SHELL)"!="4nt"
.IF "$(COM)"=="GCC"
MOZ_TOOLS_DOS:=$(shell @cygpath -ad "$(MISC)")\build\moztoolsinst
PATH!:=$(PATH):$(shell @cygpath $(MOZ_TOOLS_DOS))/bin:$(shell @cygpath $(MOZ_TOOLS_DOS))/vc71/bin
SET_MOZ_TOOLS_INSTALL_BAT:=export "MOZ_TOOLS=$(MOZ_TOOLS_DOS)"
.ELSE
MOZ_TOOLS_DOS:=$(shell @cygpath -ad "$(MISC)")\build\moztools\vc8-moztools
PATH!:=$(shell @cygpath $(MOZ_TOOLS_DOS))/bin:$(PATH)
.ENDIF
.ELSE # "$(USE_SHELL)"!="4nt"
# MOZ_TOOLS must contain an absolute path
MOZ_TOOLS_DOS:=$(shell @echo %@SFN[$(MISC)])\build\moztools\vc8-moztools
PATH!:=$(MOZ_TOOLS_DOS)\bin;$(PATH)
.ENDIF # "$(USE_SHELL)"!="4nt"

MOZ_TOOLS:=$(subst,\,/ $(MOZ_TOOLS_DOS))
.IF "$(COM)"=="GCC"
GLIB_PREFIX:=$(MOZ_TOOLS)/vc71
LIBIDL_PREFIX:=$(MOZ_TOOLS)/vc71
.EXPORT : GLIB_PREFIX LIBIDL_PREFIX
.ENDIF

.EXPORT : PATH MOZ_TOOLS
.ENDIF # "$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.IF "$(BUILD_MOZAB)"==""
all:
    @echo "Never Build Mozilla."
.ENDIF	

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.IF "$(GUI)"=="WNT"
NEEDWINTOOLSFLAGFILE:=$(MISC)$/build$/moztools.complete
.ENDIF # "$(GUI)"=="WNT"

ALLTAR: $(NEEDWINTOOLSFLAGFILE) \
    $(MISC)$/remove_build.flag \
    extract_mozab_files

$(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE) : $(MISC)$/remove_build.flag

# Since you never know what will be in a patch (for example, it may already
# patch at configure level), we remove the entire package directory if a patch
# is newer.
$(MISC)$/remove_build.flag : $(PATCH_FILES)
    $(REMOVE_PACKAGE_COMMAND)
    $(TOUCH) $(MISC)$/remove_build.flag

# Unpack/setup Windows build tools
.IF "$(GUI)"=="WNT"
$(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE) : $(MISC)$/build$/moztools.complete

.IF "$(COM)"=="GCC"
$(MISC)$/build$/moztools.unpack : $(PRJ)$/download$/$(MOZTOOLS_ZIPFILE_NAME)
    -$(RENAME) $(MOZTOOLSUNPACK) $(MOZTOOLSUNPACK)_removeme
    -$(RENAME) $(MOZTOOLSINST) $(MOZTOOLSINST)_removeme
    -rm -rf $(MOZTOOLSUNPACK)_removeme $(MOZTOOLSINST)_removeme
    @-$(MKDIRHIER) $(MOZTOOLSUNPACK)
    unzip $(PRJ)$/download$/$(MOZTOOLS_ZIPFILE_NAME) -d $(MOZTOOLSUNPACK)
    $(TOUCH) $(MISC)$/build$/moztools.unpack

$(MISC)$/build$/moztools.install : $(MISC)$/build$/moztools.unpack
    cd $(MOZTOOLSUNPACK)$/buildtools$/windows && $(SET_MOZ_TOOLS_INSTALL_BAT) && cmd /c install.bat
    $(TOUCH) $(MISC)$/build$/moztools.install

$(MISC)$/build$/moztools.complete : \
  $(MISC)$/build$/moztools.install \
  $(PRJ)$/download$/$(LIBIDL_VC71_ZIPFILE_NAME) \
  $(PRJ)$/download$/$(LIBGLIB_VC71_ZIPFILE_NAME)
    unzip $(PRJ)$/download$/$(LIBIDL_VC71_ZIPFILE_NAME) -d $(MOZTOOLSINST)
    unzip $(PRJ)$/download$/$(LIBGLIB_VC71_ZIPFILE_NAME) -d $(MOZTOOLSINST)
# chmod is also needed for W32-4nt build (when cygwin unzip is used)
    -chmod -R +x $(MOZTOOLSINST)$/vc71$/bin
    $(TOUCH) $(MISC)$/build$/moztools.complete
.ELSE
$(MISC)$/build$/moztools.unpack : $(PRJ)$/download$/$(MOZTOOLS_ZIPFILE_NAME)
    -$(RENAME) $(MOZTOOLS_EXTRACT) $(MOZTOOLS_EXTRACT)_removeme
    -rm -rf $(MOZTOOLS_EXTRACT)_removeme
    @-$(MKDIRHIER) $(MOZTOOLS_EXTRACT)
    unzip $(PRJ)$/download$/$(MOZTOOLS_ZIPFILE_NAME) -d $(MOZTOOLS_EXTRACT)
# chmod is also needed for W32-4nt build (when cygwin unzip is used)
    -chmod -R +x $(MOZTOOLS_EXTRACT)$/vc8-moztools$/bin
    $(TOUCH) $(MISC)$/build$/moztools.unpack

$(MISC)$/build$/moztools.complete : $(MISC)$/build$/moztools.unpack
    $(TOUCH) $(MISC)$/build$/moztools.complete
.ENDIF
.ENDIF # "$(GUI)"=="WNT"

zip:	\
    $(MISC)$/CREATETARBALL

.INCLUDE : extractfiles.mk
