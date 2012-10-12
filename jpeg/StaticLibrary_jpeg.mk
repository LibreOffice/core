# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,jpeg))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,jpeg))

$(eval $(call gb_StaticLibrary_use_unpacked,jpeg,jpeg))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,jpeg,\
    UnpackedTarball/jpeg/jaricom \
    UnpackedTarball/jpeg/jcapimin \
    UnpackedTarball/jpeg/jcapistd \
    UnpackedTarball/jpeg/jcarith \
    UnpackedTarball/jpeg/jccoefct \
    UnpackedTarball/jpeg/jccolor \
    UnpackedTarball/jpeg/jcdctmgr \
    UnpackedTarball/jpeg/jchuff \
    UnpackedTarball/jpeg/jcinit \
    UnpackedTarball/jpeg/jcmainct \
    UnpackedTarball/jpeg/jcmarker \
    UnpackedTarball/jpeg/jcmaster \
    UnpackedTarball/jpeg/jcomapi \
    UnpackedTarball/jpeg/jcparam \
    UnpackedTarball/jpeg/jcprepct \
    UnpackedTarball/jpeg/jcsample \
    UnpackedTarball/jpeg/jctrans \
    UnpackedTarball/jpeg/jdapimin \
    UnpackedTarball/jpeg/jdapistd \
    UnpackedTarball/jpeg/jdarith \
    UnpackedTarball/jpeg/jdatadst \
    UnpackedTarball/jpeg/jdatasrc \
    UnpackedTarball/jpeg/jdcoefct \
    UnpackedTarball/jpeg/jdcolor \
    UnpackedTarball/jpeg/jddctmgr \
    UnpackedTarball/jpeg/jdhuff \
    UnpackedTarball/jpeg/jdinput \
    UnpackedTarball/jpeg/jdmainct \
    UnpackedTarball/jpeg/jdmarker \
    UnpackedTarball/jpeg/jdmaster \
    UnpackedTarball/jpeg/jdmerge \
    UnpackedTarball/jpeg/jdpostct \
    UnpackedTarball/jpeg/jdsample \
    UnpackedTarball/jpeg/jdtrans \
    UnpackedTarball/jpeg/jerror \
    UnpackedTarball/jpeg/jfdctflt \
    UnpackedTarball/jpeg/jfdctfst \
    UnpackedTarball/jpeg/jfdctint \
    UnpackedTarball/jpeg/jidctflt \
    UnpackedTarball/jpeg/jidctfst \
    UnpackedTarball/jpeg/jidctint \
    UnpackedTarball/jpeg/jquant1 \
    UnpackedTarball/jpeg/jquant2 \
    UnpackedTarball/jpeg/jmemmgr \
    UnpackedTarball/jpeg/jmemnobs \
    UnpackedTarball/jpeg/jutils \
))

# vim: set noet sw=4 ts=4:
