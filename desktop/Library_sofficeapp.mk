# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,sofficeapp))

$(eval $(call gb_Library_set_include,sofficeapp,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/source/inc \
    -I$(SRCDIR)/desktop/source/deployment/inc \
))

$(eval $(call gb_Library_use_sdk_api,sofficeapp))

$(eval $(call gb_Library_add_defs,sofficeapp,\
    -DDESKTOP_DLLIMPLEMENTATION \
    $(if $(filter TRUE,$(ENABLE_GNOMEVFS)),-DGNOME_VFS_ENABLED) \
    $(if $(filter WNT,$(GUI)),-DENABLE_QUICKSTART_APPLET) \
    $(if $(filter aqua,$(GUIBASE)),-DENABLE_QUICKSTART_APPLET) \
    $(if $(filter TRUE,$(ENABLE_SYSTRAY_GTK)),-DENABLE_QUICKSTART_APPLET) \
))

$(eval $(call gb_Library_use_libraries,sofficeapp,\
    comphelper \
    cppu \
    cppuhelper \
    deploymentmisc \
    i18nisolang1 \
    sal \
    salhelper \
    sfx \
    svl \
    svt \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

ifeq ($(GUIBASE),cocoatouch)
$(eval $(call gb_Library_add_cflags,sofficeapp,\
    $(gb_OBJCFLAGS) \
))
endif

$(eval $(call gb_Library_add_exception_objects,sofficeapp,\
    desktop/source/app/app \
    desktop/source/app/appfirststart \
    desktop/source/app/appinit \
    desktop/source/app/check_ext_deps \
    desktop/source/app/cmdlineargs \
    desktop/source/app/cmdlinehelp \
    desktop/source/app/configinit \
    desktop/source/app/desktopcontext \
    desktop/source/app/desktopresid \
    desktop/source/app/dispatchwatcher \
    desktop/source/app/langselect \
    desktop/source/app/lockfile2 \
    desktop/source/app/officeipcthread \
    desktop/source/app/sofficemain \
    desktop/source/app/userinstall \
    desktop/source/migration/migration \
))

ifeq ($(ENABLE_TELEPATHY),TRUE)
$(eval $(call gb_Library_use_libraries,sofficeapp,tubes))
endif

#
# We need the lo_main symbol for our boostrap loader
#
ifeq ($(OS),ANDROID)
$(eval $(call gb_Library_add_cobjects,sofficeapp,\
    desktop/source/app/main \
))
endif

# vim: set ts=4 sw=4 et:
