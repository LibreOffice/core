# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#


$(eval $(call gb_AllLangResTarget_AllLangResTarget,sm))

$(eval $(call gb_AllLangResTarget_set_reslocation,sm,starmath))

$(eval $(call gb_AllLangResTarget_add_srs,sm,\
	sm/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sm/res))

$(eval $(call gb_SrsTarget_set_include,sm/res,\
	-I$(SRCDIR)/starmath/inc \
    -I$(call gb_SrsTemplateTarget_get_include_dir,) \
	$$(INCLUDE) \
))

$(eval $(call gb_SrsTarget_add_files,sm/res,\
    starmath/source/smres.src \
    starmath/source/commands.src \
    starmath/source/symbol.src \
	starmath/source/toolbox.src \
))

# vim: set noet sw=4 ts=4:
