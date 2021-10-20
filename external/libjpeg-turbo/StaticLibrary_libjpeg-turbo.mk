# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozillarg/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,libjpeg-turbo))

$(eval $(call gb_StaticLibrary_use_unpacked,libjpeg-turbo,libjpeg-turbo))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,libjpeg-turbo))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_cflags,libjpeg-turbo,\
    -DHAVE_INTRIN_H=1 \
))
endif

$(eval $(call gb_Library_add_cflags,libjpeg-turbo,\
    -DSIZEOF_SIZE_T=$(SIZEOF_SIZE_T) \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,libjpeg-turbo,\
    UnpackedTarball/libjpeg-turbo/jsimd_none \
    UnpackedTarball/libjpeg-turbo/jcapimin \
    UnpackedTarball/libjpeg-turbo/jcapistd \
    UnpackedTarball/libjpeg-turbo/jccoefct \
    UnpackedTarball/libjpeg-turbo/jccolor \
    UnpackedTarball/libjpeg-turbo/jcdctmgr \
    UnpackedTarball/libjpeg-turbo/jchuff \
    UnpackedTarball/libjpeg-turbo/jcicc \
    UnpackedTarball/libjpeg-turbo/jcinit \
    UnpackedTarball/libjpeg-turbo/jcmainct \
    UnpackedTarball/libjpeg-turbo/jcmarker \
    UnpackedTarball/libjpeg-turbo/jcmaster \
    UnpackedTarball/libjpeg-turbo/jcomapi \
    UnpackedTarball/libjpeg-turbo/jcparam \
    UnpackedTarball/libjpeg-turbo/jcphuff \
    UnpackedTarball/libjpeg-turbo/jcprepct \
    UnpackedTarball/libjpeg-turbo/jcsample \
    UnpackedTarball/libjpeg-turbo/jctrans \
    UnpackedTarball/libjpeg-turbo/jdapimin \
    UnpackedTarball/libjpeg-turbo/jdapistd \
    UnpackedTarball/libjpeg-turbo/jdatadst \
    UnpackedTarball/libjpeg-turbo/jdatasrc \
    UnpackedTarball/libjpeg-turbo/jdcoefct \
    UnpackedTarball/libjpeg-turbo/jdcolor \
    UnpackedTarball/libjpeg-turbo/jddctmgr \
    UnpackedTarball/libjpeg-turbo/jdhuff \
    UnpackedTarball/libjpeg-turbo/jdicc \
    UnpackedTarball/libjpeg-turbo/jdinput \
    UnpackedTarball/libjpeg-turbo/jdmainct \
    UnpackedTarball/libjpeg-turbo/jdmarker \
    UnpackedTarball/libjpeg-turbo/jdmaster \
    UnpackedTarball/libjpeg-turbo/jdmerge \
    UnpackedTarball/libjpeg-turbo/jdphuff \
    UnpackedTarball/libjpeg-turbo/jdpostct \
    UnpackedTarball/libjpeg-turbo/jdsample \
    UnpackedTarball/libjpeg-turbo/jdtrans \
    UnpackedTarball/libjpeg-turbo/jerror \
    UnpackedTarball/libjpeg-turbo/jfdctflt \
    UnpackedTarball/libjpeg-turbo/jfdctfst \
    UnpackedTarball/libjpeg-turbo/jfdctint \
    UnpackedTarball/libjpeg-turbo/jidctflt \
    UnpackedTarball/libjpeg-turbo/jidctfst \
    UnpackedTarball/libjpeg-turbo/jidctint \
    UnpackedTarball/libjpeg-turbo/jidctred \
    UnpackedTarball/libjpeg-turbo/jquant1 \
    UnpackedTarball/libjpeg-turbo/jquant2 \
    UnpackedTarball/libjpeg-turbo/jutils \
    UnpackedTarball/libjpeg-turbo/jmemmgr \
    UnpackedTarball/libjpeg-turbo/jmemnobs \
    UnpackedTarball/libjpeg-turbo/jaricom \
    UnpackedTarball/libjpeg-turbo/jcarith \
    UnpackedTarball/libjpeg-turbo/jdarith \
))

# vim: set noet sw=4 ts=4:
