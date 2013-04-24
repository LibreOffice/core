# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,npapi,$(SRCDIR)/np_sdk))

$(eval $(call gb_Package_add_file,npapi,inc/external/npsdk/npapi.h,inc/npapi.h))
$(eval $(call gb_Package_add_file,npapi,inc/external/npsdk/npfunctions.h,inc/npfunctions.h))
$(eval $(call gb_Package_add_file,npapi,inc/external/npsdk/npruntime.h,inc/npruntime.h))
$(eval $(call gb_Package_add_file,npapi,inc/external/npsdk/nptypes.h,inc/nptypes.h))

# vim: set noet sw=4 ts=4:
