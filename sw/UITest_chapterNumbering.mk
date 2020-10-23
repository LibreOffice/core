# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call 	gb_UITest_UITest,chapterNumbering))

$(eval $(call gb_UITest_add_modules,chapterNumbering,$(SRCDIR)/sw/qa/uitest,\
	chapterNumbering/ \
))

$(eval $(call gb_UITest_set_defs,chapterNumbering, \
    TDOC="$(SRCDIR)/sw/qa/uitest/data" \
))
