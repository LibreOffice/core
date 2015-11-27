# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,instsetoo_native_setup,$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)))

$(eval $(call gb_Package_add_files,instsetoo_native_setup,$(LIBO_ETC_FOLDER),\
	$(call gb_Helper_get_rcfile,bootstrap) \
	$(call gb_Helper_get_rcfile,fundamental) \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),$(if $(filter-out MACOSX,$(OS)), \
	    $(call gb_Helper_get_rcfile,redirect))) \
	$(call gb_Helper_get_rcfile,setup) \
	$(call gb_Helper_get_rcfile,soffice) \
	$(call gb_Helper_get_rcfile,louno) \
	$(call gb_Helper_get_rcfile,version) \
))

# vim: set noet sw=4 ts=4:
