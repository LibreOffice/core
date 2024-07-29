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

ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/ifbclient.dll,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/bin/ifbclient.dll))
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/Engine12.dll,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/plugins/Engine12.dll))
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/intl/fbintl.dll,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/intl/fbintl.dll))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/libfbclient.dylib.3.0.7,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/lib/libfbclient.dylib.3.0.7))
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/libEngine12.dylib,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/plugins/libEngine12.dylib))
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/intl/libfbintl.dylib,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/intl/libfbintl.dylib))
else ifeq ($(DISABLE_DYNLOADING),)
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/libfbclient.so.2,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/lib/libfbclient.so.3.0.7))
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/libEngine12.so,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/plugins/libEngine12.so))
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/intl/libfbintl.so,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/intl/libfbintl.so))
endif

ifeq ($(OS),MACOSX)
# only mach-o binaries allowed in bin folder (signing scripts would require extended attributes)
# so install it into Resources folder and use a symlink instead
# see https://developer.apple.com/library/archive/technotes/tn2206/_index.html
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_SHARE_FOLDER)/firebird/fbintl.conf,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/intl/fbintl.conf))
$(eval $(call gb_ExternalPackage_add_symbolic_link,firebird,$(LIBO_LIB_FOLDER)/intl/fbintl.conf,../../$(LIBO_SHARE_FOLDER)/firebird/fbintl.conf))
else
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_LIB_FOLDER)/intl/fbintl.conf,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/intl/fbintl.conf))
endif

$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_SHARE_FOLDER)/firebird/firebird.msg,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/firebird.msg))
$(eval $(call gb_ExternalPackage_add_file,firebird,$(LIBO_SHARE_FOLDER)/firebird/security3.fdb,gen/$(if $(ENABLE_DEBUG),Debug,Release)/firebird/security3.fdb))

# vim: set noet sw=4 ts=4:
