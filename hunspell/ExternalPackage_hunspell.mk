# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,hunspell,hunspell))

ifneq ($(COM),MSC)

$(eval $(call gb_ExternalPackage_use_external_project,hunspell,hunspell))

$(eval $(call gb_ExternalPackage_add_file,hunspell,lib/libhunspell.a,src/hunspell/.libs/libhunspell-1.3.a))

endif

# vim: set noet sw=4 ts=4:
