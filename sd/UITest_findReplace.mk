# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call 	gb_UITest_UITest,findReplace))

$(eval $(call gb_UITest_add_modules,findReplace,$(SRCDIR)/sd/qa/uitest,\
	findReplace/ \
))

$(eval $(call gb_UITest_set_defs,findReplace, \
    TDOC="$(SRCDIR)/sd/qa/uitest/impress_tests/data" \
))
