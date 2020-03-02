# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UITest_UITest,writer_demo3))

$(eval $(call gb_UITest_add_modules,writer_demo3,$(SRCDIR)/uitest,\
	writer_tests3/ \
))

$(eval $(call gb_UITest_set_defs,writer_demo3, \
    TDOC="$(SRCDIR)/uitest/writer_tests/data" \
))

# vim: set noet sw=4 ts=4:
