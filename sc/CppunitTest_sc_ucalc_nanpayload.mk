# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call sc_ucalc_test,_nanpayload))

# some tests related to NaN payload may fail on some arch(e.g. riscv64)
# that does not support part of IEEE 754 voluntary standards
ifeq ($(DISABLE_NAN_TESTS),TRUE)
$(eval $(call gb_CppunitTest_add_defs,sc_ucalc_nanpayload,\
    -DDISABLE_NAN_TESTS \
))
endif

# vim: set noet sw=4 ts=4:
