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
# Copyright (C) 2011 Peter Foley <pefoley2@verizon.net> (initial developer)
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

$(eval $(call gb_Library_Library,mysql))

$(eval $(call gb_Library_set_componentfile,mysql,connectivity/source/drivers/mysql/mysql))

$(eval $(call gb_Library_use_sdk_api,mysql))

$(eval $(call gb_Library_set_include,mysql,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_use_libraries,mysql,\
	cppu \
	cppuhelper \
	sal \
	dbtools \
	comphelper \
	$(gb_UWINAPI) \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,mysql,\
	connectivity/source/drivers/mysql/YDriver \
	connectivity/source/drivers/mysql/YTables \
	connectivity/source/drivers/mysql/YTable \
	connectivity/source/drivers/mysql/YViews \
	connectivity/source/drivers/mysql/YCatalog \
	connectivity/source/drivers/mysql/YColumns \
	connectivity/source/drivers/mysql/YUser \
	connectivity/source/drivers/mysql/YUsers \
	connectivity/source/drivers/mysql/Yservices \
))

# vim: set noet sw=4 ts=4:
