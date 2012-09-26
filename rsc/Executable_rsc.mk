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
# The Initial Developer of the Original Code is
#       Matúš Kukan <matus.kukan@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Executable_Executable,rsc))

$(eval $(call gb_Executable_set_include,rsc,\
	$$(INCLUDE) \
	-I$(SRCDIR)/rsc/inc \
))

$(eval $(call gb_Executable_use_sdk_api,rsc))

$(eval $(call gb_Executable_add_defs,rsc,\
	-DSOLAR \
))

$(eval $(call gb_Executable_use_libraries,rsc,\
	i18nisolang1 \
	comphelper \
	sal \
	tl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_standard_system_libs,rsc))

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

ifeq ($(GUI),UNX)
$(eval $(call gb_Executable_add_defs,rsc,\
	-Dunix \
))
endif

# vim: set noet sw=4 ts=4:
