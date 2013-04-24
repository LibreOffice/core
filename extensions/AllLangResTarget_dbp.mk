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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,dbp))

$(eval $(call gb_AllLangResTarget_add_srs,dbp,dbp/res))

$(eval $(call gb_SrsTarget_SrsTarget,dbp/res))

$(eval $(call gb_SrsTarget_set_include,dbp/res,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/inc \
))

$(eval $(call gb_SrsTarget_add_files,dbp/res,\
	extensions/source/dbpilots/commonpagesdbp.src \
	extensions/source/dbpilots/dbpilots.src \
	extensions/source/dbpilots/gridpages.src \
	extensions/source/dbpilots/groupboxpages.src \
	extensions/source/dbpilots/listcombopages.src \
))

# vim:set noet sw=4 ts=4:
