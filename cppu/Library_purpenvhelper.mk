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
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com>
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

$(eval $(call gb_Library_Library,purpenvhelper))

$(eval $(call gb_Library_set_soversion_script,purpenvhelper,3,$(SRCDIR)/cppu/util/uno_purpenvhelpergcc3.map))

$(eval $(call gb_Library_add_defs,purpenvhelper,\
	-DPURPENV_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,purpenvhelper))

$(eval $(call gb_Library_use_libraries,purpenvhelper,\
	cppu \
	sal \
	$(gb_UWINAPI) \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,purpenvhelper,\
	cppu/source/helper/purpenv/helper_purpenv_Environment \
	cppu/source/helper/purpenv/helper_purpenv_Mapping \
	cppu/source/helper/purpenv/helper_purpenv_Proxy \
))

# vim: set noet sw=4 ts=4:
