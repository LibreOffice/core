# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_libstdcpp,$(call gb_CustomTarget_get_workdir,extras/libstdcpp)))

$(eval $(call gb_ExternalPackage_add_file,extras_libstdcpp,$(LIBO_LIB_FOLDER)/libstdc++.so.6,libstdc++.so.6.0.28))

$(eval $(call gb_Package_use_custom_target,extras_libstdcpp,extras/libstdcpp))

# vim: set noet sw=4 ts=4:
