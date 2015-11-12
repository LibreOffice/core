# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,cmislib))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,cmislib))

ifeq ($(COM_IS_CLANG),TRUE)
# Avoid narrowing conversion error (even though the option is technically a warning)
# caused by boost.
$(eval $(call gb_StaticLibrary_add_cxxflags,cmislib,\
    -Wno-error=c++11-narrowing \
))
endif

$(eval $(call gb_StaticLibrary_use_unpacked,cmislib,cmis))

$(eval $(call gb_StaticLibrary_use_externals,cmislib,\
	boost_headers \
	curl \
	libxml2 \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,cmislib,\
	UnpackedTarball/cmis/src/libcmis/allowable-actions \
	UnpackedTarball/cmis/src/libcmis/atom-document \
	UnpackedTarball/cmis/src/libcmis/atom-folder \
	UnpackedTarball/cmis/src/libcmis/atom-object-type \
	UnpackedTarball/cmis/src/libcmis/atom-object \
	UnpackedTarball/cmis/src/libcmis/atom-session \
	UnpackedTarball/cmis/src/libcmis/atom-workspace \
	UnpackedTarball/cmis/src/libcmis/base-session \
	UnpackedTarball/cmis/src/libcmis/document \
	UnpackedTarball/cmis/src/libcmis/folder \
	UnpackedTarball/cmis/src/libcmis/gdrive-document \
	UnpackedTarball/cmis/src/libcmis/gdrive-folder \
	UnpackedTarball/cmis/src/libcmis/gdrive-object \
	UnpackedTarball/cmis/src/libcmis/gdrive-object-type \
	UnpackedTarball/cmis/src/libcmis/gdrive-property \
	UnpackedTarball/cmis/src/libcmis/gdrive-repository \
	UnpackedTarball/cmis/src/libcmis/gdrive-session \
	UnpackedTarball/cmis/src/libcmis/gdrive-utils \
	UnpackedTarball/cmis/src/libcmis/http-session \
	UnpackedTarball/cmis/src/libcmis/json-utils \
	UnpackedTarball/cmis/src/libcmis/oauth2-data \
	UnpackedTarball/cmis/src/libcmis/oauth2-handler \
	UnpackedTarball/cmis/src/libcmis/oauth2-providers \
	UnpackedTarball/cmis/src/libcmis/object \
	UnpackedTarball/cmis/src/libcmis/object-type \
	UnpackedTarball/cmis/src/libcmis/onedrive-document \
	UnpackedTarball/cmis/src/libcmis/onedrive-folder \
	UnpackedTarball/cmis/src/libcmis/onedrive-object \
	UnpackedTarball/cmis/src/libcmis/onedrive-object-type \
	UnpackedTarball/cmis/src/libcmis/onedrive-property \
	UnpackedTarball/cmis/src/libcmis/onedrive-repository \
	UnpackedTarball/cmis/src/libcmis/onedrive-session \
	UnpackedTarball/cmis/src/libcmis/onedrive-utils \
	UnpackedTarball/cmis/src/libcmis/property \
	UnpackedTarball/cmis/src/libcmis/property-type \
	UnpackedTarball/cmis/src/libcmis/rendition \
	UnpackedTarball/cmis/src/libcmis/repository \
	UnpackedTarball/cmis/src/libcmis/session-factory \
	UnpackedTarball/cmis/src/libcmis/sharepoint-document \
	UnpackedTarball/cmis/src/libcmis/sharepoint-folder \
	UnpackedTarball/cmis/src/libcmis/sharepoint-object \
	UnpackedTarball/cmis/src/libcmis/sharepoint-object-type \
	UnpackedTarball/cmis/src/libcmis/sharepoint-property \
	UnpackedTarball/cmis/src/libcmis/sharepoint-repository \
	UnpackedTarball/cmis/src/libcmis/sharepoint-session \
	UnpackedTarball/cmis/src/libcmis/sharepoint-utils \
	UnpackedTarball/cmis/src/libcmis/ws-document \
	UnpackedTarball/cmis/src/libcmis/ws-folder \
	UnpackedTarball/cmis/src/libcmis/ws-navigationservice \
	UnpackedTarball/cmis/src/libcmis/ws-object \
	UnpackedTarball/cmis/src/libcmis/ws-object-type \
	UnpackedTarball/cmis/src/libcmis/ws-objectservice \
	UnpackedTarball/cmis/src/libcmis/ws-relatedmultipart \
	UnpackedTarball/cmis/src/libcmis/ws-repositoryservice \
	UnpackedTarball/cmis/src/libcmis/ws-requests \
	UnpackedTarball/cmis/src/libcmis/ws-session \
	UnpackedTarball/cmis/src/libcmis/ws-soap \
	UnpackedTarball/cmis/src/libcmis/ws-versioningservice \
	UnpackedTarball/cmis/src/libcmis/xml-utils \
))

# vim: set noet sw=4 ts=4:
