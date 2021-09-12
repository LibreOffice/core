# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

gb__LinkTarget_LDFLAGS_zdefs := -Wl,-z,defs
ifeq ($(COM_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(CC) $(LDFLAGS)),)
gb__LinkTarget_LDFLAGS_zdefs :=
endif
endif
gb_LinkTarget_LDFLAGS += $(gb__LinkTarget_LDFLAGS_zdefs)

include $(GBUILDDIR)/platform/unxgcc.mk

ifneq ($(ATOMIC_LIB),)
gb_STDLIBS_CXX += $(ATOMIC_LIB)
endif

ifeq ($(DISABLE_DYNLOADING),TRUE)
gb_STDLIBS_CXX += -lstdc++
endif

# vim: set noet sw=4 ts=4:
