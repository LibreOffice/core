# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,xmlsec,xmlsec))

$(eval $(call gb_ExternalPackage_use_external_project,xmlsec,xmlsec))

ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_library_for_install,xmlsec,lib/libxmlsec1.dll,src/.libs/libxmlsec1.dll))
$(eval $(call gb_ExternalPackage_add_library_for_install,xmlsec,lib/libxmlsec1-nss.dll,src/nss/.libs/libxmlsec1-nss.dll))
else
$(eval $(call gb_ExternalPackage_add_library_for_install,xmlsec,bin/libxmlsec-mscrypto.dll,win32/binaries/libxmlsec-mscrypto.dll))
$(eval $(call gb_ExternalPackage_add_library_for_install,xmlsec,bin/libxmlsec.dll,win32/binaries/libxmlsec.dll))
endif
endif

# vim: set noet sw=4 ts=4:
