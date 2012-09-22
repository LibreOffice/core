# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libcmis_inc,cmis))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libcmis_inc,inc/external/libcmis,\
	src/libcmis/allowable-actions.hxx \
	src/libcmis/document.hxx \
	src/libcmis/exception.hxx \
	src/libcmis/folder.hxx \
	src/libcmis/object-type.hxx \
	src/libcmis/object.hxx \
	src/libcmis/property-type.hxx \
	src/libcmis/property.hxx \
	src/libcmis/repository.hxx \
	src/libcmis/session-factory.hxx \
	src/libcmis/session.hxx \
	src/libcmis/xml-utils.hxx \
	src/libcmis/xmlserializable.hxx \
))

# vim: set noet sw=4 ts=4:
