# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,demoshow))

$(eval $(call gb_Executable_use_package,demoshow,sd_qa_unit))

$(eval $(call gb_Executable_set_include,demoshow,\
    $$(INCLUDE) \
    -I$(SRCDIR)/slideshow/source/inc \
))

ifeq ($(strip $(VERBOSE)),TRUE)
$(eval $(call gb_Executable_set_defs,demoshow,\
    $$(DEFS) \
    -DVERBOSE \
))
endif

ifneq ($(strip $(debug)$(DEBUG)),)
$(eval $(call gb_Executable_set_defs,demoshow,\
    $$(DEFS) \
    -DBOOST_SP_ENABLE_DEBUG_HOOKS \
))
endif

$(eval $(call gb_Library_use_sdk_api,demoshow))

$(eval $(call gb_Executable_use_libraries,demoshow,\
    basegfx \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    sal \
    tl \
    ucbhelper \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_exception_objects,demoshow,\
    slideshow/test/demoshow \
))

# vim: set noet sw=4 ts=4:
