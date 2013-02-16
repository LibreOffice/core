# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,scripts_generated,$(call gb_CustomTarget_get_workdir,setup_native/scripts)))

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Package_add_file,scripts_generated,bin/userscripts/install,install))
$(eval $(call gb_Package_add_file,scripts_generated,bin/userscripts/uninstall,uninstall))
endif

ifeq ($(OS),LINUX)
ifneq ($(filter rpm,$(PKGFORMAT)),)
$(eval $(call gb_Package_add_file,scripts_generated,bin/userscripts/install,install))
$(eval $(call gb_Package_add_file,scripts_generated,bin/userscripts/uninstall,uninstall))
endif
endif
# vim: set noet sw=4 ts=4:
