# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,tubes))

$(eval $(call gb_Library_use_sdk_api,tubes))

$(eval $(call gb_Library_add_defs,tubes,\
	-DTUBES_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,tubes,\
	comphelper \
	cppu \
	sal \
	svt \
	svxcore \
	tl \
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,tubes,\
	boost_headers \
	gtk \
	telepathy \
))

$(eval $(call gb_Library_add_exception_objects,tubes,\
	tubes/source/collaboration \
	tubes/source/conference \
	tubes/source/contacts \
	tubes/source/manager \
))

$(eval $(call gb_Library_add_cobjects,tubes,\
	tubes/source/file-transfer-helper \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
