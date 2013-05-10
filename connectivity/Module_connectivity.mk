# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,connectivity))

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
))

ifneq ($(SOLAR_JAVA),)
$(eval $(call gb_Module_add_targets,connectivity,\
	Configuration_hsqldb \
	Configuration_jdbc \
	Jar_sdbc_hsqldb \
	Library_hsqldb \
	Library_jdbc \
))
endif

ifneq ($(OS),WNT)

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

ifeq ($(OS),WNT)
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

ifeq ($(ENABLE_FIREBIRD_SDBC),TRUE)
$(eval $(call gb_Module_add_targets,connectivity,\
	Configuration_firebird \
	Library_firebird-sdbc \
	Rdb_firebird-sdbc \
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

ifeq ($(WITH_MOZAB4WIN),YES)
$(eval $(call gb_Module_add_targets,connectivity,\
	Configuration_mozab \
	Library_mozab \
	Library_mozabdrv \
))
else
$(eval $(call gb_Module_add_targets,connectivity,\
	Library_mozbootstrap \
))
endif

else ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))

$(eval $(call gb_Module_add_targets,connectivity,\
	Configuration_mork \
	Executable_mork_helper \
	Library_mork \
	Library_mozbootstrap \
))

$(eval $(call gb_Module_add_check_targets,connectivity,\
	CppunitTest_connectivity_mork \
))

endif

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_check_targets,connectivity,\
	CppunitTest_connectivity_ado \
))
endif

ifneq ($(filter QADEVOOO,$(BUILD_TYPE)),)
$(eval $(call gb_Module_add_subsequentcheck_targets,connectivity,\
	Jar_ConnectivityTools \
))
# FIXME: Does not work. Convert to JUnit.
# JunitTest_complex \

endif

# general tests
$(eval $(call gb_Module_add_check_targets,connectivity,\
	CppunitTest_connectivity_commontools \
))

# vim: set noet sw=4 ts=4:
