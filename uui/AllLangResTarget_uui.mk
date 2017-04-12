# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,uui))

$(eval $(call gb_AllLangResTarget_add_srs,uui,\
	uui/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,uui/res))

$(eval $(call gb_SrsTarget_set_include,uui/res,\
	-I$(SRCDIR)/uui/source \
	$$(INCLUDE) \
))

$(eval $(call gb_SrsTarget_add_files,uui/res,\
	uui/source/alreadyopen.src \
	uui/source/filechanged.src \
	uui/source/ids.src \
	uui/source/lockfailed.src \
	uui/source/lockcorrupt.src \
	uui/source/nameclashdlg.src \
	uui/source/openlocked.src \
	uui/source/passworddlg.src \
	uui/source/passworderrs.src \
	uui/source/trylater.src \
))

# vim: set noet sw=4 ts=4:
