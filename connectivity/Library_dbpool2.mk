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

$(eval $(call gb_Library_Library,dbpool2))

$(eval $(call gb_Library_set_componentfile,dbpool2,connectivity/source/cpool/dbpool2))

$(eval $(call gb_Library_set_include,dbpool2,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_use_sdk_api,dbpool2))

$(eval $(call gb_Library_use_libraries,dbpool2,\
	comphelper \
	cppu \
	cppuhelper \
	dbtools \
	sal \
	salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,dbpool2))

$(eval $(call gb_Library_add_exception_objects,dbpool2,\
	connectivity/source/cpool/ZConnectionWrapper \
	connectivity/source/cpool/ZDriverWrapper \
	connectivity/source/cpool/ZPooledConnection \
	connectivity/source/cpool/ZConnectionPool \
	connectivity/source/cpool/ZPoolCollection \
	connectivity/source/cpool/Zregistration \
))

# vim: set noet sw=4 ts=4:
