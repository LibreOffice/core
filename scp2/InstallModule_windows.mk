# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_InstallModule_InstallModule,scp2/windows))

$(eval $(call gb_InstallModule_define_if_set,scp2/windows,\
	BUILD_X64 \
))

$(eval $(call gb_InstallModule_add_defs,scp2/windows,\
	$(if $(WINDOWS_SDK_HOME),\
		-DHAVE_WINDOWS_SDK \
	) \
))

$(eval $(call gb_InstallModule_add_scpfiles,scp2/windows,\
    scp2/source/ooo/folder_ooo \
    scp2/source/ooo/vc_redist \
    scp2/source/ooo/windowscustomaction_ooo \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/windows,\
    scp2/source/base/folderitem_base \
    scp2/source/base/registryitem_base \
    scp2/source/calc/folderitem_calc \
    scp2/source/calc/registryitem_calc \
    scp2/source/draw/folderitem_draw \
    scp2/source/draw/registryitem_draw \
    scp2/source/impress/folderitem_impress \
    scp2/source/impress/registryitem_impress \
    scp2/source/math/folderitem_math \
    scp2/source/math/registryitem_math \
    scp2/source/ooo/folderitem_ooo \
    scp2/source/ooo/registryitem_ooo \
    scp2/source/writer/folderitem_writer \
    scp2/source/writer/registryitem_writer \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
