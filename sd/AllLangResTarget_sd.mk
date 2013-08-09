# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,sd))

$(eval $(call gb_AllLangResTarget_set_reslocation,sd,sd))

$(eval $(call gb_AllLangResTarget_add_srs,sd,\
    sd/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sd/res))

$(eval $(call gb_SrsTarget_use_srstargets,sd/res,\
	svx/res \
))

$(eval $(call gb_SrsTarget_set_include,sd/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/source/ui/slidesorter/inc \
    -I$(call gb_SrsTemplateTarget_get_include_dir,sd) \
    -I$(call gb_SrsTemplateTarget_get_include_dir,) \
))

$(eval $(call gb_SrsTarget_add_files,sd/res,\
    sd/source/core/glob.src \
    sd/source/filter/html/pubdlg.src \
    sd/source/ui/accessibility/accessibility.src \
    sd/source/ui/animations/CustomAnimationDialog.src \
    sd/source/ui/animations/CustomAnimationPane.src \
    sd/source/ui/animations/CustomAnimation.src \
    sd/source/ui/animations/SlideTransitionPane.src \
    sd/source/ui/annotations/annotations.src \
    sd/source/ui/app/popup.src \
    sd/source/ui/app/res_bmp.src \
    sd/source/ui/app/sdstring.src \
    sd/source/ui/app/strings.src \
    sd/source/ui/app/toolbox.src \
    sd/source/ui/dlg/animobjs.src \
    sd/source/ui/dlg/brkdlg.src \
    sd/source/ui/dlg/dlgass.src \
    sd/source/ui/dlg/dlg_char.src \
    sd/source/ui/dlg/dlgfield.src \
    sd/source/ui/dlg/dlgolbul.src \
    sd/source/ui/dlg/dlgpage.src \
    sd/source/ui/dlg/headerfooterdlg.src \
    sd/source/ui/dlg/inspagob.src \
    sd/source/ui/dlg/ins_paste.src \
    sd/source/ui/dlg/LayerDialog.src \
    sd/source/ui/dlg/morphdlg.src \
    sd/source/ui/dlg/navigatr.src \
    sd/source/ui/dlg/PaneDockingWindow.src \
    sd/source/ui/dlg/prltempl.src \
    sd/source/ui/dlg/RemoteDialog.src \
    sd/source/ui/dlg/sdpreslt.src \
    sd/source/ui/dlg/tpaction.src \
    sd/source/ui/dlg/vectdlg.src \
    sd/source/ui/slideshow/slideshow.src \
    sd/source/ui/table/TableDesignPane.src \
    sd/source/ui/view/DocumentRenderer.src \
))

$(eval $(call gb_SrsTarget_add_nonlocalized_files,sd/res,\
    sd/source/ui/slidesorter/view/SlsResource.src \
))

$(eval $(call gb_SrsTarget_add_templates,sd/res,\
    sd/source/ui/app/menuids3_tmpl.src \
    sd/source/ui/app/menuids_tmpl.src \
    sd/source/ui/app/popup2_tmpl.src \
    sd/source/ui/app/tbxids_tmpl.src \
    sd/source/ui/app/toolbox2_tmpl.src \
))

# vim: set noet sw=4 ts=4:
