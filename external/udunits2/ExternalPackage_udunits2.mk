# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,udunits2,udunits2))

$(eval $(call gb_ExternalPackage_use_external_project,udunits2,udunits2))

ifeq ($(OS)-$(COM),WNT-MSC)
$(eval $(call gb_ExternalPackage_add_file,udunits2,$(LIBO_LIB_FOLDER)/iudunits2.dll,lib/.libs/iudunits2.dll))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,udunits2,$(LIBO_LIB_FOLDER)/libudunits2.dylib,lib/.libs/libudunits2.dylib))
else
$(eval $(call gb_ExternalPackage_add_file,udunits2,$(LIBO_LIB_FOLDER)/libudunits2.so,lib/.libs/libudunits2.so))
$(eval $(call gb_ExternalPackage_add_file,udunits2,$(LIBO_LIB_FOLDER)/libudunits2.so.0,lib/.libs/libudunits2.so.0))
$(eval $(call gb_ExternalPackage_add_file,udunits2,$(LIBO_LIB_FOLDER)/libudunits2.so.0.1.0,lib/.libs/libudunits2.so.0.1.0))
endif

$(eval $(call gb_ExternalPackage_add_file,udunits2,$(LIBO_SHARE_FOLDER)/udunits2/udunits2.xml,lib/udunits2.xml))
$(eval $(call gb_ExternalPackage_add_file,udunits2,$(LIBO_SHARE_FOLDER)/udunits2/udunits2-prefixes.xml,lib/udunits2-prefixes.xml))
$(eval $(call gb_ExternalPackage_add_file,udunits2,$(LIBO_SHARE_FOLDER)/udunits2/udunits2-base.xml,lib/udunits2-base.xml))
$(eval $(call gb_ExternalPackage_add_file,udunits2,$(LIBO_SHARE_FOLDER)/udunits2/udunits2-derived.xml,lib/udunits2-derived.xml))
$(eval $(call gb_ExternalPackage_add_file,udunits2,$(LIBO_SHARE_FOLDER)/udunits2/udunits2-accepted.xml,lib/udunits2-accepted.xml))
$(eval $(call gb_ExternalPackage_add_file,udunits2,$(LIBO_SHARE_FOLDER)/udunits2/udunits2-common.xml,lib/udunits2-common.xml))

# vim: set noet sw=4 ts=4:
