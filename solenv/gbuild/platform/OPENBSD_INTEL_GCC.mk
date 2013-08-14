# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

#please make generic modifications to unxgcc.mk

gb_CPUDEFS := -DX86
gb_COMPILERDEFAULTOPTFLAGS := -O2
gb_STDLIBS := $(PTHREAD_LIBS)

include $(GBUILDDIR)/platform/unxgcc.mk

# vim: set noet sw=4:
