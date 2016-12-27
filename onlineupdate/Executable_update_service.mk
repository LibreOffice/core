# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,update_service))

$(eval $(call gb_Executable_set_targettype_gui,update_service,YES))

$(eval $(call gb_Executable_set_include,update_service,\
	-I$(SRCDIR)/onlineupdate/source/libmar/src/ \
	-I$(SRCDIR)/onlineupdate/source/libmar/verify/ \
	-I$(SRCDIR)/onlineupdate/source/libmar/sign/ \
	-I$(SRCDIR)/onlineupdate/source/update/common/ \
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_use_static_libraries,update_service,\
	updatehelper \
	winhelper \
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

$(eval $(call gb_Executable_add_defs,update_service,\
	-DUNICODE \
))

$(eval $(call gb_Executable_add_ldflags,update_service,\
    /ENTRY:wmainCRTStartup \
))

$(eval $(call gb_Executable_add_cxxobjects,update_service,\
	onlineupdate/source/service/certificatecheck \
	onlineupdate/source/service/maintenanceservice \
	onlineupdate/source/service/registrycertificates \
	onlineupdate/source/service/servicebase \
	onlineupdate/source/service/serviceinstall \
	onlineupdate/source/service/workmonitor \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
