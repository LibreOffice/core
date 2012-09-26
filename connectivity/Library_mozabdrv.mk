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

$(eval $(call gb_Library_Library,mozabdrv))

$(eval $(call gb_Library_set_include,mozabdrv,\
	-I$(SRCDIR)/connectivity/source/drivers/mozab \
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,mozabdrv))

$(eval $(call gb_Library_use_libraries,mozabdrv,\
	comphelper \
	cppu \
	cppuhelper \
	dbtools \
	sal \
	salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,mozabdrv))

$(eval $(call gb_Library_use_externals,mozabdrv,\
	mozilla \
))

$(eval $(call gb_Library_add_exception_objects,mozabdrv,\
	connectivity/source/drivers/mozab/MCatalog \
	connectivity/source/drivers/mozab/MColumnAlias \
	connectivity/source/drivers/mozab/MColumns \
	connectivity/source/drivers/mozab/MConfigAccess \
	connectivity/source/drivers/mozab/MConnection \
	connectivity/source/drivers/mozab/MDatabaseMetaData \
	connectivity/source/drivers/mozab/MPreparedStatement \
	connectivity/source/drivers/mozab/MResultSet \
	connectivity/source/drivers/mozab/MResultSetMetaData \
	connectivity/source/drivers/mozab/MStatement \
	connectivity/source/drivers/mozab/MTable \
	connectivity/source/drivers/mozab/MTables \
	connectivity/source/drivers/mozab/bootstrap/MMozillaBootstrap \
	connectivity/source/drivers/mozab/bootstrap/MNSFolders \
	connectivity/source/drivers/mozab/bootstrap/MNSINIParser \
	connectivity/source/drivers/mozab/bootstrap/MNSInit \
	connectivity/source/drivers/mozab/bootstrap/MNSProfile \
	connectivity/source/drivers/mozab/bootstrap/MNSProfileDirServiceProvider \
	connectivity/source/drivers/mozab/bootstrap/MNSProfileDiscover \
	connectivity/source/drivers/mozab/bootstrap/MNSProfileManager \
	connectivity/source/drivers/mozab/bootstrap/MNSRunnable \
	connectivity/source/drivers/mozab/mozillasrc/MDatabaseMetaDataHelper \
	connectivity/source/drivers/mozab/mozillasrc/MLdapAttributeMap \
	connectivity/source/drivers/mozab/mozillasrc/MNSMozabProxy \
	connectivity/source/drivers/mozab/mozillasrc/MNSTerminateListener \
	connectivity/source/drivers/mozab/mozillasrc/MNameMapper \
	connectivity/source/drivers/mozab/mozillasrc/MQuery \
	connectivity/source/drivers/mozab/mozillasrc/MQueryHelper \
	connectivity/source/drivers/mozab/mozillasrc/MTypeConverter \
))

# vim: set noet sw=4 ts=4:
