# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,langtag))

$(eval $(call gb_Library_use_unpacked,langtag,langtag))

$(eval $(call gb_Library_use_externals,langtag,\
	libxml2 \
))

$(eval $(call gb_Library_use_static_libraries,langtag,\
	langtag-ext-t \
	langtag-ext-u \
))

$(eval $(call gb_Library_set_warnings_not_errors,langtag))

$(eval $(call gb_Library_set_include,langtag,\
	-I$(WORKDIR)/UnpackedTarball/langtag/liblangtag \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,langtag,\
	-DALIGNOF_VOID_P=$(if $(WINDOWS_X64),8,4) \
	-DENABLE_MODULE=0 \
	-DHAVE_STRING_H \
	-DHAVE_STRNDUP \
	-DHAVE_VSNPRINTF \
	-DREGDATADIR=\"/an/irrelevant/path\" \
	-D_WIN32 \
	-D__LANGTAG_COMPILATION \
	-D__LANGTAG_H__INSIDE \
	-D__LANGTAG_PRIVATE \
))

$(eval $(call gb_Library_add_generated_cobjects,langtag,\
	UnpackedTarball/langtag/liblangtag/lt-database \
	UnpackedTarball/langtag/liblangtag/lt-error \
	UnpackedTarball/langtag/liblangtag/lt-ext-module \
	UnpackedTarball/langtag/liblangtag/lt-ext-module-data \
	UnpackedTarball/langtag/liblangtag/lt-extension \
	UnpackedTarball/langtag/liblangtag/lt-extlang \
	UnpackedTarball/langtag/liblangtag/lt-extlang-db \
	UnpackedTarball/langtag/liblangtag/lt-grandfathered \
	UnpackedTarball/langtag/liblangtag/lt-grandfathered-db \
	UnpackedTarball/langtag/liblangtag/lt-iter \
	UnpackedTarball/langtag/liblangtag/lt-lang \
	UnpackedTarball/langtag/liblangtag/lt-lang-db \
	UnpackedTarball/langtag/liblangtag/lt-list \
	UnpackedTarball/langtag/liblangtag/lt-mem \
	UnpackedTarball/langtag/liblangtag/lt-messages \
	UnpackedTarball/langtag/liblangtag/lt-redundant \
	UnpackedTarball/langtag/liblangtag/lt-redundant-db \
	UnpackedTarball/langtag/liblangtag/lt-region \
	UnpackedTarball/langtag/liblangtag/lt-region-db \
	UnpackedTarball/langtag/liblangtag/lt-script \
	UnpackedTarball/langtag/liblangtag/lt-script-db \
	UnpackedTarball/langtag/liblangtag/lt-string \
	UnpackedTarball/langtag/liblangtag/lt-tag \
	UnpackedTarball/langtag/liblangtag/lt-trie \
	UnpackedTarball/langtag/liblangtag/lt-utils \
	UnpackedTarball/langtag/liblangtag/lt-variant \
	UnpackedTarball/langtag/liblangtag/lt-variant-db \
	UnpackedTarball/langtag/liblangtag/lt-xml \
))

# vim: set noet sw=4 ts=4:
