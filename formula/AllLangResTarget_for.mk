# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,for))

$(eval $(call gb_AllLangResTarget_set_reslocation,for,formula))

$(eval $(call gb_AllLangResTarget_add_srs,for,\
    formula/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,formula/res))

$(eval $(call gb_SrsTarget_use_packages,formula/res,\
	formula_inc \
	svl_inc \
))

$(eval $(call gb_SrsTarget_set_include,formula/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/formula/source/core/inc \
))

$(eval $(call gb_SrsTarget_add_files,formula/res,\
    formula/source/core/resource/core_resource.src \
))

# vim: set noet sw=4 ts=4:
