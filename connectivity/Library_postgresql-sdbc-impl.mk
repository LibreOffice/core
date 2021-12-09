# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,postgresql-sdbc-impl))

$(eval $(call gb_Library_add_defs,postgresql-sdbc-impl,\
	-DPQ_SDBC_MAJOR=0 \
	-DPQ_SDBC_MINOR=8 \
	-DPQ_SDBC_MICRO=2 \
))

$(eval $(call gb_Library_set_precompiled_header,postgresql-sdbc-impl,connectivity/inc/pch/precompiled_postgresql-sdbc-impl))

$(eval $(call gb_Library_use_sdk_api,postgresql-sdbc-impl))

$(eval $(call gb_Library_use_libraries,postgresql-sdbc-impl,\
	comphelper \
	cppu \
	cppuhelper \
	dbtools \
	sal \
	salhelper \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,postgresql-sdbc-impl,\
	shell32 \
	wldap32 \
))

ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_ldflags,postgresql-sdbc-impl,\
	/NODEFAULTLIB:libcmt.lib \
))
endif

endif

$(eval $(call gb_Library_use_externals,postgresql-sdbc-impl,\
	boost_headers \
	postgresql \
	$(if $(filter-out MSC,$(COM)), \
		openssl \
		$(if $(ENABLE_LDAP),openldap) \
		nss3 \
		plc4 \
		ssl3 \
	) \
))

ifeq ($(SYSTEM_POSTGRESQL),)
ifneq ($(OS),WNT)

$(eval $(call gb_Library_add_libs,postgresql-sdbc-impl,\
	$(if $(WITH_GSSAPI),$(GSSAPI_LIBS)) \
	$(if $(WITH_KRB5),$(KRB5_LIBS)) \
	$(if $(filter-out MACOSX,$(OS)),-ldl) \
))

endif
endif

$(eval $(call gb_Library_set_componentfile,postgresql-sdbc-impl,connectivity/source/drivers/postgresql/postgresql-sdbc-impl,postgresql-sdbc))

$(eval $(call gb_Library_add_exception_objects,postgresql-sdbc-impl,\
	connectivity/source/drivers/postgresql/pq_array \
	connectivity/source/drivers/postgresql/pq_baseresultset \
	connectivity/source/drivers/postgresql/pq_connection \
	connectivity/source/drivers/postgresql/pq_databasemetadata \
	connectivity/source/drivers/postgresql/pq_fakedupdateableresultset \
	connectivity/source/drivers/postgresql/pq_preparedstatement \
	connectivity/source/drivers/postgresql/pq_resultset \
	connectivity/source/drivers/postgresql/pq_resultsetmetadata \
	connectivity/source/drivers/postgresql/pq_sequenceresultset \
	connectivity/source/drivers/postgresql/pq_sequenceresultsetmetadata \
	connectivity/source/drivers/postgresql/pq_statement \
	connectivity/source/drivers/postgresql/pq_statics \
	connectivity/source/drivers/postgresql/pq_tools \
	connectivity/source/drivers/postgresql/pq_updateableresultset \
	connectivity/source/drivers/postgresql/pq_xbase \
	connectivity/source/drivers/postgresql/pq_xcolumn \
	connectivity/source/drivers/postgresql/pq_xcolumns \
	connectivity/source/drivers/postgresql/pq_xcontainer \
	connectivity/source/drivers/postgresql/pq_xindex \
	connectivity/source/drivers/postgresql/pq_xindexcolumn \
	connectivity/source/drivers/postgresql/pq_xindexcolumns \
	connectivity/source/drivers/postgresql/pq_xindexes \
	connectivity/source/drivers/postgresql/pq_xkey \
	connectivity/source/drivers/postgresql/pq_xkeycolumn \
	connectivity/source/drivers/postgresql/pq_xkeycolumns \
	connectivity/source/drivers/postgresql/pq_xkeys \
	connectivity/source/drivers/postgresql/pq_xtable \
	connectivity/source/drivers/postgresql/pq_xtables \
	connectivity/source/drivers/postgresql/pq_xuser \
	connectivity/source/drivers/postgresql/pq_xusers \
	connectivity/source/drivers/postgresql/pq_xview \
	connectivity/source/drivers/postgresql/pq_xviews \
))

# vim: set noet sw=4 ts=4:
