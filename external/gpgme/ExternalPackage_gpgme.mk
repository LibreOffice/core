# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,gpgme,gpgme))

$(eval $(call gb_ExternalPackage_use_external_project,gpgme,gpgme))

ifneq ($(DISABLE_DYNLOADING),TRUE)

$(eval $(call gb_ExternalPackage_add_file,gpgme,$(LIBO_LIB_FOLDER)/libgpgmepp.so.6,lang/cpp/src/.libs/libgpgmepp.so.6.4.0))
$(eval $(call gb_ExternalPackage_add_file,gpgme,$(LIBO_LIB_FOLDER)/libgpgme.so.11,src/.libs/libgpgme.so.11.18.0))

endif # $(DISABLE_DYNLOADING)

# vim: set noet sw=4 ts=4:
