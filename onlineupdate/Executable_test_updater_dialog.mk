# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,test_updater_dialog))

$(eval $(call gb_Executable_set_include,test_updater_dialog,\
	-I$(SRCDIR)/onlineupdate/inc \
	-I$(SRCDIR)/onlineupdate/source/update/common \
	-I$(SRCDIR)/onlineupdate/source/update/updater/xpcom/glue \
	-I$(SRCDIR)/onlineupdate/source/update/updater \
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_use_static_libraries,test_updater_dialog,\
    updatehelper \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Executable_add_libs,test_updater_dialog,\
	Ws2_32.lib \
	Gdi32.lib \
	Comctl32.lib \
	Shell32.lib \
	Shlwapi.lib \
	Crypt32.lib \
))

$(eval $(call gb_Executable_set_targettype_gui,test_updater_dialog,YES))

$(eval $(call gb_Executable_add_nativeres,test_updater_dialog,updaterres))

$(eval $(call gb_Executable_add_ldflags,test_updater_dialog,\
	/ENTRY:wmainCRTStartup \
))

$(eval $(call gb_Executable_add_defs,test_updater_dialog,\
	-DUNICODE \
))

else

$(eval $(call gb_Executable_add_defs,test_updater_dialog,\
	-DVERIFY_MAR_SIGNATURE \
	-DNSS3 \
))

$(eval $(call gb_Executable_use_externals,test_updater_dialog,\
	nss3 \
	gtk \
))

$(eval $(call gb_Executable_add_libs,test_updater_dialog,\
	-lX11 \
	-lXext \
	-lXrender \
	-lSM \
	-lICE \
))
endif

$(eval $(call gb_Executable_add_exception_objects,test_updater_dialog,\
 	onlineupdate/workben/test_dialog \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
