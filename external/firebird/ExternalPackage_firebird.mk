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

# TODO Windows and Mac
ifeq ($(OS)-$(COM),WNT-MSC)
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/libfbclient.dll,gen/Release/firebird/bin/libfbclient.dll))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/libfbclient.dylib,gen/Release/firebird/lib/libfbclient.dylib.3.0.0))
else ifeq ($(DISABLE_DYNLOADING),)
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/libfbclient.so.2,gen/Release/firebird/lib/libfbclient.so.3.0.0))
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/libEngine12.so,gen/Release/firebird/plugins/libEngine12.so))
endif

$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_SHARE_FOLDER)/firebird/firebird.msg,gen/Release/firebird/firebird.msg))

# vim: set noet sw=4 ts=4:
