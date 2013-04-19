# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,deploymentgui))

$(eval $(call gb_AllLangResTarget_set_reslocation,deploymentgui,desktop))

$(eval $(call gb_AllLangResTarget_add_srs,deploymentgui,\
    deploymentgui/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,deploymentgui/res))

$(eval $(call gb_SrsTarget_use_packages,deploymentgui/res,\
))

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
