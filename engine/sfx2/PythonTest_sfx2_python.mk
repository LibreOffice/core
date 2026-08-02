# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_PythonTest_PythonTest,sfx2_python))

$(eval $(call gb_PythonTest_set_defs,sfx2_python,\
    TDOC="$(SRCDIR)/sfx2/qa/python/testdocuments" \
))

# The sidebar's UNO API only creates the controller when the Kit is inactive, a Kit build
# expecting the client to ask for the sidebar, which check_sidebar has no way of doing
$(eval $(call gb_PythonTest_add_modules,sfx2_python,$(SRCDIR)/sfx2/qa/python,\
	$(if $(ENABLE_KIT_ALWAYS_ACTIVE),,check_sidebar) \
	check_sidebar_registry \
))

# vim: set noet sw=4 ts=4:
