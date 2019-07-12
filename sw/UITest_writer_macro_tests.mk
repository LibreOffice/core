# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
# 

$(eval $(call 	gb_UITest_UITest,macro_tests))

$(eval $(call gb_UITest_add_modules,macro_tests,$(SRCDIR)/sw/qa/uitest,\
	macro_tests/ \
))

$(eval $(call gb_UITest_set_defs,macro_tests, \
    TDOC="$(SRCDIR)/sw/qa/uitest/macro_tests/data" \
))
