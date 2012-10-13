# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

.PHONY: compilerplugins compilerplugins-clean

ifeq ($(COMPILER_PLUGINS),)

# no support

compilerplugins:
compilerplugins-clean:
compilerplugins.clean:

else

ifeq ($(COM_GCC_IS_CLANG),TRUE)

include $(SRCDIR)/compilerplugins/Makefile-clang.mk

compilerplugins.clean: compilerplugins-clean

endif

endif

# vim: set noet sw=4 ts=4:
