# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,kitgtk))

$(eval $(call gb_Library_use_sdk_api,kitgtk))

$(eval $(call gb_Library_add_exception_objects,kitgtk,\
    kit/source/gtk/kitdocview \
    kit/source/gtk/tilebuffer \
))

$(eval $(call gb_Library_use_externals,kitgtk,\
    boost_headers \
))

$(eval $(call gb_Library_set_include,kitgtk,\
    $$(INCLUDE) \
    $$(GTK3_CFLAGS) \
))

$(eval $(call gb_Library_add_libs,kitgtk,\
    $(GTK3_LIBS) \
))

$(eval $(call gb_Library_add_defs,kitgtk,\
	-DKIT_PATH="\"$(LIBDIR)/libreoffice/$(LIBO_LIB_FOLDER)\"" \
	-DKIT_DOC_VIEW_IMPLEMENTATION \
))

ifeq ($(OS),$(filter LINUX %BSD SOLARIS, $(OS)))
$(eval $(call gb_Library_add_libs,kitgtk,\
    $(UNIX_DLAPI_LIBS) -lm \
))
endif

$(eval $(call gb_Library_use_packages,kitgtk, \
    kit_selectionhandles \
))

# vim: set noet sw=4 ts=4:
