# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,infoplist,$(call gb_CustomTarget_get_workdir,sysui/infoplist)))

# workaround to avoid gb_Package_add_files with empty directory adding extra '/'
$(eval $(call gb_Package_set_outdir,infoplist,$(INSTDIR)))

$(eval $(call gb_Package_add_files,infoplist,$(PRODUCTNAME_WITHOUT_SPACES).app/Contents,\
	PkgInfo \
	Info.plist \
))

# note: en-US was substituted with en in ITERATE_ALL_LANG_DIR_LPROJ
$(foreach lang,$(filter ca cs da de el en es fi fr hr hu id it ja ko ms nl no pl pt pt_PT ro ru sk sv th tr uk vi zh_CN zh_TW,$(subst en-US,en,$(gb_WITH_LANG))),\
$(eval $(call gb_Package_add_files,infoplist,$(PRODUCTNAME_WITHOUT_SPACES).app/Contents/Resources/$(lang).lproj,\
	InfoPlist_$(lang)/InfoPlist.strings \
)))

# note: en-US was substituted with en in ITERATE_ALL_LANG_DIR_LPROJ
# assumption is that the $(lang).lproj dir must exist even if empty;
# if it's non-empty the above add_files creates it, if empty we need this:
$(eval $(call gb_Package_add_empty_directories,infoplist,\
	$(foreach lang,$(filter-out ca cs da de el en es fi fr hr hu id it ja ko ms nl no pl pt pt_PT ro ru sk sv th tr uk vi zh_CN zh_TW,$(subst en-US,en,$(gb_WITH_LANG))),\
		$(PRODUCTNAME_WITHOUT_SPACES).app/Contents/Resources/$(lang).lproj \
	) \
))

# vim: set noet sw=4 ts=4:
