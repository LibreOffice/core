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

$(eval $(call gb_Module_Module,connectivity))

ifneq (,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))

$(eval $(call gb_Module_add_targets,connectivity,\
	AllLangResTarget_cnr \
	AllLangResTarget_sdbcl \
	AllLangResTarget_sdberr \
	Configuration_calc \
	Configuration_dbase \
	Configuration_flat \
	Configuration_mysql \
	Configuration_odbc \
	Library_calc \
	Library_dbase \
	Library_dbpool2 \
	Library_dbtools \
	Library_file \
	Library_flat \
	Library_mysql \
	$(if $(filter ANDROID IOS,$(OS)),, \
		Library_odbc \
		Library_odbcbase) \
	Library_sdbc2 \
	Package_generated \
	Package_inc \
))

ifneq ($(SOLAR_JAVA),)
$(eval $(call gb_Module_add_targets,connectivity,\
	AllLangResTarget_hsqldb \
	Configuration_hsqldb \
	Configuration_jdbc \
	Jar_sdbc_hsqldb \
	Library_hsqldb \
	Library_jdbc \
))
endif

ifeq ($(GUI),UNX)

ifeq ($(ENABLE_KAB),TRUE)
$(eval $(call gb_Module_add_targets,connectivity,\
	Configuration_kab \
	Library_kab1 \
	Library_kabdrv1 \
))
endif

ifeq ($(ENABLE_TDEAB),TRUE)
$(eval $(call gb_Module_add_targets,connectivity,\
	Configuration_tdeab \
	Library_tdeab1 \
	Library_tdeabdrv1 \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,connectivity,\
	Configuration_macab \
	Library_macab1 \
	Library_macabdrv1 \
))
endif

endif

ifeq ($(GUI),WNT)
$(eval $(call gb_Module_add_targets,connectivity,\
	Configuration_ado \
	Library_ado \
))
endif

ifeq ($(ENABLE_EVOAB2),TRUE)
$(eval $(call gb_Module_add_targets,connectivity,\
	Configuration_evoab \
	Library_evoab \
))
endif

ifeq ($(BUILD_POSTGRESQL_SDBC),YES)
$(eval $(call gb_Module_add_targets,connectivity,\
	Configuration_postgresql \
	Library_postgresql-sdbc \
	Library_postgresql-sdbc-impl \
	Package_postgresql-sdbc \
	Rdb_postgresql-sdbc \
))
endif

ifeq ($(OS),WNT)

ifeq ($(if $(or $(filter-out YES,$(WITH_MOZILLA)),$(filter YES,$(SYSTEM_MOZILLA))),YES),YES)
$(eval $(call gb_Module_add_targets,connectivity,\
	Library_mozbootstrap \
))
else
$(eval $(call gb_Module_add_targets,connectivity,\
	Configuration_mozab \
	Library_mozab \
	Library_mozabdrv \
))
endif

else ifneq ($(filter-out ANDROID IOS,$(OS)),)

$(eval $(call gb_Module_add_targets,connectivity,\
	Configuration_mork \
	Executable_mork_helper \
	Library_mork \
))

endif

ifneq ($(filter QADEVOOO,$(BUILD_TYPE)),)
$(eval $(call gb_Module_add_subsequentcheck_targets,connectivity,\
	Jar_ConnectivityTools \
))
# FIXME: Does not work. Convert to JUnit.
# JunitTest_complex \

endif

$(eval $(call gb_Module_add_check_targets,connectivity,\
	CppunitTest_connectivity_mork \
))

endif

# vim: set noet sw=4 ts=4:
