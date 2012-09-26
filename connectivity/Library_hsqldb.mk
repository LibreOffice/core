# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

$(eval $(call gb_Library_Library,hsqldb))

$(eval $(call gb_Library_set_include,hsqldb,\
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_externals,hsqldb,\
	hsqldb \
))

$(eval $(call gb_Library_use_sdk_api,hsqldb))

$(eval $(call gb_Library_use_libraries,hsqldb,\
	comphelper \
	cppu \
	cppuhelper \
	dbtools \
	jvmfwk \
	sal \
	tl \
	utl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,hsqldb))

$(eval $(call gb_Library_set_componentfile,hsqldb,connectivity/source/drivers/hsqldb/hsqldb))

$(eval $(call gb_Library_add_exception_objects,hsqldb,\
	connectivity/source/drivers/hsqldb/HCatalog \
	connectivity/source/drivers/hsqldb/HColumns \
	connectivity/source/drivers/hsqldb/HConnection \
	connectivity/source/drivers/hsqldb/HDriver \
	connectivity/source/drivers/hsqldb/HStorageAccess \
	connectivity/source/drivers/hsqldb/HStorageMap \
	connectivity/source/drivers/hsqldb/HTable \
	connectivity/source/drivers/hsqldb/HTables \
	connectivity/source/drivers/hsqldb/HTerminateListener \
	connectivity/source/drivers/hsqldb/HTools \
	connectivity/source/drivers/hsqldb/HUser \
	connectivity/source/drivers/hsqldb/HUsers \
	connectivity/source/drivers/hsqldb/HView \
	connectivity/source/drivers/hsqldb/HViews \
	connectivity/source/drivers/hsqldb/Hservices \
	connectivity/source/drivers/hsqldb/StorageFileAccess \
	connectivity/source/drivers/hsqldb/StorageNativeInputStream \
	connectivity/source/drivers/hsqldb/StorageNativeOutputStream \
	connectivity/source/drivers/hsqldb/accesslog \
))

# vim: set noet sw=4 ts=4:
