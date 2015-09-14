# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_PythonTest_PythonTest,sfx2_python))

$(eval $(call gb_PythonTest_set_defs,sfx2_python,\
    TDOC="$(SRCDIR)/sfx2/qa/python/testdocuments" \
))

$(eval $(call gb_PythonTest_add_modules,sfx2_python,$(SRCDIR)/sfx2/qa/python,\
	check_sidebar \
	check_sidebar_registry \
))

# vim: set noet sw=4 ts=4:
