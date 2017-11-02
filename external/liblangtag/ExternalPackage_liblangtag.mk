# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,liblangtag,liblangtag))

$(eval $(call gb_ExternalPackage_use_external_project,liblangtag,liblangtag))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,liblangtag,$(LIBO_LIB_FOLDER)/liblangtag.1.dylib,liblangtag/.libs/liblangtag.1.dylib))
else ifeq ($(ANDROID_PORTS),1)
$(eval $(call gb_ExternalPackage_add_file,liblangtag,$(LIBO_LIB_FOLDER)/liblangtag.so,liblangtag/.libs/liblangtag-lo.so))
else ifeq ($(DISABLE_DYNLOADING),)
$(eval $(call gb_ExternalPackage_add_file,liblangtag,$(LIBO_LIB_FOLDER)/liblangtag-lo.so.1,liblangtag/.libs/liblangtag-lo.so.1.4.1))
endif

# vim: set noet sw=4 ts=4:
