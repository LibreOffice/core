# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,reg))

$(eval $(call gb_Library_add_defs,reg,\
	-DREG_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_include,reg, \
    $$(INCLUDE) \
    -I$(SRCDIR)/registry/inc \
))

$(eval $(call gb_Library_use_libraries,reg,\
	sal \
	store \
))

$(eval $(call gb_Library_add_exception_objects,reg,\
	registry/source/keyimpl \
	registry/source/reflread \
	registry/source/reflwrit \
	registry/source/regimpl \
	registry/source/registry \
	registry/source/regkey \
))

# vim: set noet sw=4 ts=4:
