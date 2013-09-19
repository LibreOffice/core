# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,openssl,openssl))

$(eval $(call gb_ExternalPackage_use_external_project,openssl,openssl))

ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_files,openssl,lib,\
	out32dll/ssleay32.lib \
	out32dll/libeay32.lib \
))
$(eval $(call gb_ExternalPackage_add_libraries_for_install,openssl,bin,\
	out32dll/ssleay32.dll \
	out32dll/libeay32.dll \
))
endif
# vim: set noet sw=4 ts=4:
