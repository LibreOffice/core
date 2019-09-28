# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,updater))

$(eval $(call gb_Executable_set_include,updater,\
	-I$(SRCDIR)/onlineupdate/inc \
	-I$(SRCDIR)/onlineupdate/source/update/common \
	-I$(SRCDIR)/onlineupdate/source/update/updater/xpcom/glue \
	$$(INCLUDE) \
	$(if $(filter-out WNT,$(OS)),$$(GTK3_CFLAGS) ) \
))

$(eval $(call gb_Executable_use_custom_headers,updater,onlineupdate/generated))

$(eval $(call gb_Executable_use_static_libraries,updater,\
	libmar \
    libmarverify \
    updatehelper \
	$(if $(filter WNT,$(OS)), \
		windows_process )\
))

$(eval $(call gb_Executable_use_externals,updater,\
	bzip2 \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Executable_add_libs,updater,\
	Ws2_32.lib \
	Gdi32.lib \
	Comctl32.lib \
	Shell32.lib \
	Shlwapi.lib \
	Crypt32.lib \
))

$(eval $(call gb_Executable_set_targettype_gui,updater,YES))

$(eval $(call gb_Executable_add_nativeres,updater,updaterres))

$(eval $(call gb_Executable_add_ldflags,updater,\
	/ENTRY:wmainCRTStartup \
))

$(eval $(call gb_Executable_add_defs,updater,\
	-DVERIFY_MAR_SIGNATURE \
))

else

$(eval $(call gb_Executable_add_defs,updater,\
	-DVERIFY_MAR_SIGNATURE \
	-DNSS3 \
))

$(eval $(call gb_Executable_use_externals,updater,\
	nss3 \
))

$(eval $(call gb_Executable_add_libs,updater,\
	$(GTK3_LIBS) \
))

endif

$(eval $(call gb_Executable_add_exception_objects,updater,\
	onlineupdate/source/update/updater/xpcom/glue/nsVersionComparator \
	onlineupdate/source/update/updater/archivereader \
	onlineupdate/source/update/updater/bspatch \
	onlineupdate/source/update/updater/progressui_gtk \
	onlineupdate/source/update/updater/updater \
	$(if $(filter WNT,$(OS)),\
		onlineupdate/source/update/updater/loaddlls \
		onlineupdate/source/update/updater/progressui_win \
		onlineupdate/source/update/updater/win_dirent )\
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
