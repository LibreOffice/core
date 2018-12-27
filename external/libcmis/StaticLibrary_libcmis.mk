# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,libcmis))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,libcmis))

ifeq ($(COM_IS_CLANG),TRUE)
# Avoid narrowing conversion error (even though the option is technically a warning)
# caused by boost
$(eval $(call gb_StaticLibrary_add_cxxflags,libcmis,\
    -Wno-error=c++11-narrowing \
))
endif

$(eval $(call gb_StaticLibrary_add_defs,libcmis, \
	-DBOOST_ALL_NO_LIB \
))

$(eval $(call gb_StaticLibrary_set_include,libcmis, \
    -I$(call gb_UnpackedTarball_get_dir,libcmis/inc) \
    -I$(call gb_UnpackedTarball_get_dir,libcmis/src/libcmis) \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_use_unpacked,libcmis,libcmis))

$(eval $(call gb_StaticLibrary_use_externals,libcmis,\
	boost_headers \
	curl \
	libxml2 \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,libcmis,\
	UnpackedTarball/libcmis/src/libcmis/allowable-actions \
	UnpackedTarball/libcmis/src/libcmis/atom-document \
	UnpackedTarball/libcmis/src/libcmis/atom-folder \
	UnpackedTarball/libcmis/src/libcmis/atom-object-type \
	UnpackedTarball/libcmis/src/libcmis/atom-object \
	UnpackedTarball/libcmis/src/libcmis/atom-session \
	UnpackedTarball/libcmis/src/libcmis/atom-workspace \
	UnpackedTarball/libcmis/src/libcmis/base-session \
	UnpackedTarball/libcmis/src/libcmis/document \
	UnpackedTarball/libcmis/src/libcmis/folder \
	UnpackedTarball/libcmis/src/libcmis/gdrive-document \
	UnpackedTarball/libcmis/src/libcmis/gdrive-folder \
	UnpackedTarball/libcmis/src/libcmis/gdrive-object \
	UnpackedTarball/libcmis/src/libcmis/gdrive-object-type \
	UnpackedTarball/libcmis/src/libcmis/gdrive-property \
	UnpackedTarball/libcmis/src/libcmis/gdrive-repository \
	UnpackedTarball/libcmis/src/libcmis/gdrive-session \
	UnpackedTarball/libcmis/src/libcmis/gdrive-utils \
	UnpackedTarball/libcmis/src/libcmis/http-session \
	UnpackedTarball/libcmis/src/libcmis/json-utils \
	UnpackedTarball/libcmis/src/libcmis/oauth2-data \
	UnpackedTarball/libcmis/src/libcmis/oauth2-handler \
	UnpackedTarball/libcmis/src/libcmis/oauth2-providers \
	UnpackedTarball/libcmis/src/libcmis/object \
	UnpackedTarball/libcmis/src/libcmis/object-type \
	UnpackedTarball/libcmis/src/libcmis/onedrive-document \
	UnpackedTarball/libcmis/src/libcmis/onedrive-folder \
	UnpackedTarball/libcmis/src/libcmis/onedrive-object \
	UnpackedTarball/libcmis/src/libcmis/onedrive-object-type \
	UnpackedTarball/libcmis/src/libcmis/onedrive-property \
	UnpackedTarball/libcmis/src/libcmis/onedrive-repository \
	UnpackedTarball/libcmis/src/libcmis/onedrive-session \
	UnpackedTarball/libcmis/src/libcmis/onedrive-utils \
	UnpackedTarball/libcmis/src/libcmis/property \
	UnpackedTarball/libcmis/src/libcmis/property-type \
	UnpackedTarball/libcmis/src/libcmis/rendition \
	UnpackedTarball/libcmis/src/libcmis/repository \
	UnpackedTarball/libcmis/src/libcmis/session-factory \
	UnpackedTarball/libcmis/src/libcmis/sharepoint-document \
	UnpackedTarball/libcmis/src/libcmis/sharepoint-folder \
	UnpackedTarball/libcmis/src/libcmis/sharepoint-object \
	UnpackedTarball/libcmis/src/libcmis/sharepoint-object-type \
	UnpackedTarball/libcmis/src/libcmis/sharepoint-property \
	UnpackedTarball/libcmis/src/libcmis/sharepoint-repository \
	UnpackedTarball/libcmis/src/libcmis/sharepoint-session \
	UnpackedTarball/libcmis/src/libcmis/sharepoint-utils \
	UnpackedTarball/libcmis/src/libcmis/ws-document \
	UnpackedTarball/libcmis/src/libcmis/ws-folder \
	UnpackedTarball/libcmis/src/libcmis/ws-navigationservice \
	UnpackedTarball/libcmis/src/libcmis/ws-object \
	UnpackedTarball/libcmis/src/libcmis/ws-object-type \
	UnpackedTarball/libcmis/src/libcmis/ws-objectservice \
	UnpackedTarball/libcmis/src/libcmis/ws-relatedmultipart \
	UnpackedTarball/libcmis/src/libcmis/ws-repositoryservice \
	UnpackedTarball/libcmis/src/libcmis/ws-requests \
	UnpackedTarball/libcmis/src/libcmis/ws-session \
	UnpackedTarball/libcmis/src/libcmis/ws-soap \
	UnpackedTarball/libcmis/src/libcmis/ws-versioningservice \
	UnpackedTarball/libcmis/src/libcmis/xml-utils \
))

# vim: set noet sw=4 ts=4:
