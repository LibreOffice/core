# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,test_updater_dialog))

$(eval $(call gb_Executable_use_unpacked,test_updater_dialog,onlineupdate))

$(eval $(call gb_Executable_set_include,test_updater_dialog,\
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/inc \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/update/common \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/update/updater/xpcom/glue \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/update/updater \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/include/onlineupdate \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/include \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/toolkit/mozapps/update/common \
	$$(INCLUDE) \
	$(if $(filter-out WNT,$(OS)),$$(GTK3_CFLAGS) ) \
))

$(eval $(call gb_Executable_use_static_libraries,test_updater_dialog,\
    updatehelper \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Executable_add_defs,test_updater_dialog, \
    -DXP_WIN \
))

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

else

$(eval $(call gb_Executable_add_defs,test_updater_dialog,\
	-DMOZ_VERIFY_MAR_SIGNATURE \
	-DNSS3 \
))

$(eval $(call gb_Executable_use_externals,test_updater_dialog,\
	nss3 \
))

$(eval $(call gb_Executable_add_libs,test_updater_dialog,\
	$(GTK3_LIBS) \
))
endif

$(eval $(call gb_Executable_add_exception_objects,test_updater_dialog,\
	external/onlineupdate/workben/test_dialog \
))

# external/onlineupdate/workben/test_dialog.cxx #include's lots of external .cpp code:
$(eval $(call gb_Executable_set_warnings_not_errors,test_updater_dialog))

ifeq ($(OS),WNT)
$(eval $(call gb_Executable_add_defs,test_updater_dialog,-DUNICODE))
endif

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
