# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,mbsdiff))

$(eval $(call gb_Executable_use_unpacked,mbsdiff,onlineupdate))

$(eval $(call gb_Executable_set_include,mbsdiff,\
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/inc \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/toolkit/mozapps/update/updater \
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_use_externals,mbsdiff,\
	bzip2 \
))


ifeq ($(OS),WNT)

$(eval $(call gb_Executable_add_defs,mbsdiff, \
    -DXP_WIN \
))

$(eval $(call gb_Executable_add_libs,mbsdiff,\
    ws2_32.lib \
))

endif

$(eval $(call gb_Executable_add_generated_cobjects,mbsdiff,\
	UnpackedTarball/onlineupdate/onlineupdate/source/mbsdiff/bsdiff \
))

$(eval $(call gb_Executable_set_warnings_disabled,mbsdiff))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
