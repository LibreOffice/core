# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,frm))

$(eval $(call gb_AllLangResTarget_set_reslocation,frm,forms))

$(eval $(call gb_AllLangResTarget_add_srs,frm,\
    frm/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,frm/res))

$(eval $(call gb_SrsTarget_use_packages,frm/res,\
))

$(eval $(call gb_SrsTarget_set_include,frm/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/forms/source/inc \
))

$(eval $(call gb_SrsTarget_add_files,frm/res,\
    forms/source/resource/strings.src \
    forms/source/resource/xforms.src \
))

# vim: set noet sw=4 ts=4:
