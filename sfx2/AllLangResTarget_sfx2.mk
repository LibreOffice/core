#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_AllLangResTarget_AllLangResTarget,sfx))

$(eval $(call gb_AllLangResTarget_set_reslocation,sfx,sfx2))

$(eval $(call gb_AllLangResTarget_add_srs,sfx,\
    sfx/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sfx/res))

$(eval $(call gb_SrsTarget_set_include,sfx/res,\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc \
    -I$(WORKDIR)/inc \
    -I$(realpath $(SRCDIR)/sfx2/source/dialog) \
    -I$(realpath $(SRCDIR)/sfx2/source/inc) \
    -I$(realpath $(SRCDIR)/sfx2/inc/) \
    -I$(realpath $(SRCDIR)/sfx2/inc/sfx) \
))

$(eval $(call gb_SrsTarget_add_files,sfx/res,\
    sfx2/source/appl/app.src \
    sfx2/source/appl/dde.src \
    sfx2/source/appl/newhelp.src \
    sfx2/source/appl/sfx.src \
    sfx2/source/bastyp/bastyp.src \
    sfx2/source/bastyp/fltfnc.src \
    sfx2/source/dialog/alienwarn.src \
    sfx2/source/dialog/dialog.src \
    sfx2/source/dialog/dinfdlg.src \
    sfx2/source/dialog/dinfedt.src \
    sfx2/source/dialog/filedlghelper.src \
    sfx2/source/dialog/mailwindow.src \
    sfx2/source/dialog/mgetempl.src \
    sfx2/source/dialog/newstyle.src \
    sfx2/source/dialog/passwd.src \
    sfx2/source/dialog/printopt.src \
    sfx2/source/dialog/recfloat.src \
    sfx2/source/dialog/securitypage.src \
    sfx2/source/dialog/srchdlg.src \
    sfx2/source/dialog/taskpane.src \
    sfx2/source/dialog/templdlg.src \
    sfx2/source/dialog/titledockwin.src \
    sfx2/source/dialog/versdlg.src \
    sfx2/source/doc/doc.src \
    sfx2/source/doc/doctdlg.src \
    sfx2/source/doc/doctempl.src \
    sfx2/source/doc/docvor.src \
    sfx2/source/doc/graphhelp.src \
    sfx2/source/doc/new.src \
    sfx2/source/menu/menu.src \
    sfx2/source/view/view.src \
))


