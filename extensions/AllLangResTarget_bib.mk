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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,bib))

$(eval $(call gb_AllLangResTarget_add_srs,bib,bib/res))

$(eval $(call gb_SrsTarget_SrsTarget,bib/res))

$(eval $(call gb_SrsTarget_set_include,bib/res,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/inc \
))

$(eval $(call gb_SrsTarget_add_files,bib/res,\
	extensions/source/bibliography/bib.src \
	extensions/source/bibliography/datman.src \
	extensions/source/bibliography/sections.src \
	extensions/source/bibliography/toolbar.src \
))

# vim:set noet sw=4 ts=4:
