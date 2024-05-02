# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,readlicense_oo_license,$(gb_CustomTarget_workdir)/readlicense_oo/license))

ifeq ($(OS),WNT)
$(eval $(call gb_Package_add_file,readlicense_oo_license,license.txt,license.txt))
else
ifneq ($(OS),MACOSX)
$(eval $(call gb_Package_add_file,readlicense_oo_license,LICENSE,LICENSE))
else
$(eval $(call gb_Package_add_file,readlicense_oo_license,Resources/LICENSE,LICENSE))
endif
endif

ifneq ($(OS),MACOSX)
$(eval $(call gb_Package_add_file,readlicense_oo_license,LICENSE.html,LICENSE.html))
else
$(eval $(call gb_Package_add_file,readlicense_oo_license,Resources/LICENSE.html,LICENSE.html))
endif

# vim: set noet sw=4 ts=4:
