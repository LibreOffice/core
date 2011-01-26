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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,svt))

$(eval $(call gb_AllLangResTarget_set_reslocation,svt,svtools))

$(eval $(call gb_AllLangResTarget_add_srs,svt,\
    svt/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,svt/res))

$(eval $(call gb_SrsTarget_set_include,svt/res,\
    $$(INCLUDE) \
    -I$(WORKDIR)/inc \
    -I$(SRCDIR)/svtools/source/uno \
    -I$(SRCDIR)/svtools/source/inc \
    -I$(SRCDIR)/svtools/inc/ \
    -I$(SRCDIR)/svtools/inc/svtools \
))

$(eval $(call gb_SrsTarget_add_files,svt/res,\
    svtools/source/brwbox/editbrowsebox.src \
    svtools/source/contnr/fileview.src \
    svtools/source/contnr/svcontnr.src \
    svtools/source/contnr/templwin.src \
    svtools/source/control/calendar.src \
    svtools/source/control/ctrlbox.src \
    svtools/source/control/ctrltool.src \
    svtools/source/control/filectrl.src \
    svtools/source/dialogs/addresstemplate.src \
    svtools/source/dialogs/colrdlg.src \
    svtools/source/dialogs/filedlg2.src \
    svtools/source/dialogs/formats.src \
    svtools/source/dialogs/prnsetup.src \
    svtools/source/dialogs/so3res.src \
    svtools/source/dialogs/wizardmachine.src \
    svtools/source/filter.vcl/filter/exportdialog.src \
    svtools/source/java/javaerror.src \
    svtools/source/misc/ehdl.src \
    svtools/source/misc/helpagent.src \
    svtools/source/misc/imagemgr.src \
    svtools/source/misc/langtab.src \
    svtools/source/misc/undo.src \
    svtools/source/plugapp/testtool.src \
    svtools/source/toolpanel/toolpanel.src \
))


# vim: set noet sw=4 ts=4:
