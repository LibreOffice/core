# -*- Mode: makefile-gmake; tab-width: 4; indent-uabs-mode: t -*-
#
# This file is part of the StaticLibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,langtag-ext-u))

$(eval $(call gb_StaticLibrary_use_unpacked,langtag-ext-u,langtag))

$(eval $(call gb_StaticLibrary_use_externals,langtag-ext-u,\
	libxml2 \
))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,langtag-ext-u))

$(eval $(call gb_StaticLibrary_set_include,langtag-ext-u,\
	-I$(WORKDIR)/UnpackedTarball/langtag/liblangtag \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,langtag-ext-u,\
	-DALIGNOF_VOID_P=$(if $(WINDOWS_X64),8,4) \
	-DENABLE_MODULE=0 \
	-DHAVE_STRING_H \
	-DHAVE_STRNDUP \
	-DHAVE_VSNPRINTF \
	-DLT_MODULE_PREFIX=lt_module_ext_u \
	-DREGDATADIR=\"/an/irrelevant/path\" \
	-D_WIN32 \
	-D__LANGTAG_COMPILATION \
	-D__LANGTAG_H__INSIDE \
	-D__LANGTAG_PRIVATE \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,langtag-ext-u,\
	UnpackedTarball/langtag/extensions/lt-ext-ldml-u \
))

# vim: set noet sw=4 ts=4:
