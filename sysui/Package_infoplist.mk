# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,infoplist,$(gb_CustomTarget_workdir)/sysui/infoplist))

# workaround to avoid gb_Package_add_files with empty directory adding extra '/'
$(eval $(call gb_Package_set_outdir,infoplist,$(INSTDIR)))

$(eval $(call gb_Package_add_files,infoplist,$(PRODUCTNAME_WITHOUT_SPACES).app/Contents,\
	PkgInfo \
	Info.plist \
))

$(foreach lang,$(filter ca cs da de el es fi fr hr hu id it ja ko ms nl no pl pt pt_PT ro ru sk sv th tr uk vi zh_CN zh_TW,$(gb_WITH_LANG)),\
$(eval $(call gb_Package_add_files,infoplist,$(PRODUCTNAME_WITHOUT_SPACES).app/Contents/Resources/$(lang).lproj,\
	InfoPlist_$(lang)/InfoPlist.strings \
)))

$(eval $(call gb_Package_add_empty_directory,infoplist,$(PRODUCTNAME_WITHOUT_SPACES).app/Contents/Resources/en.lproj))

# vim: set noet sw=4 ts=4:
