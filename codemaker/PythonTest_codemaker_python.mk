# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_PythonTest_PythonTest,codemaker_python))
$(eval $(call gb_PythonTest_add_modules,codemaker_python,$(SRCDIR)/codemaker/tests/pythonmaker,\
	test_pythonmaker \
))

$(call gb_PythonTest_get_target,codemaker_python): $(call gb_Executable_get_target,pythonmaker)

# vim: set noet sw=4 ts=4:
