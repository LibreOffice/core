# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_PythonTest_PythonTest,sc_python))

$(eval $(call gb_PythonTest_add_modules,sc_python,$(SRCDIR)/sc/qa/python,\
	CalcRTL \
))

# vim: set noet sw=4 ts=4: