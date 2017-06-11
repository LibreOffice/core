# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_Module_Module,i18nlangtag))

$(eval $(call gb_Module_add_targets,i18nlangtag,\
	Library_i18nlangtag \
	Executable_localestr \
))

$(eval $(call gb_Module_add_check_targets,i18nlangtag,\
	CppunitTest_i18nlangtag_test_languagetag \
))

# vim: set noet sw=4 ts=4:
