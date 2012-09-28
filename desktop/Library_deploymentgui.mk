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

$(eval $(call gb_Library_Library,deploymentgui))

$(eval $(call gb_Library_set_include,deploymentgui,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/source/deployment/inc \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Library_use_sdk_api,deploymentgui))

$(eval $(call gb_Library_use_libraries,deploymentgui,\
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
    svxcore \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Library_use_system_win32_libs,deploymentgui,\
    ole32 \
))

endif


$(eval $(call gb_Library_set_componentfile,deploymentgui,desktop/source/deployment/gui/deploymentgui))

$(eval $(call gb_Library_add_exception_objects,deploymentgui,\
    desktop/source/deployment/gui/descedit \
    desktop/source/deployment/gui/dp_gui_autoscrolledit \
    desktop/source/deployment/gui/dp_gui_dependencydialog \
    desktop/source/deployment/gui/dp_gui_dialog2 \
    desktop/source/deployment/gui/dp_gui_extensioncmdqueue \
    desktop/source/deployment/gui/dp_gui_extlistbox \
    desktop/source/deployment/gui/dp_gui_service \
    desktop/source/deployment/gui/dp_gui_theextmgr \
    desktop/source/deployment/gui/dp_gui_updatedialog \
    desktop/source/deployment/gui/dp_gui_updateinstalldialog \
    desktop/source/deployment/gui/license_dialog \
))

# vim: set ts=4 sw=4 et:
