# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
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

$(eval $(call gb_StaticLibrary_StaticLibrary,ulingu))

$(eval $(call gb_StaticLibrary_use_packages,ulingu,\
	comphelper_inc \
	cppu_inc \
	cppuhelper_inc \
	i18npool_inc \
	linguistic_inc \
	tools_inc \
	unotools_inc \
))

$(eval $(call gb_StaticLibrary_use_api,ulingu,\
	udkapi \
	offapi \
))

ifeq ($(SYSTEM_DICTS),YES)
$(eval $(call gb_StaticLibrary_add_defs,ulingu,\
	-DSYSTEM_DICTS -DDICT_SYSTEM_DIR=\"$(DICT_SYSTEM_DIR)\" -DHYPH_SYSTEM_DIR=\"$(HYPH_SYSTEM_DIR)\" -DTHES_SYSTEM_DIR=\"$(THES_SYSTEM_DIR)\" \
))

endif

$(eval $(call gb_StaticLibrary_use_externals,ulingu,\
	hunspell \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,ulingu,\
	lingucomponent/source/lingutil/lingutil \
))

# vim: set noet sw=4 ts=4:
