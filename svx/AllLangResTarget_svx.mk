# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,svx))

$(eval $(call gb_AllLangResTarget_set_reslocation,svx,svx))

$(eval $(call gb_AllLangResTarget_add_srs,svx,\
    svx/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,svx/res))

$(eval $(call gb_SrsTarget_set_include,svx/res,\
    $$(INCLUDE) \
    -I$(call gb_SrsTemplateTarget_get_include_dir,svx) \
    -I$(SRCDIR)/svx/source/inc \
    -I$(SRCDIR)/svx/source/dialog \
    -I$(SRCDIR)/svx/inc/ \
))

$(eval $(call gb_SrsTarget_add_files,svx/res,\
    svx/source/accessibility/accessibility.src \
    svx/source/core/graphichelper.src \
    svx/source/dialog/bmpmask.src \
    svx/source/dialog/contdlg.src \
    svx/source/dialog/compressgraphicdialog.src \
    svx/source/dialog/dlgctrl.src \
    svx/source/dialog/docrecovery.src \
    svx/source/dialog/fontwork.src \
    svx/source/dialog/frmsel.src \
    svx/source/dialog/imapdlg.src \
    svx/source/dialog/langbox.src \
    svx/source/dialog/language.src \
    svx/source/dialog/linkwarn.src \
    svx/source/dialog/passwd.src \
    svx/source/dialog/prtqry.src \
    svx/source/dialog/ruler.src \
    svx/source/dialog/sdstring.src \
    svx/source/dialog/srchdlg.src \
    svx/source/dialog/svxbmpnumvalueset.src \
    svx/source/dialog/swframeposstrings.src \
    svx/source/dialog/txenctab.src \
    svx/source/dialog/ucsubset.src \
    svx/source/engine3d/float3d.src \
    svx/source/engine3d/string3d.src \
    svx/source/fmcomp/gridctrl.src \
    svx/source/form/datanavi.src \
    svx/source/form/filtnav.src \
    svx/source/form/fmexpl.src \
    svx/source/form/fmstring.src \
    svx/source/form/formshell.src \
    svx/source/items/svxerr.src \
    svx/source/items/svxitems.src \
    svx/source/sidebar/EmptyPanel.src \
    svx/source/sidebar/area/AreaPropertyPanel.src \
    svx/source/sidebar/graphic/GraphicPropertyPanel.src \
    svx/source/sidebar/line/LinePropertyPanel.src \
    svx/source/sidebar/text/TextPropertyPanel.src \
    svx/source/sidebar/paragraph/ParaPropertyPanel.src \
    svx/source/stbctrls/stbctrls.src \
    svx/source/svdraw/svdstr.src \
    svx/source/table/table.src \
    svx/source/tbxctrls/colrctrl.src \
    svx/source/tbxctrls/extrusioncontrols.src \
    svx/source/tbxctrls/fontworkgallery.src \
    svx/source/tbxctrls/grafctrl.src \
    svx/source/tbxctrls/lboxctrl.src \
    svx/source/tbxctrls/tbcontrl.src \
    svx/source/tbxctrls/tbunosearchcontrollers.src \
    svx/source/toolbars/extrusionbar.src \
    svx/source/toolbars/fontworkbar.src \
))

$(eval $(call gb_SrsTarget_add_nonlocalized_files,svx/res,\
    svx/source/unodraw/unodraw.src \
))

$(eval $(call gb_SrsTarget_add_templates,svx/res,\
	svx/inc/globlmn_tmpl.hrc \
))

# vim: set noet sw=4 ts=4:
