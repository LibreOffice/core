# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UITest_UITest,solver))

$(eval $(call gb_UITest_add_modules,solver,$(SRCDIR)/sc/qa/uitest,\
	solver/ \
))

# Disable bundled extensions with a hack, so that the alternative solver from --enable-ext-nlpsolver
# does not kick in and cause the test's expectations to not be met:
$(eval $(call gb_UITest_set_defs,solver, \
    TDOC="$(SRCDIR)/sc/qa/uitest/data" \
    BUNDLED_EXTENSIONS='$$$$BRAND_BASE_DIR/NONE' \
))

# vim: set noet sw=4 ts=4:
