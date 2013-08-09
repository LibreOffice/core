# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

gb_COMPILERDEFAULTOPTFLAGS := -Os
gb_CPUDEFS := -D_M_IX86
gb_TMPDIR:=$(if $(TMPDIR),$(TMPDIR),/tmp)

# please make generic modifications to either of these
include $(GBUILDDIR)/platform/com_GCC_defs.mk
include $(GBUILDDIR)/platform/windows.mk

ifeq ($(COM_GCC_IS_CLANG),)
# This has to do something with calling conventions, which are different
# for x86 and x64. Don't put it in the common part since it is breaking
# and conde that uses boost::bind
gb_GccLess470 := $(shell expr $(GCC_VERSION) \< 407)

# Until GCC 4.6, MinGW used __cdecl by default, and BOOST_MEM_FN_ENABLE_CDECL
# would result in ambiguous calls to overloaded boost::bind; since GCC 4.7,
# MinGW uses __thiscall by default, so now needs BOOST_MEM_FN_ENABLE_CDECL for
# uses of boost::bind with functions annotated with SAL_CALL:
ifeq ($(gb_GccLess470),0)
gb_COMPILERDEFS += -DBOOST_MEM_FN_ENABLE_CDECL
endif
endif

include $(GBUILDDIR)/platform/mingw.mk

include $(GBUILDDIR)/platform/com_GCC_class.mk

# vim: set noet sw=4:
