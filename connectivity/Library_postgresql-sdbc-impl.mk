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

$(eval $(call gb_Library_Library,postgresql-sdbc-impl))

$(eval $(call gb_Library_add_defs,postgresql-sdbc-impl,\
	-DPQ_SDBC_MAJOR=0 \
	-DPQ_SDBC_MINOR=8 \
	-DPQ_SDBC_MICRO=2 \
))

$(eval $(call gb_Library_use_sdk_api,postgresql-sdbc-impl))

$(eval $(call gb_Library_use_libraries,postgresql-sdbc-impl,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	$(gb_UWINAPI) \
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
	postgresql \
))

ifneq ($(SYSTEM_POSTGRESQL),YES)
ifneq ($(GUI)$(COM),WNTMSC)

$(eval $(call gb_Library_use_external_package,postgresql-sdbc-impl,\
	postgresql \
))

-include $(OUTDIR)/inc/postgresql/libpq-flags.mk

$(eval $(call gb_Library_add_libs,postgresql-sdbc-impl,\
	$(if $(filter-out MACOSX,$(OS)),-Wl$(COMMA)--as-needed) \
	$(LIBPQ_DEP_LIBS) \
	$(if $(filter-out MACOSX,$(OS)),-ldl) \
	$(if $(filter-out MACOSX,$(OS)),-Wl$(COMMA)--no-as-needed) \
))

endif
endif

$(eval $(call gb_Library_set_componentfile,postgresql-sdbc-impl,connectivity/source/drivers/postgresql/postgresql-sdbc-impl))

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
