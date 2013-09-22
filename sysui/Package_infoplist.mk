# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,infoplist,$(call gb_CustomTarget_get_workdir,sysui/infoplist)))

$(eval $(call gb_Package_set_outdir,infoplist,$(INSTROOT)))

$(eval $(call gb_Package_add_files,infoplist,,\
	PkgInfo \
	Info.plist \
))

$(eval $(call gb_Package_add_files,infoplist,bin,\
	$(foreach lang,en-US $(gb_WITH_LANG),InfoPlist_$(lang).zip) \
))

# vim: set noet sw=4 ts=4:
