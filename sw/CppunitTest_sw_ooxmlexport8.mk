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

ifeq ($(OS),MACOSX)

$(eval $(call gb_CppunitTest_add_cxxflags,sw_ooxmlexport8,\
    $(gb_OBJCXXFLAGS) \
))

$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,sw_ooxmlexport8,\
	AppKit \
))

endif


include $(SRCDIR)/sw/ooxmlexport_setup.mk

$(eval $(call sw_ooxmlexport_test,8))

# vim: set noet sw=4 ts=4:
