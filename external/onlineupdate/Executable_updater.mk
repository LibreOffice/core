# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,updater))

$(eval $(call gb_Executable_use_unpacked,updater,onlineupdate))

$(eval $(call gb_Executable_set_include,updater,\
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/inc \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/update/common \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/update/updater/xpcom/glue \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/service \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/include/onlineupdate \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/include \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/modules/xz-embedded/src \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/toolkit/mozapps/update/common \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/toolkit/mozapps/update/updater \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/toolkit/xre \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/xpcom/base \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/xpcom/string \
	$$(INCLUDE) \
	$(if $(filter-out WNT,$(OS)),$$(GTK3_CFLAGS) ) \
))

$(eval $(call gb_Executable_use_custom_headers,updater,external/onlineupdate/generated))

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
	Ole32.lib \
	Rpcrt4.lib \
	wintrust.lib \
))

$(eval $(call gb_Executable_set_targettype_gui,updater,YES))

$(eval $(call gb_Executable_add_nativeres,updater,updaterres))

$(eval $(call gb_Executable_add_ldflags,updater,\
	/ENTRY:wmainCRTStartup \
))

$(eval $(call gb_Executable_add_defs,updater,\
	-DMOZ_APP_VERSION=\"$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)\" \
	-DMOZ_VERIFY_MAR_SIGNATURE \
	-DNS_NO_XPCOM \
	-DMOZ_MAINTENANCE_SERVICE \
	-DXP_WIN \
	-DXZ_USE_CRC64 \
	-DXZ_DEC_X86 \
	-DXZ_DEC_POWERPC \
	-DXZ_DEC_IA64 \
	-DXZ_DEC_ARM \
	-DXZ_DEC_ARMTHUMB \
	-DXZ_DEC_SPARC \
	$(if $(filter AARCH64 INTEL X86_64,$(CPUNAME)),-D__BYTE_ORDER__=1234) \
	-D__ORDER_BIG_ENDIAN__=4321 \
	-D__ORDER_LITTLE_ENDIAN__=1234 \
	-U_WIN32_WINNT \
))

else

$(eval $(call gb_Executable_add_defs,updater,\
	-DMOZ_APP_VERSION=\"$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)\" \
	-DMOZ_VERIFY_MAR_SIGNATURE \
	-DNSS3 \
	-DNS_NO_XPCOM \
	-DXP_UNIX \
	-DXZ_USE_CRC64 \
	-DXZ_DEC_X86 \
	-DXZ_DEC_POWERPC \
	-DXZ_DEC_IA64 \
	-DXZ_DEC_ARM \
	-DXZ_DEC_ARMTHUMB \
	-DXZ_DEC_SPARC \
))

$(eval $(call gb_Executable_use_externals,updater,\
	nss3 \
))

$(eval $(call gb_Executable_add_libs,updater,\
	$(GTK3_LIBS) \
))

endif

$(eval $(call gb_Executable_set_generated_cxx_suffix,updater,cpp))

$(eval $(call gb_Executable_add_generated_exception_objects,updater,\
	UnpackedTarball/onlineupdate/onlineupdate/source/update/updater/archivereader \
	UnpackedTarball/onlineupdate/onlineupdate/source/update/updater/bspatch \
	$(if $(filter-out WNT,$(OS)),UnpackedTarball/onlineupdate/onlineupdate/source/update/updater/progressui_gtk) \
	UnpackedTarball/onlineupdate/onlineupdate/source/update/updater/updater \
	$(if $(filter WNT,$(OS)),\
		UnpackedTarball/onlineupdate/onlineupdate/source/update/updater/loaddlls \
		UnpackedTarball/onlineupdate/onlineupdate/source/update/updater/progressui_win )\
))

$(eval $(call gb_Executable_add_generated_cobjects,updater, \
    UnpackedTarball/onlineupdate/modules/xz-embedded/src/xz_crc32 \
    UnpackedTarball/onlineupdate/modules/xz-embedded/src/xz_crc64 \
    UnpackedTarball/onlineupdate/modules/xz-embedded/src/xz_dec_bcj \
    UnpackedTarball/onlineupdate/modules/xz-embedded/src/xz_dec_lzma2 \
    UnpackedTarball/onlineupdate/modules/xz-embedded/src/xz_dec_stream \
))

$(eval $(call gb_Executable_set_warnings_disabled,updater))

ifeq ($(OS),WNT)

$(eval $(call gb_Executable_add_cxxflags,updater,-Zc:strictStrings-))

$(eval $(call gb_Executable_add_defs,updater,-DUNICODE))

endif

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
