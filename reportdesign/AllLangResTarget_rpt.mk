# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,rpt))

$(eval $(call gb_AllLangResTarget_set_reslocation,rpt,reportdesign))

$(eval $(call gb_AllLangResTarget_add_srs,rpt,\
    reportdesign/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,reportdesign/res))

$(eval $(call gb_SrsTarget_use_packages,reportdesign/res,\
	svl_inc \
	svx_globlmn_hrc \
))

$(eval $(call gb_SrsTarget_set_include,reportdesign/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/reportdesign/source/core/inc \
))

$(eval $(call gb_SrsTarget_add_files,reportdesign/res,\
    reportdesign/source/core/resource/strings.src \
))

# vim: set noet sw=4 ts=4:
