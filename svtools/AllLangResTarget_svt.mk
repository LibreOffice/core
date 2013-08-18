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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,svt))

$(eval $(call gb_AllLangResTarget_set_reslocation,svt,svtools))

$(eval $(call gb_AllLangResTarget_add_srs,svt,\
    svt/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,svt/res))

$(eval $(call gb_SrsTarget_set_include,svt/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/svtools/source/inc \
    -I$(SRCDIR)/svtools/inc \
))

$(eval $(call gb_SrsTarget_add_files,svt/res,\
    svtools/source/contnr/fileview.src \
    svtools/source/contnr/svcontnr.src \
    svtools/source/contnr/templwin.src \
    svtools/source/control/calendar.src \
    svtools/source/control/ctrlbox.src \
    svtools/source/control/ctrltool.src \
    svtools/source/control/filectrl.src \
    svtools/source/dialogs/addresstemplate.src \
	svtools/source/dialogs/filedlg2.src \
    svtools/source/dialogs/formats.src \
    svtools/source/dialogs/so3res.src \
    svtools/source/dialogs/wizardmachine.src \
    $(if $(ENABLE_JAVA), \
        svtools/source/java/javaerror.src) \
    svtools/source/misc/ehdl.src \
    svtools/source/misc/imagemgr.src \
    svtools/source/misc/langtab.src \
    svtools/source/misc/svtools.src \
    svtools/source/misc/undo.src \
    svtools/source/toolpanel/toolpanel.src \
))

$(eval $(call gb_SrsTarget_add_nonlocalized_files,svt/res,\
    svtools/source/brwbox/editbrowsebox.src \
))

# vim: set noet sw=4 ts=4:
