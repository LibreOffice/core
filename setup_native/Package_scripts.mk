# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,setup_native_scripts,$(SRCDIR)/setup_native/scripts))

ifeq ($(ENABLE_ONLINE_UPDATE),TRUE)
ifneq ($(filter LINUX SOLARIS,$(OS)),)
$(eval $(call gb_Package_add_file,setup_native_scripts,$(LIBO_BIN_FOLDER)/unpack_update,unpack_update.sh))
endif
endif

# vim: set noet sw=4 ts=4:
