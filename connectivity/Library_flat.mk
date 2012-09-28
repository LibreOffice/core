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

$(eval $(call gb_Library_Library,flat))

$(eval $(call gb_Library_set_componentfile,flat,connectivity/source/drivers/flat/flat))

$(eval $(call gb_Library_use_sdk_api,flat))

$(eval $(call gb_Library_set_include,flat,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_use_libraries,flat,\
	cppu \
	cppuhelper \
	tl \
	svl \
	utl \
	sal \
	salhelper \
	dbtools \
	file \
	comphelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,flat,\
	connectivity/source/drivers/flat/EResultSet \
	connectivity/source/drivers/flat/EStatement \
	connectivity/source/drivers/flat/EPreparedStatement \
	connectivity/source/drivers/flat/ETable \
	connectivity/source/drivers/flat/EDatabaseMetaData \
	connectivity/source/drivers/flat/ECatalog \
	connectivity/source/drivers/flat/EColumns \
	connectivity/source/drivers/flat/ETables \
	connectivity/source/drivers/flat/EConnection \
	connectivity/source/drivers/flat/Eservices \
	connectivity/source/drivers/flat/EDriver \
))

# vim: set noet sw=4 ts=4:
