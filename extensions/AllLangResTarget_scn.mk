# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,scn))

$(eval $(call gb_AllLangResTarget_set_reslocation,scn,extensions/source/scanner))

$(eval $(call gb_AllLangResTarget_add_srs,scn,\
	scn/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,scn/res))

$(eval $(call gb_SrsTarget_set_include,scn/res,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/source/scanner \
))

$(eval $(call gb_SrsTarget_add_files,scn/res,\
	extensions/source/scanner/grid.src \
	extensions/source/scanner/sanedlg.src \
))

# vim:set noet sw=4 ts=4:
