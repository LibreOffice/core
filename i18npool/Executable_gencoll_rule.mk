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

$(eval $(call gb_Executable_Executable,gencoll_rule))

$(eval $(call gb_Executable_set_include,gencoll_rule,\
	$$(INCLUDE) \
	-I$(SRCDIR)/i18npool/inc \
))

$(eval $(call gb_Executable_use_libraries,gencoll_rule,\
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_standard_system_libs,gencoll_rule))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_Executable_use_externals,gencoll_rule,\
	icudt \
	icuin \
	icuuc \
))
else
$(eval $(call gb_Executable_use_externals,gencoll_rule,\
	icudata \
	icui18n \
	icuuc \
))
endif

$(eval $(call gb_Executable_add_exception_objects,gencoll_rule,\
	i18npool/source/collator/gencoll_rule \
))

# vim: set noet sw=4 ts=4:
