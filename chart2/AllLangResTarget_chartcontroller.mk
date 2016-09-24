# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,chartcontroller))

$(eval $(call gb_AllLangResTarget_set_reslocation,chartcontroller,chart2))

$(eval $(call gb_AllLangResTarget_add_srs,chartcontroller,\
    chart2/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,chart2/res))

$(eval $(call gb_SrsTarget_set_include,chart2/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/chart2/source/controller/dialogs \
    -I$(SRCDIR)/chart2/source/controller/inc \
    -I$(SRCDIR)/chart2/source/controller/main \
    -I$(SRCDIR)/chart2/source/inc \
))

$(eval $(call gb_SrsTarget_add_files,chart2/res,\
    chart2/source/controller/dialogs/Strings_AdditionalControls.src \
    chart2/source/controller/dialogs/Strings_ChartTypes.src \
    chart2/source/controller/dialogs/Strings_Scale.src \
    chart2/source/controller/dialogs/Strings.src \
    chart2/source/controller/dialogs/Strings_Statistic.src \
))

$(eval $(call gb_SrsTarget_add_nonlocalizable_files,chart2/res,\
    chart2/source/controller/dialogs/Bitmaps.src \
))

# vim: set noet sw=4 ts=4:
