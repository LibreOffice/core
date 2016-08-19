# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,update_service))

$(eval $(call gb_Executable_set_include,update_service,\
	-I$(SRCDIR)/onlineupdate/source/libmar/src/ \
	-I$(SRCDIR)/onlineupdate/source/libmar/verify/ \
	-I$(SRCDIR)/onlineupdate/source/libmar/sign/ \
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_add_libs,update_service,\
    ws2_32.lib \
    Crypt32.lib \
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
