# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,jvmfwk_inc,$(SRCDIR)/jvmfwk/inc/jvmfwk))

$(eval $(call gb_Package_add_file,jvmfwk_inc,inc/jvmfwk/framework.h,framework.h))
$(eval $(call gb_Package_add_file,jvmfwk_inc,inc/jvmfwk/jvmfwkdllapi.h,jvmfwkdllapi.h))
$(eval $(call gb_Package_add_file,jvmfwk_inc,inc/jvmfwk/jvmfwkplugindllapi.h,jvmfwkplugindllapi.h))
$(eval $(call gb_Package_add_file,jvmfwk_inc,inc/jvmfwk/vendorplugin.h,vendorplugin.h))

# vim:set noet sw=4 ts=4:
