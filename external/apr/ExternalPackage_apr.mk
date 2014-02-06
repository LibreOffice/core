# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,apr,apr))

$(eval $(call gb_ExternalPackage_use_external_project,apr,apr))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,apr,$(LIBO_LIB_FOLDER)/libapr-1.dylib,.libs/libapr-1.0.5.0.dylib))
else ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_file,apr,$(LIBO_LIB_FOLDER)/libapr-1.dll,Release/libapr-1.dll))
else
$(eval $(call gb_ExternalPackage_add_file,apr,$(LIBO_LIB_FOLDER)/libapr-1.so,.libs/libapr-1.so.0.5.0))
endif

# vim: set noet sw=4 ts=4:
