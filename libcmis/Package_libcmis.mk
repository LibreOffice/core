# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,libcmis_inc,$(call gb_UnpackedTarball_get_dir,cmis/src/libcmis)))

$(eval $(call gb_Package_use_unpacked,libcmis_inc,cmis))

$(eval $(call gb_Package_add_files,libcmis_inc,inc/external/libcmis,\
	allowable-actions.hxx \
	atom-document.hxx \
	atom-folder.hxx \
	atom-object-type.hxx \
	atom-object.hxx \
	atom-session.hxx \
	atom-utils.hxx \
	atom-workspace.hxx \
	document.hxx \
	exception.hxx \
	folder.hxx \
	object-type.hxx \
	object.hxx \
	property-type.hxx \
	property.hxx \
	repository.hxx \
	session-factory.hxx \
	session.hxx \
	test-helpers.hxx \
	xml-utils.hxx \
	xmlserializable.hxx \
))

# vim: set noet sw=4 ts=4:
