# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,firebird,firebird))

$(eval $(call gb_ExternalPackage_use_external_project,firebird,firebird))

ifeq ($(OS)-$(COM),WNT-MSC)
$(eval $(call gb_ExternalPackage_add_file,firebird,bin/ifbembed.dll,gen/firebird/bin/ifbembed.dll))
$(eval $(call gb_ExternalPackage_add_file,firebird,lib/ifbembed.lib,gen/firebird/bin/ifbembed.lib))
else
ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,firebird,lib/libfbembed.2.5.2.dylib,gen/firebird/lib/libfbembed.dylib.2.5.2))
$(eval $(call gb_ExternalPackage_add_file,firebird,lib/libfbembed.2.5.dylib,gen/firebird/lib/libfbembed.dylib.2.5))
$(eval $(call gb_ExternalPackage_add_file,firebird,lib/libfbembed.dylib,gen/firebird/lib/libfbembed.dylib))
else
$(eval $(call gb_ExternalPackage_add_file,firebird,lib/libfbembed.so.2.5.2,gen/firebird/lib/libfbembed.so.2.5.2))
$(eval $(call gb_ExternalPackage_add_file,firebird,lib/libfbembed.so.2.5,gen/firebird/lib/libfbembed.so.2.5))
$(eval $(call gb_ExternalPackage_add_file,firebird,lib/libfbembed.so,gen/firebird/lib/libfbembed.so))
endif
endif

# vim: set noet sw=4 ts=4:
