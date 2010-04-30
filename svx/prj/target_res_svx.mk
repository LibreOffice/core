#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_AllLangResTarget_AllLangResTarget,svx))

$(eval $(call gb_AllLangResTarget_set_reslocation,svx,svx))
#$(eval $(call gb_AllLangResTarget_set_reslocation,svx,svx/source/svdraw))

$(eval $(call gb_AllLangResTarget_add_srs,svx,\
    svx/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,svx/res))

$(eval $(call gb_SrsTarget_set_include,svx/res,\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc \
    -I$(WORKDIR)/inc \
    -I$(SRCDIR)/svx/source/inc \
    -I$(SRCDIR)/svx/source/dialog \
    -I$(SRCDIR)/svx/inc/ \
    -I$(SRCDIR)/svx/unxlngi6.pro/inc/ \
))

#svx/source/src/errtxt.src
#svx/source/src/app.src

#svx/source/intro/iso.src
#svx/source/intro/ooo.src

#svx/source/unodialogs/textconversiondlgs/chinese_dictionarydialog.src
#svx/source/unodialogs/textconversiondlgs/chinese_translationdialog.src

$(eval $(call gb_SrsTarget_add_files,svx/res,\
    svx/source/stbctrls/stbctrls.src \
    svx/source/dialog/passwd.src \
    svx/source/dialog/language.src \
    svx/source/dialog/bmpmask.src \
    svx/source/dialog/ucsubset.src \
    svx/source/dialog/hdft.src \
    svx/source/dialog/imapdlg.src \
    svx/source/dialog/swframeposstrings.src \
    svx/source/dialog/prtqry.src \
    svx/source/dialog/txenctab.src \
    svx/source/dialog/dlgctrl.src \
    svx/source/dialog/frmsel.src \
    svx/source/dialog/ctredlin.src \
    svx/source/dialog/sdstring.src \
    svx/source/dialog/ruler.src \
    svx/source/dialog/srchdlg.src \
    svx/source/dialog/optgrid.src \
    svx/source/dialog/langbox.src \
    svx/source/dialog/contdlg.src \
    svx/source/dialog/rubydialog.src \
    svx/source/dialog/fontwork.src \
    svx/source/dialog/docrecovery.src \
    svx/source/dialog/hyprlink.src \
    svx/source/dialog/svxbmpnumvalueset.src \
    svx/source/engine3d/string3d.src \
    svx/source/engine3d/float3d.src \
    svx/source/mnuctrls/mnuctrls.src \
    svx/source/tbxctrls/linectrl.src \
    svx/source/tbxctrls/colrctrl.src \
    svx/source/tbxctrls/extrusioncontrols.src \
    svx/source/tbxctrls/tbxdraw.src \
    svx/source/tbxctrls/grafctrl.src \
    svx/source/tbxctrls/lboxctrl.src \
    svx/source/tbxctrls/tbcontrl.src \
    svx/source/tbxctrls/fontworkgallery.src \
    svx/source/fmcomp/gridctrl.src \
    svx/source/form/fmexpl.src \
    svx/source/form/formshell.src \
    svx/source/form/fmstring.src \
    svx/source/form/datanavi.src \
    svx/source/form/filtnav.src \
    svx/source/unodraw/unodraw.src \
    svx/source/table/table.src \
    svx/source/toolbars/extrusionbar.src \
    svx/source/toolbars/fontworkbar.src \
    svx/source/gallery2/galtheme.src \
    svx/source/gallery2/gallery.src \
    svx/source/svdraw/svdstr.src \
    svx/source/accessibility/accessibility.src \
    svx/source/items/svxitems.src \
    svx/source/items/svxerr.src \
))


