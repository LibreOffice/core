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

.IF "$(SYSTEM_LIBLANGTAG)" == "YES"
@all:
    @echo "Using system liblangtag."
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

CONFIGURE_DIR=.
BUILD_DIR=$(CONFIGURE_DIR)

my_misc='$(SRC_ROOT)$/$(PRJNAME)$/$(MISC)'
my_prefix='$(my_misc)$/install'
my_data='$(my_prefix)$/share/liblangtag'

CONFIGURE_FLAGS+= --prefix='$(my_prefix)' --libdir='$(my_prefix)/lib'

.IF "$(SYSTEM_LIBXML)"!="YES"
CONFIGURE_FLAGS+= LIBXML2_CFLAGS='-I$(SOLARINCDIR)$/external'
.IF "$(GUI)"=="WNT" && "$(COM)"!="GCC"
CONFIGURE_FLAGS+= LIBXML2_LIBS='$(SOLARLIBDIR)$/libxml2.lib'
.ELSE
CONFIGURE_FLAGS+= LIBXML2_LIBS='-L$(SOLARLIBDIR) -lxml2'
.ENDIF
.ENDIF

.IF "$(SYSTEM_GLIB)"!="YES"
# we're cheating here.. pkg-config wouldn't find anything useful, see configure patch
CONFIGURE_FLAGS+= LIBO_GLIB_CHEAT=YES
CONFIGURE_FLAGS+= GLIB_CFLAGS='-I$(SOLARINCDIR)/external/glib-2.0'
.IF "$(GUI)"=="WNT" && "$(COM)"!="GCC"
CONFIGURE_FLAGS+= GLIB_LIBS='$(SOLARLIBDIR)$/gio-2.0.lib $(SOLARLIBDIR)$/gobject-2.0.lib $(SOLARLIBDIR)$/gthread-2.0.lib $(SOLARLIBDIR)$/gmodule-2.0.lib $(SOLARLIBDIR)$/glib-2.0.lib'
.ELSE
CONFIGURE_FLAGS+= GLIB_LIBS='-L$(SOLARLIBDIR) -lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lgmodule-2.0 -lglib-2.0'
.ENDIF
CONFIGURE_FLAGS+= GLIB_GENMARSHAL=glib-genmarshal
CONFIGURE_FLAGS+= GLIB_MKENUMS=glib-mkenums
CONFIGURE_FLAGS+= GOBJECT_QUERY=gobject-query
CONFIGURE_FLAGS+= --disable-glibtest
.ENDIF

CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) .$/configure

BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) $(GNUMAKE) && \
			 $(AUGMENT_LIBRARY_PATH) $(GNUMAKE) install


.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"

CONFIGURE_FLAGS+= LDFLAGS=-Wl,--enable-runtime-pseudo-reloc-v2

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+= --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

.ELSE	# "$(COM)"=="GCC"

PATCH_FILES+=liblangtag-0.2-msc-configure.patch

.ENDIF	# "$(COM)"=="GCC"
.ENDIF	# "$(GUI)"=="WNT"


# --- Targets ------------------------------------------------------

ALLTAR: $(MISC)/liblangtag_data.zip

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

$(MISC)/liblangtag_data.zip: $(PACKAGE_DIR)/$(PREDELIVER_FLAG_FILE)
	@-rm -f $@
	@echo creating $@
	$(COMMAND_ECHO)cd $(my_data) && zip $(ZIP_VERBOSITY) -r $(my_misc)$/$(@:f) *
