# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,clew))

$(eval $(call gb_Library_add_defs,clew,\
	-DCLEW_BUILD \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,clew,\
	-ldl \
	-lrt \
))
endif

ifeq ($(OS),MACOSX)
ifneq (1050,$(MACOSX_SDK_VERSION))
$(eval $(call gb_Library_add_libs,clew,\
    -framework OpenCL \
))
endif
endif

$(eval $(call gb_Library_add_cobjects,clew,\
    external/clew/source/clew \
))

$(eval $(call gb_Library_set_include,clew, \
    -I$(SRCDIR)/external/clew/source/include \
    $$(INCLUDE) \
))

# This is required for module-deps.pl to produce correct dependencies.
$(eval $(call gb_Library_use_libraries,clew,))

# vim: set noet sw=4 ts=4:
