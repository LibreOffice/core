# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

#please make generic modifications to unxgcc.mk

gb_COMPILERDEFAULTOPTFLAGS := -O -g

include $(GBUILDDIR)/platform/unxgcc.mk

NB_ADD_LIBPATH := /usr/pkg/lib:/usr/X11R7/lib

gb_CppunitTest_CPPTESTPRECOMMAND := LD_LIBRARY_PATH=$${LD_LIBRARY_PATH:+$$LD_LIBRARY_PATH:}$(WORKDIR)/UnpackedTarball/cppunit/src/cppunit/.libs:$(NB_ADD_LIBPATH)

# vim: set noet sw=4:
