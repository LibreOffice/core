# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpgvfs1))

$(eval $(call gb_Library_set_componentfile,ucpgvfs1,ucb/source/ucp/gvfs/ucpgvfs))

$(eval $(call gb_Library_set_include,ucpgvfs1,\
	$(GNOMEVFS_CFLAGS) \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,ucpgvfs1))

$(eval $(call gb_Library_add_libs,ucpgvfs1,\
	$(GNOMEVFS_LIBS) \
))

$(eval $(call gb_Library_use_libraries,ucpgvfs1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	ucbhelper \
))

$(eval $(call gb_Library_add_exception_objects,ucpgvfs1,\
	ucb/source/ucp/gvfs/gvfs_content \
	ucb/source/ucp/gvfs/gvfs_directory \
	ucb/source/ucp/gvfs/gvfs_provider \
	ucb/source/ucp/gvfs/gvfs_stream \
))

# vim: set noet sw=4 ts=4:
