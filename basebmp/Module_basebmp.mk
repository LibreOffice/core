# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,basebmp))

ifeq ($(OS),ANDROID)
$(eval $(call gb_Module_add_targets,basebmp,\
    StaticLibrary_basebmp \
))
else
$(eval $(call gb_Module_add_targets,basebmp,\
    Library_basebmp \
))
endif

$(eval $(call gb_Module_add_targets,basebmp,\
    Package_inc \
))

$(eval $(call gb_Module_add_check_targets,basebmp,\
    CppunitTest_basebmp \
))

# vim: set noet sw=4 ts=4:
