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
    TDOC="$(SRCDIR)/sw/qa/python/testdocuments" \
))

$(eval $(call gb_PythonTest_add_modules,sw_python,$(SRCDIR)/sw/qa/python,\
	check_bookmarks \
	check_change_color \
	check_index \
	check_flies \
	check_fields \
	check_cross_references \
	check_named_property_values \
	check_indexed_property_values \
	check_styles \
	check_table \
	get_expression \
	set_expression \
	text_portion_enumeration_test \
	var_fields \
))

# vim: set noet sw=4 ts=4:
