# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_PythonTest_PythonTest,sw_python))

$(eval $(call gb_PythonTest_set_defs,sw_python,\
    TDOC="$(SRCDIR)/sw/qa/complex/writer/testdocuments" \
))

$(eval $(call gb_PythonTest_add_modules,sw_python,$(SRCDIR)/sw/qa/python,\
	check_index \
	check_fields \
	check_table \
	get_expression \
	set_expression \
	var_fields \
))

# vim: set noet sw=4 ts=4:
