# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,cppumaker))

$(eval $(call gb_Executable_use_external,cppumaker,boost_headers))

$(eval $(call gb_Executable_use_libraries,cppumaker,\
    unoidl \
    $(if $(filter TRUE,$(DISABLE_DYNLOADING)),reg) \
    $(if $(filter TRUE,$(DISABLE_DYNLOADING)),store) \
    salhelper \
    sal \
))

$(eval $(call gb_Executable_use_static_libraries,cppumaker,\
    codemaker_cpp \
    codemaker \
))

$(eval $(call gb_Executable_add_exception_objects,cppumaker,\
    codemaker/source/cppumaker/cppumaker \
    codemaker/source/cppumaker/cppuoptions \
    codemaker/source/cppumaker/cpputype \
    codemaker/source/cppumaker/dependencies \
    codemaker/source/cppumaker/dumputils \
    codemaker/source/cppumaker/includes \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Executable_add_cxxflags,cppumaker,\
	-Ob0 \
))
endif

# vim:set noet sw=4 ts=4:
