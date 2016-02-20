# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,breakpad,breakpad))

$(eval $(call gb_ExternalPackage_use_external_project,breakpad,breakpad))

ifeq ($(OS),LINUX)
$(eval $(call gb_ExternalPackage_add_file,breakpad,$(LIBO_LIB_FOLDER)/minidump_upload,src/tools/linux/symupload/minidump_upload))
else ifeq ($(OS)),WNT)
$(eval $(call gb_ExternalPackage_add_file,breakpad,$(LIBO_LIB_FOLDER)/minidump_upload,src/tools/linux/symupload/minidump_upload))
endif

# vim: set noet sw=4 ts=4:
