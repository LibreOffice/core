# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,np_sdk_inc,$(SRCDIR)/np_sdk))

$(eval $(call gb_Package_add_file,np_sdk_inc,inc/npsdk/jri.h,mozsrc/jri.h))
$(eval $(call gb_Package_add_file,np_sdk_inc,inc/npsdk/jri_md.h,mozsrc/jri_md.h))
$(eval $(call gb_Package_add_file,np_sdk_inc,inc/npsdk/jritypes.h,mozsrc/jritypes.h))
$(eval $(call gb_Package_add_file,np_sdk_inc,inc/npsdk/npupp.h,mozsrc/npupp.h))
$(eval $(call gb_Package_add_file,np_sdk_inc,inc/npsdk/plugin.h,mozsrc/plugin.h))

# vim: set noet sw=4 ts=4:
