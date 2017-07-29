# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,gtktiledviewer))

$(eval $(call gb_Library_use_sdk_api,gtktiledviewer))

$(eval $(call gb_Executable_set_include,gtktiledviewer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/libreofficekit/qa/gtktiledviewer/ \
    -I$(WORKDIR)/UnoApiHeadersTarget/offapi/normal/ \
    -I$(WORKDIR)/UnoApiHeadersTarget/udkapi/normal/ \
))

$(eval $(call gb_Executable_use_externals,gtktiledviewer,\
    boost_headers \
))

$(eval $(call gb_Executable_add_cxxflags,gtktiledviewer,\
    $$(GTK3_CFLAGS) \
))

$(eval $(call gb_Executable_add_libs,gtktiledviewer,\
    $(GTK3_LIBS) \
    -lX11 \
    -lXext \
    -lXrender \
    -lSM \
    -lICE \
))

$(eval $(call gb_Executable_use_libraries,gtktiledviewer,\
    libreofficekitgtk \
))

ifeq ($(OS), $(filter LINUX %BSD SOLARIS, $(OS)))
$(eval $(call gb_Executable_add_libs,gtktiledviewer,\
    -lm $(DLOPEN_LIBS) \
    -lpthread \
))
endif

$(eval $(call gb_Executable_add_exception_objects,gtktiledviewer,\
    libreofficekit/qa/gtktiledviewer/gtv-main \
    libreofficekit/qa/gtktiledviewer/gtv-application \
    libreofficekit/qa/gtktiledviewer/gtv-application-window \
    libreofficekit/qa/gtktiledviewer/gtv-main-toolbar \
    libreofficekit/qa/gtktiledviewer/gtv-signal-handlers \
    libreofficekit/qa/gtktiledviewer/gtv-helpers \
    libreofficekit/qa/gtktiledviewer/gtv-lokdocview-signal-handlers \
    libreofficekit/qa/gtktiledviewer/gtv-calc-header-bar \
    libreofficekit/qa/gtktiledviewer/gtv-comments-sidebar \
    libreofficekit/qa/gtktiledviewer/gtv-lok-dialog \
))

# vim: set noet sw=4 ts=4:
