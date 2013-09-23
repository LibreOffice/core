# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,rsc))

$(eval $(call gb_Executable_use_external,rsc,boost_headers))

$(eval $(call gb_Executable_set_include,rsc,\
	$$(INCLUDE) \
	-I$(SRCDIR)/rsc/inc \
))

$(eval $(call gb_Executable_use_sdk_api,rsc))

$(eval $(call gb_Executable_add_defs,rsc,\
	-DSOLAR \
))

$(eval $(call gb_Executable_use_libraries,rsc,\
	i18nlangtag \
	comphelper \
	sal \
	tl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_use_externals,rsc,\
    icudata \
    icuuc \
    icui18n \
))

$(eval $(call gb_Executable_add_grammars,rsc,\
	rsc/source/parser/rscyacc \
))

$(eval $(call gb_Executable_add_exception_objects,rsc,\
	rsc/source/parser/erscerr \
	rsc/source/parser/rscdb \
	rsc/source/parser/rscibas \
	rsc/source/parser/rscicpx \
	rsc/source/parser/rscinit \
	rsc/source/parser/rsckey \
	rsc/source/parser/rsclex \
	rsc/source/parser/rscpar \
	rsc/source/prj/gui \
	rsc/source/prj/start \
	rsc/source/res/rscall \
	rsc/source/res/rscarray \
	rsc/source/res/rscclass \
	rsc/source/res/rscclobj \
	rsc/source/res/rscconst \
	rsc/source/res/rsccont \
	rsc/source/res/rscflag \
	rsc/source/res/rscmgr \
	rsc/source/res/rscrange \
	rsc/source/res/rscstr \
	rsc/source/res/rsctop \
	rsc/source/rsc/rsc \
	rsc/source/tools/rscchar \
	rsc/source/tools/rscdef \
	rsc/source/tools/rschash \
	rsc/source/tools/rsctools \
	rsc/source/tools/rsctree \
))

$(eval $(call gb_Executable_add_cobjects,rsc,\
	rsc/source/rscpp/cpp1 \
	rsc/source/rscpp/cpp2 \
	rsc/source/rscpp/cpp3 \
	rsc/source/rscpp/cpp4 \
	rsc/source/rscpp/cpp5 \
	rsc/source/rscpp/cpp6 \
))

ifneq ($(OS),WNT)
$(eval $(call gb_Executable_add_defs,rsc,\
	-Dunix \
))
endif

# vim: set noet sw=4 ts=4:
