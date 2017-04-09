# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExtensionPackageSet_ExtensionPackageSet,misc_extensions))

ifneq ($(NUMBERTEXT_EXTENSION_PACK),)
$(eval $(call gb_ExtensionPackageSet_add_extension,misc_extensions,numbertext,$(NUMBERTEXT_EXTENSION_PACK)))
endif

# vim: set noet sw=4 ts=4:
