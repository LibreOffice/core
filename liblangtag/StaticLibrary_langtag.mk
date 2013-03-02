# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,langtag))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,langtag))

$(eval $(call gb_StaticLibrary_use_unpacked,langtag,langtag))

$(eval $(call gb_StaticLibrary_use_external,langtag,libxml2))

$(eval $(call gb_StaticLibrary_set_include,langtag,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,langtag) \
	-I$(call gb_UnpackedTarball_get_dir,langtag)/liblangtag \
))

# Hardcoded for MSVC
$(eval $(call gb_StaticLibrary_add_defs,langtag,\
	-D__LANGTAG_COMPILATION \
	-DREGDATADIR \
	-DLT_HAVE___INLINE \
	-DLT_CAN_INLINE \
	-DALIGNOF_VOID_P=4 \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,langtag,\
	UnpackedTarball/langtag/liblangtag/lt-database \
	UnpackedTarball/langtag/liblangtag/lt-error \
	UnpackedTarball/langtag/liblangtag/lt-ext-module \
	UnpackedTarball/langtag/liblangtag/lt-ext-module-data \
	UnpackedTarball/langtag/liblangtag/lt-extension \
	UnpackedTarball/langtag/liblangtag/lt-extlang \
	UnpackedTarball/langtag/liblangtag/lt-extlang-db \
	UnpackedTarball/langtag/liblangtag/lt-grandfathered \
	UnpackedTarball/langtag/liblangtag/lt-grandfathered-db \
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

$(eval $(call gb_StaticLibrary_add_generated_cobjects,langtag,\
	UnpackedTarball/langtag/extensions/lt-ext-ldml-t \
	, -DLT_MODULE_PREFIX=lt_module_ext_t \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,langtag,\
	UnpackedTarball/langtag/extensions/lt-ext-ldml-u \
	, -DLT_MODULE_PREFIX=lt_module_ext_u \
))

# vim: set noet sw=4 ts=4:
