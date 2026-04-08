# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Golden-file tests for both cppumaker and pythonmaker (see README.md there)
$(eval $(call gb_PythonTest_PythonTest,codemaker))
$(eval $(call gb_PythonTest_add_modules,codemaker,$(SRCDIR)/codemaker/tests,\
	codemakertests \
))

$(call gb_PythonTest_get_target,codemaker): \
	$(call gb_Executable_get_target,cppumaker) \
	$(call gb_Executable_get_target,pythonmaker)

# vim: set noet sw=4 ts=4:
