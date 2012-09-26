# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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

$(eval $(call gb_Library_add_standard_system_libs,ucpgvfs1))

$(eval $(call gb_Library_add_exception_objects,ucpgvfs1,\
	ucb/source/ucp/gvfs/gvfs_content \
	ucb/source/ucp/gvfs/gvfs_directory \
	ucb/source/ucp/gvfs/gvfs_provider \
	ucb/source/ucp/gvfs/gvfs_stream \
))

# vim: set noet sw=4 ts=4:
