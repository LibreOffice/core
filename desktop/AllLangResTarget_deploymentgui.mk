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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,deploymentgui))

$(eval $(call gb_AllLangResTarget_set_reslocation,deploymentgui,desktop))

$(eval $(call gb_AllLangResTarget_add_srs,deploymentgui,\
    deploymentgui/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,deploymentgui/res))

$(eval $(call gb_SrsTarget_set_include,deploymentgui/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/source/deployment/registry/inc \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_SrsTarget_add_files,deploymentgui/res,\
    desktop/source/deployment/gui/dp_gui_backend.src \
    desktop/source/deployment/gui/dp_gui_dependencydialog.src \
    desktop/source/deployment/gui/dp_gui_dialog2.src \
    desktop/source/deployment/gui/dp_gui_dialog.src \
    desktop/source/deployment/gui/dp_gui_updatedialog.src \
    desktop/source/deployment/gui/dp_gui_updateinstalldialog.src \
    desktop/source/deployment/gui/dp_gui_versionboxes.src \
))

# vim: set ts=4 sw=4 et:
