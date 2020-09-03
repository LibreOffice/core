# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CompilerTest_CompilerTest,sal_rtl_oustring))

$(eval $(call gb_CompilerTest_add_exception_objects,sal_rtl_oustring, \
    sal/qa/rtl/strings/compile-oustring \
))

# vim: set noet sw=4 ts=4:
