# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call 	gb_UITest_UITest,sw_options))

$(eval $(call gb_UITest_add_modules,sw_options,$(SRCDIR)/sw/qa/uitest,\
	options/ \
))

$(eval $(call gb_UITest_set_defs,sw_options, \
    TDOC="$(SRCDIR)/sw/qa/uitest/writer_tests/data" \
))
