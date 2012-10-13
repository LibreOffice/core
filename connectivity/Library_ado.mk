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

$(eval $(call gb_Library_Library,ado))

$(eval $(call gb_Library_set_componentfile,ado,connectivity/source/drivers/ado/ado))

$(eval $(call gb_Library_set_include,ado,\
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,ado))

ifeq ($(COM),GCC)
$(eval $(call gb_Library_add_cxxflags,ado,\
	-fpermissive \
))
endif

$(eval $(call gb_Library_use_system_win32_libs,ado,\
	ole32 \
	oleaut32 \
	uuid \
))

ifeq ($(WINDOWS_SDK_VERSION),80)
$(eval $(call gb_Library_add_defs,ado,\
	-DNTDDI_VERSION=0x0601 \
))
endif

$(eval $(call gb_Library_use_libraries,ado,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	dbtools \
	comphelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,ado,\
	connectivity/source/drivers/ado/ADatabaseMetaDataImpl \
	connectivity/source/drivers/ado/Aolevariant \
	connectivity/source/drivers/ado/ADatabaseMetaData \
	connectivity/source/drivers/ado/AColumn \
	connectivity/source/drivers/ado/AColumns \
	connectivity/source/drivers/ado/AIndex \
	connectivity/source/drivers/ado/AIndexes \
	connectivity/source/drivers/ado/AKey \
	connectivity/source/drivers/ado/AKeys \
	connectivity/source/drivers/ado/AUser \
	connectivity/source/drivers/ado/AUsers \
	connectivity/source/drivers/ado/AGroup \
	connectivity/source/drivers/ado/AGroups \
	connectivity/source/drivers/ado/ACatalog \
	connectivity/source/drivers/ado/AView \
	connectivity/source/drivers/ado/AViews \
	connectivity/source/drivers/ado/ATable \
	connectivity/source/drivers/ado/ATables \
	connectivity/source/drivers/ado/ACallableStatement \
	connectivity/source/drivers/ado/ADatabaseMetaDataResultSetMetaData \
	connectivity/source/drivers/ado/ADatabaseMetaDataResultSet \
	connectivity/source/drivers/ado/AResultSet \
	connectivity/source/drivers/ado/AConnection \
	connectivity/source/drivers/ado/AStatement \
	connectivity/source/drivers/ado/APreparedStatement \
	connectivity/source/drivers/ado/AResultSetMetaData \
	connectivity/source/drivers/ado/ADriver \
	connectivity/source/drivers/ado/Aservices \
	connectivity/source/drivers/ado/Awrapado \
	connectivity/source/drivers/ado/adoimp \
))

# vim: set noet sw=4 ts=4:
