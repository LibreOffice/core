# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,purpenvhelper))

$(eval $(call gb_Library_set_soversion_script,purpenvhelper,$(SRCDIR)/cppu/util/uno_purpenvhelpergcc3.map))

$(eval $(call gb_Library_add_defs,purpenvhelper,\
	-DPURPENV_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_udk_api,purpenvhelper))

$(eval $(call gb_Library_use_libraries,purpenvhelper,\
	cppu \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,purpenvhelper,\
	cppu/source/helper/purpenv/helper_purpenv_Environment \
	cppu/source/helper/purpenv/helper_purpenv_Mapping \
	cppu/source/helper/purpenv/helper_purpenv_Proxy \
))

# vim: set noet sw=4 ts=4:
