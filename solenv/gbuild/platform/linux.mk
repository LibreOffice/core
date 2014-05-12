# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifneq ($(COM_GCC_IS_CLANG)$(filter -fsanitize=address,$(CC)),TRUE-fsanitize=address)
gb_LinkTarget_LDFLAGS += \
        -Wl,-z,defs \

endif

include $(GBUILDDIR)/platform/unxgcc.mk

# vim: set noet sw=4 ts=4:
