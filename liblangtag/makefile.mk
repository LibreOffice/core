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
# currently liblangtag.so.0.1.0 is generated, presumably a bug?
LIBLANGTAG_LIBMINOR=1

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

CONFIGURE_DIR=.
CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) .$/configure

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) $(GNUMAKE) -j$(EXTMAXPROCESS)

.IF "$(GUI)"=="UNX"

OUT2LIB= \
	$(BUILD_DIR)$/liblangtag/.libs$/$(TARGET)$(DLLPOST).$(LIBLANGTAG_MAJOR).$(LIBLANGTAG_LIBMINOR).$(LIBLANGTAG_MICRO) \
    $(BUILD_DIR)$/liblangtag/.libs$/$(TARGET)$(DLLPOST).$(LIBLANGTAG_MAJOR) \
    $(BUILD_DIR)$/liblangtag/.libs$/$(TARGET)$(DLLPOST)

.ENDIF

.IF "$(GUI)"=="WNT"
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk
