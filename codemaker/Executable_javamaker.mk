# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,javamaker))

$(eval $(call gb_Executable_use_external,javamaker,boost_headers))

ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_Executable_use_externals,javamaker,\
    dtoa \
    zlib \
))
endif

$(eval $(call gb_Executable_use_libraries,javamaker,\
    unoidl \
    $(if $(filter TRUE,$(DISABLE_DYNLOADING)),reg) \
    $(if $(filter TRUE,$(DISABLE_DYNLOADING)),store) \
    salhelper \
    sal \
))

$(eval $(call gb_Executable_use_static_libraries,javamaker,\
    codemaker_java \
    codemaker \
))

$(eval $(call gb_Executable_add_exception_objects,javamaker,\
    codemaker/source/javamaker/classfile \
    codemaker/source/javamaker/javamaker \
    codemaker/source/javamaker/javaoptions \
    codemaker/source/javamaker/javatype \
))

# vim:set noet sw=4 ts=4:
