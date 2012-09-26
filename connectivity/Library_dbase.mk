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

$(eval $(call gb_Library_Library,dbase))

$(eval $(call gb_Library_set_componentfile,dbase,connectivity/source/drivers/dbase/dbase))

$(eval $(call gb_Library_use_sdk_api,dbase))

$(eval $(call gb_Library_set_include,dbase,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_use_libraries,dbase,\
	cppu \
	cppuhelper \
	svl \
	tl \
	ucbhelper \
	sal \
	salhelper \
	dbtools \
	file \
	utl \
	comphelper \
	$(gb_UWINAPI) \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,dbase,\
	connectivity/source/drivers/dbase/DCode \
	connectivity/source/drivers/dbase/DResultSet \
	connectivity/source/drivers/dbase/DStatement \
	connectivity/source/drivers/dbase/DPreparedStatement \
	connectivity/source/drivers/dbase/dindexnode \
	connectivity/source/drivers/dbase/DIndexIter \
	connectivity/source/drivers/dbase/DDatabaseMetaData \
	connectivity/source/drivers/dbase/DCatalog \
	connectivity/source/drivers/dbase/DColumns \
	connectivity/source/drivers/dbase/DIndexColumns \
	connectivity/source/drivers/dbase/DIndex \
	connectivity/source/drivers/dbase/DIndexes \
	connectivity/source/drivers/dbase/DTables \
	connectivity/source/drivers/dbase/DConnection \
	connectivity/source/drivers/dbase/Dservices \
	connectivity/source/drivers/dbase/DDriver \
	connectivity/source/drivers/dbase/DTable \
))

#connectivity/source/drivers/dbase/DTable disable optimization?

# vim: set noet sw=4 ts=4:
