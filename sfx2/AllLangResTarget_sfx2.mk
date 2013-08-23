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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,sfx))

$(eval $(call gb_AllLangResTarget_set_reslocation,sfx,sfx2))

$(eval $(call gb_AllLangResTarget_add_srs,sfx,\
    sfx/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sfx/res))

$(eval $(call gb_SrsTarget_set_include,sfx/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sfx2/source/dialog \
    -I$(SRCDIR)/sfx2/source/inc \
    -I$(SRCDIR)/sfx2/inc/ \
))

$(eval $(call gb_SrsTarget_add_files,sfx/res,\
    sfx2/source/appl/app.src \
    sfx2/source/appl/dde.src \
    sfx2/source/appl/newhelp.src \
    sfx2/source/appl/sfx.src \
    sfx2/source/bastyp/bastyp.src \
    sfx2/source/bastyp/fltfnc.src \
    sfx2/source/control/templateview.src \
    sfx2/source/dialog/alienwarn.src \
    sfx2/source/dialog/dialog.src \
    sfx2/source/dialog/dinfdlg.src \
    sfx2/source/dialog/filedlghelper.src \
    sfx2/source/dialog/inputdlg.src \
    sfx2/source/dialog/newstyle.src \
    sfx2/source/dialog/recfloat.src \
    sfx2/source/dialog/srchdlg.src \
    sfx2/source/dialog/taskpane.src \
    sfx2/source/dialog/templateinfodlg.src \
    sfx2/source/dialog/templdlg.src \
    sfx2/source/dialog/titledockwin.src \
    sfx2/source/dialog/versdlg.src \
    sfx2/source/doc/doc.src \
    sfx2/source/doc/doctempl.src \
    sfx2/source/doc/graphhelp.src \
    sfx2/source/doc/new.src \
    sfx2/source/doc/templatelocnames.src \
    sfx2/source/doc/templatedlg.src \
    sfx2/source/menu/menu.src \
    sfx2/source/sidebar/Sidebar.src \
    sfx2/source/view/view.src \
))

# vim: set noet sw=4 ts=4:
