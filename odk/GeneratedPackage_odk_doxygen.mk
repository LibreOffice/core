# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_GeneratedPackage_GeneratedPackage,odk_doxygen,$(call gb_CustomTarget_get_workdir,odk/docs)))

$(eval $(call gb_GeneratedPackage_use_customtarget,odk_doxygen,odk/docs))

$(eval $(call gb_GeneratedPackage_add_dir,odk_doxygen,$(INSTDIR)/$(gb_Package_SDKDIRNAME)/docs/cpp/ref,cpp/ref))
$(eval $(call gb_GeneratedPackage_add_dir,odk_doxygen,$(INSTDIR)/$(gb_Package_SDKDIRNAME)/docs/idl/ref,idl/ref))

# vim: set noet sw=4 ts=4:
