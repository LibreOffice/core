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

$(eval $(call gb_Library_Library,ucpfile1))

$(eval $(call gb_Library_set_componentfile,ucpfile1,ucb/source/ucp/file/ucpfile1))

$(eval $(call gb_Library_use_sdk_api,ucpfile1))

$(eval $(call gb_Library_use_libraries,ucpfile1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	ucbhelper \
	$(gb_UWINAPI) \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,ucpfile1,\
	ucb/source/ucp/file/bc \
	ucb/source/ucp/file/filcmd \
	ucb/source/ucp/file/filglob \
	ucb/source/ucp/file/filid \
	ucb/source/ucp/file/filinpstr \
	ucb/source/ucp/file/filinsreq \
	ucb/source/ucp/file/filnot \
	ucb/source/ucp/file/filprp \
	ucb/source/ucp/file/filrec \
	ucb/source/ucp/file/filrow \
	ucb/source/ucp/file/filrset \
	ucb/source/ucp/file/filstr \
	ucb/source/ucp/file/filtask \
	ucb/source/ucp/file/prov \
	ucb/source/ucp/file/shell \
))

# vim: set noet sw=4 ts=4:
