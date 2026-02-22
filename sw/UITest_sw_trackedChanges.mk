# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call 	gb_UITest_UITest,sw_trackedChanges))

$(eval $(call gb_UITest_add_modules,sw_trackedChanges,$(SRCDIR)/sw/qa/uitest,\
	trackedChanges/ \
))

$(eval $(call gb_UITest_set_defs,sw_trackedChanges, \
    TDOC="$(SRCDIR)/sw/qa/uitest/data" \
))

$(eval $(call gb_UITest_avoid_oneprocess,sw_trackedChanges))

# vim: set noet sw=4 ts=4:
