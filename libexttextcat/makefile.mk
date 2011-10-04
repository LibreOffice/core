#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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

PRJ=.

PRJNAME=libexttextcat
TARGET=libexttextcat

.IF "$(SYSTEM_LIBEXTTEXTCAT)" == "YES"
all:
        @echo "An already available installation of libexttextcat should exist on your system."
        @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

# See http://cgit.freedesktop.org/libreoffice/libexttextcat/ for upstream
# sources, far better to commit your changes in there
TARFILE_NAME=libexttextcat-3.1.1
TARFILE_MD5=33f01c57f92eb6f940b59015c35a4eea
TARFILE_ROOTDIR=libexttextcat-3.1.1

.IF "$(GUI)"=="UNX"
#relative to CONFIGURE_DIR
CONFIGURE_ACTION=configure --disable-shared --with-pic CFLAGS="$(ARCH_FLAGS) $(EXTRA_CFLAGS)"
CONFIGURE_FLAGS=$(eq,$(OS),MACOSX CPPFLAGS="$(EXTRA_CDEFS)" $(NULL))
.IF "$(OS)"=="AIX"
CONFIGURE_FLAGS+= CFLAGS=-D_LINUX_SOURCE_COMPAT
.ENDIF
.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+= --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

BUILD_ACTION=make

OUT2LIB=$(BUILD_DIR)$/src$/.libs$/libexttextcat.a

.ENDIF # "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT"
BUILD_ACTION=cd src && dmake $(MAKEMACROS)
.ENDIF # "$(GUI)"=="WNT"

ALLTAR: $(BIN)/fingerprint.zip

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : tg_ext.mk
.INCLUDE : target.mk

$(BIN)/fingerprint.zip: $(PACKAGE_DIR)/$(PREDELIVER_FLAG_FILE)
	@-rm -f $@
	@echo creating ../../../../../$@
	$(COMMAND_ECHO)zip -j $(ZIP_VERBOSITY) $@ $(MISC)/build$/$(TARFILE_ROOTDIR)$/langclass$/fpdb.conf
	$(COMMAND_ECHO)find $(MISC)/build$/$(TARFILE_ROOTDIR)$/langclass$/LM \
		-name "*.lm" -print0 | \
                xargs -0 zip -j $(ZIP_VERBOSITY) $@
