# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,icontest))

$(eval $(call gb_Executable_use_externals,icontest,\
    boost_headers \
    glew \
	glm_headers \
	$(if $(filter WNT MACOSX,$(OS)),mesa_headers) \
))

$(eval $(call gb_Executable_use_api,icontest,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_static_libraries,icontest,\
    vclmain \
))

ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))
$(eval $(call gb_Executable_add_libs,icontest,\
	-lm $(DLOPEN_LIBS) \
	-lpthread \
    -lX11 \
))

$(eval $(call gb_Executable_use_static_libraries,icontest,\
	glxtest \
))
else ifeq ($(OS),WNT)

$(eval $(call gb_Executable_use_system_win32_libs,icontest,\
    opengl32 \
))

else ifeq ($(OS),MACOSX)

$(eval $(call gb_Executable_add_libs,icontest,\
    -framework OpenGL \
))

endif

$(eval $(call gb_Executable_use_libraries,icontest,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
    ucbhelper \
    vcl \
))

$(eval $(call gb_Executable_add_exception_objects,icontest,\
    vcl/workben/icontest \
))

# vim: set noet sw=4 ts=4:
