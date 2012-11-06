#
# Copyright 2012 LibreOffice contributors.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

PRJ=.

PRJNAME=liblangtag
TARGET=liblangtag

LIBLANGTAG_MAJOR=0
LIBLANGTAG_MINOR=4
LIBLANGTAG_MICRO=0
# Currently liblangtag.so.0.1.0 is generated instead of 0.2.0, presumably a bug?
# For new versions adapt symlink in prj/d.lst
# Version is currently hardcoded in scp2/source/ooo/file_library_ooo.scp
# section gid_File_Lib_Langtag, adapt for new versions, or introduce
# LIBLANGTAG_M* in configure!

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(ENABLE_LIBLANGTAG)" != "YES" || "$(SYSTEM_LIBLANGTAG)" == "YES"
@all:
	@echo "Not building liblangtag."
.ENDIF

TARFILE_NAME=liblangtag-$(LIBLANGTAG_MAJOR).$(LIBLANGTAG_MINOR).$(LIBLANGTAG_MICRO)
TARFILE_MD5=54e578c91b1b68e69c72be22adcb2195

PATCH_FILES=liblangtag-0.4.0-msvc-warning.patch
PATCH_FILES+=liblangtag-0.4.0-reg2xml-encoding-problem.patch

CONFIGURE_DIR=.
BUILD_DIR=$(CONFIGURE_DIR)
CONFIGURE_FLAGS= --disable-modules

.IF "$(OS)" == "MACOSX"
my_prefix = @.__________________________________________________$(EXTRPATH)
.ELSE
my_prefix = install-liblangtag
.END
my_misc = $(shell @pwd)/$(MISC)
my_install_sub = install
my_destdir = $(my_misc)/$(my_install_sub)
my_install = $(my_destdir)/$(my_prefix)
# relative to $(MISC)/build/liblangtag/ for OUT2LIB, OUT2INC
my_install_relative = ../../$(my_install_sub)/$(my_prefix)
my_data = $(my_install)/share/liblangtag

CONFIGURE_FLAGS+= --prefix='/$(my_prefix)' --libdir='/$(my_prefix)/lib'

# g-ir-scanner is dumb enough to turn CC=ccache gcc into trying
# to execve an app literal of "ccache gcc" including the space.
CONFIGURE_FLAGS+= --disable-introspection

.IF "$(SYSTEM_LIBXML)"!="YES"
CONFIGURE_FLAGS+= LIBXML2_CFLAGS='-I$(SOLARINCDIR)/external'
.IF "$(GUI)"=="WNT" && "$(COM)"!="GCC"
CONFIGURE_FLAGS+= LIBXML2_LIBS='$(SOLARLIBDIR)/libxml2.lib'
.ELSE
CONFIGURE_FLAGS+= LIBXML2_LIBS='-L$(SOLARLIBDIR) -lxml2'
.ENDIF
.ELSE
.IF "$(OS)" == "MACOSX"
CONFIGURE_FLAGS+= LIBXML2_CFLAGS='$(LIBXML_CFLAGS)' LIBXML2_LIBS='$(LIBXML_LIBS)'
.ENDIF
.ENDIF

CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) ./configure

BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) $(GNUMAKE) && \
			 $(AUGMENT_LIBRARY_PATH) $(GNUMAKE) install DESTDIR=$(my_destdir)

.IF "$(SYSTEM_LIBXML)"!="YES" || "$(SYSTEM_GLIB)"!="YES"
.IF "$(OS)"=="FREEBSD" || "$(OS)"=="LINUX"
CONFIGURE_FLAGS+= \
 LDFLAGS=-Wl,-z,origin\ -Wl,-rpath,\'\$$\$$ORIGIN:\$$\$$ORIGIN/../ure-link/lib\'
.ELIF "$(OS)"=="SOLARIS"
CONFIGURE_FLAGS+= LDFLAGS=-Wl,-R\'\$$\$$ORIGIN:\$$\$$ORIGIN/../ure-link/lib\'
.END
.END

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"

CONFIGURE_FLAGS+= LDFLAGS=-Wl,--enable-runtime-pseudo-reloc-v2

PATCH_FILES+=liblangtag-0.4.0-mingw.patch

.ELSE	# "$(COM)"=="GCC"

PATCH_FILES+=liblangtag-0.4.0-msc-configure.patch

.ENDIF	# "$(COM)"=="GCC"
.ENDIF	# "$(GUI)"=="WNT"

.IF "$(CROSS_COMPILING)"=="YES"
# There's a tool used only at build time to create the .xml file, and this does not work
# with cross-compiling. The file for this case is from a normal (non-cross) build.
PATCH_FILES+=liblangtag-0.4.0-cross.patch
CONFIGURE_FLAGS+= --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

OUT2INC += $(my_install_relative)/include/liblangtag/*

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2BIN += $(my_install_relative)/bin/liblangtag-0.dll
OUT2LIB += $(my_install_relative)/lib/liblangtag.dll.a
.ELSE
OUT2LIB += $(my_install_relative)/lib/langtag.lib*
.ENDIF
.ELSE
.IF "$(OS)" == "MACOSX"
OUT2LIB += $(my_install_relative)/lib/liblangtag*.dylib
.ELSE
OUT2LIB += $(my_install_relative)/lib/liblangtag.so*
.ENDIF
.ENDIF


# --- Targets ------------------------------------------------------

ALLTAR: $(MISC)/liblangtag_data.zip

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

$(MISC)/liblangtag_data.zip: $(PACKAGE_DIR)/$(PREDELIVER_FLAG_FILE)
	@-rm -f $@
	@echo creating $@
	$(COMMAND_ECHO)cd $(my_data) && zip $(ZIP_VERBOSITY) -r $(my_misc)/$(@:f) *
