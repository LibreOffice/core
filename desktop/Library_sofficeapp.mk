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
# Copyright (C) 2011 David Tardon, Red Hat Inc. <dtardon@redhat.com> (initial developer)
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

$(eval $(call gb_Library_add_precompiled_header,sofficeapp,desktop/inc/pch/precompiled_desktop.hxx))

$(eval $(call gb_Library_set_include,sofficeapp,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/inc/pch \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Library_add_api,sofficeapp,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_add_defs,sofficeapp,\
    -DDESKTOP_DLLIMPLEMENTATION \
    $(if $(firstword TRUE,$(ENABLE_GNOMEVFS)),-DGNOME_VFS_ENABLED) \
    $(if $(firstword WNT,$(GUI)),-DENABLE_QUICKSTART_APPLET) \
    $(if $(firstword aqua,$(GUIBASE)),-DENABLE_QUICKSTART_APPLET) \
    $(if $(firstword TRUE,$(ENABLE_SYSTRAY_GTK)),-DENABLE_QUICKSTART_APPLET) \
))

$(eval $(call gb_Library_set_include,sofficeapp,\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_linked_libs,sofficeapp,\
    comphelper \
    cppu \
    cppuhelper \
    deploymentmisc \
    i18nisolang1 \
    sal \
    sfx \
    svl \
    svt \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
))

$(eval $(call gb_Library_add_exception_objects,sofficeapp,\
    desktop/source/app/app \
    desktop/source/app/appfirststart \
    desktop/source/app/appinit \
    desktop/source/app/appsys \
    desktop/source/app/check_ext_deps \
    desktop/source/app/cmdlineargs \
    desktop/source/app/cmdlinehelp \
    desktop/source/app/configinit \
    desktop/source/app/desktopcontext \
    desktop/source/app/desktopresid \
    desktop/source/app/dispatchwatcher \
    desktop/source/app/langselect \
    desktop/source/app/lockfile \
    desktop/source/app/lockfile2 \
    desktop/source/app/officeipcthread \
    desktop/source/app/sofficemain \
    desktop/source/app/userinstall \
    desktop/source/migration/migration \
))

$(eval $(call gb_Library_add_cobjects,sofficeapp,\
    desktop/source/app/copyright_ascii_ooo \
    desktop/source/app/main \
))

# vim: set ts=4 sw=4 et:
