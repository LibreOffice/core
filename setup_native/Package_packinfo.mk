# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,setup_native_packinfo,$(SRCDIR)))

# Check if any bundled extensions are installed (where ENABLE_JAVA covers Extension_nlpsolver, cf.
# nlpsolver/Module_nlpsolver.mk); if none are installed, then gb_Package_add_empty_directory is
# needed to create the empty extensions directory (but if any are installed, then
# gb_Package_add_empty_directory would cause errors during CustomTarget_instsetoo_native/install, as
# the directory is not empty in that case):
ifeq ($(filter CT2N DICTIONARIES LANGUAGETOOL,$(BUILD_TYPE))$(ENABLE_MEDIAWIKI)$(NUMBERTEXT_EXTENSION_PACK)$(ENABLE_JAVA),)
$(eval $(call gb_Package_add_empty_directory,setup_native_packinfo,$(LIBO_SHARE_FOLDER)/extensions))
endif

# vim: set noet sw=4 ts=4:
