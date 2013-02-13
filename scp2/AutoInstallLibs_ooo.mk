# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AutoInstallLibs_AutoInstallLibs,ooo,OOO,STD_LIB_FILE(gid_File_Lib_$$(1)_auto_installed,$$(1))))

# vim: set noet sw=4 ts=4:
