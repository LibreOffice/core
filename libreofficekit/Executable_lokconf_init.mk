# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,lokconf_init))

$(eval $(call gb_Executable_set_include,lokconf_init,\
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,lokconf_init, \
	cppu \
	sal \
))

$(eval $(call gb_Executable_use_static_libraries,lokconf_init,\
    libreofficekit \
))

$(eval $(call gb_Executable_add_libs,lokconf_init,\
    -lSM \
    -lICE \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Executable_add_libs,lokconf_init,\
    -lm \
    -ldl \
    -lpthread \
))
endif

$(eval $(call gb_Executable_add_exception_objects,lokconf_init,\
    libreofficekit/qa/lokconf_init/lokconf_init \
))

# vim: set noet sw=4 ts=4:
