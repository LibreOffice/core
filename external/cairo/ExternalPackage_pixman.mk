# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,pixman,pixman))

$(eval $(call gb_ExternalPackage_use_external_project,pixman,pixman))

ifneq ($(OS),ANDROID)

$(eval $(call gb_ExternalPackage_add_file,pixman,$(LIBO_LIB_FOLDER)/libpixman-1.so.0,pixman/.libs/libpixman-1.so.0.24.4))

endif


# vim: set noet sw=4 ts=4:
