# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,uno_sh,$(SRCDIR)/cpputools/scripts))

ifneq (,$(filter-out MACOSX WNT,$(OS)))
$(eval $(call gb_Package_add_file,uno_sh,$(LIBO_URE_BIN_FOLDER)/uno,uno.sh))
endif

# vim:set noet sw=4 ts=4:
