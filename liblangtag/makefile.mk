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
LIBLANGTAG_MINOR=2
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

TARFILE_NAME=liblangtag-$(LIBLANGTAG_MAJOR).$(LIBLANGTAG_MINOR)
TARFILE_MD5=fa6a2f85bd28baab035b2c95e722713f

# liblangtag 132170753ea9cbd63cda8f3a80498c144f75b8ba
PATCH_FILES=liblangtag-0.2-0001-Fix-a-memory-leak.patch
# liblangtag cf8dfcf1604e534f4c9eccbd9a05571c8a9dc74d
PATCH_FILES+=liblangtag-0.2-0002-Fix-invalid-memory-access.patch
PATCH_FILES+=liblangtag-0.2-configure.patch
PATCH_FILES+=liblangtag-0.2-datadir.patch
PATCH_FILES+=liblangtag-0.2-msvc-warning.patch
PATCH_FILES+=liblangtag-0.2-reg2xml-encoding-problem.patch
PATCH_FILES+=liblangtag-0.2-xmlCleanupParser.patch
    # addressed upstream as <https://github.com/tagoh/liblangtag/pull/7> "Do not
    # call xmlCleanupParser from liblangtag"

CONFIGURE_DIR=.
BUILD_DIR=$(CONFIGURE_DIR)

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

.IF "$(SYSTEM_GLIB)"!="YES"
# we're cheating here.. pkg-config wouldn't find anything useful, see configure patch
CONFIGURE_FLAGS+= LIBO_GLIB_CHEAT=YES
CONFIGURE_FLAGS+= GLIB_CFLAGS='-I$(SOLARINCDIR)/external/glib-2.0'
.IF "$(GUI)"=="WNT" && "$(COM)"!="GCC"
CONFIGURE_FLAGS+= GLIB_LIBS='$(SOLARLIBDIR)/gio-2.0.lib $(SOLARLIBDIR)/gobject-2.0.lib $(SOLARLIBDIR)/gthread-2.0.lib $(SOLARLIBDIR)/gmodule-2.0.lib $(SOLARLIBDIR)/glib-2.0.lib'
.ELSE
CONFIGURE_FLAGS+= GLIB_LIBS='-L$(SOLARLIBDIR) -lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lgmodule-2.0 -lglib-2.0'
.IF "$(OS)" == "MACOSX"
EXTRA_LINKFLAGS+=-Wl,-dylib_file,@loader_path/libglib-2.0.0.dylib:$(SOLARLIBDIR)/libglib-2.0.0.dylib
.ENDIF
.ENDIF
CONFIGURE_FLAGS+= GLIB_GENMARSHAL=glib-genmarshal
CONFIGURE_FLAGS+= GLIB_MKENUMS=glib-mkenums
CONFIGURE_FLAGS+= GOBJECT_QUERY=gobject-query
CONFIGURE_FLAGS+= --disable-glibtest
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

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+= --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
PATCH_FILES+=liblangtag-0.2-mingw.patch
# There's a tool used only at build time to create the .xml file, and this does not work
# with cross-compiling. The file for this case is from a normal (non-cross) build.
PATCH_FILES+=liblangtag-0.2-mingw-genfile.patch
.ENDIF

.ELSE	# "$(COM)"=="GCC"

PATCH_FILES+=liblangtag-0.2-msc-configure.patch

.ENDIF	# "$(COM)"=="GCC"
.ENDIF	# "$(GUI)"=="WNT"


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
