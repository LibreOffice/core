# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,gtktiledviewer))

$(eval $(call gb_Executable_set_include,gtktiledviewer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
))

$(eval $(call gb_Executable_use_externals,gtktiledviewer,\
    boost_headers \
))

$(eval $(call gb_Executable_add_cxxflags,gtktiledviewer,\
    $$(GTK3_CFLAGS) \
))

$(eval $(call gb_Executable_add_libs,gtktiledviewer,\
    $(GTK3_LIBS) \
))

$(eval $(call gb_Executable_use_libraries,gtktiledviewer,\
    libreofficekitgtk \
))

$(eval $(call gb_Executable_add_libs,gtktiledviewer,\
    -lX11 \
    -lXext \
    -lXrender \
    -lSM \
    -lICE \
))

ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))
$(eval $(call gb_Executable_add_libs,gtktiledviewer,\
    -lm \
    $(if $(DLOPEN_NEEDS_LIBDL), -ldl) \
    -lpthread \
))
endif

$(eval $(call gb_Executable_add_exception_objects,gtktiledviewer,\
    libreofficekit/qa/gtktiledviewer/gtktiledviewer \
))

# vim: set noet sw=4 ts=4:
