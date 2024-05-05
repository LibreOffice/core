# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,update_service))

$(eval $(call gb_Executable_use_unpacked,update_service,onlineupdate))

$(eval $(call gb_Executable_set_targettype_gui,update_service,YES))

$(eval $(call gb_Executable_set_include,update_service,\
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/inc \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/libmar/src/ \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/libmar/verify/ \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/libmar/sign/ \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/update/common/ \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/include/onlineupdate \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/include \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/toolkit/mozapps/update/common \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/toolkit/xre \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/xpcom/base \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/xpcom/string \
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_use_static_libraries,update_service,\
	updatehelper \
	windows_process \
))

$(eval $(call gb_Executable_add_libs,update_service,\
    ws2_32.lib \
    Crypt32.lib \
    shell32.lib \
    wintrust.lib \
    version.lib \
    wtsapi32.lib \
    userenv.lib \
    shlwapi.lib \
    ole32.lib \
    rpcrt4.lib \
    comctl32.lib \
    shlwapi.lib \
    kernel32.lib \
    advapi32.lib \
))

$(eval $(call gb_Executable_add_ldflags,update_service,\
    /ENTRY:wmainCRTStartup \
))

$(eval $(call gb_Executable_set_generated_cxx_suffix,update_service,cpp))

$(eval $(call gb_Executable_add_generated_exception_objects,update_service,\
	UnpackedTarball/onlineupdate/onlineupdate/source/service/maintenanceservice \
	UnpackedTarball/onlineupdate/onlineupdate/source/service/servicebase \
	UnpackedTarball/onlineupdate/onlineupdate/source/service/serviceinstall \
	UnpackedTarball/onlineupdate/onlineupdate/source/service/workmonitor \
))

$(eval $(call gb_Executable_add_cxxflags,update_service,-Zc:strictStrings-))

$(eval $(call gb_Executable_add_defs,update_service, \
    -DMOZ_MAINTENANCE_SERVICE \
    -DNS_NO_XPCOM \
    -DUNICODE \
    -DXP_WIN \
    $(if $(filter AARCH64 INTEL X86_64,$(CPUNAME)),-D__BYTE_ORDER__=1234) \
    -D__ORDER_BIG_ENDIAN__=4321 \
    -D__ORDER_LITTLE_ENDIAN__=1234 \
    -U_WIN32_WINNT \
))

$(eval $(call gb_Executable_set_warnings_disabled,update_service))

$(eval $(call gb_Executable_add_default_nativeres,update_service))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
