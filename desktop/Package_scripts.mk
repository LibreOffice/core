# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,desktop_scripts_install,$(SRCDIR)/desktop/scripts))

ifeq (,$(filter MACOSX WNT,$(OS)))

$(eval $(call gb_Package_add_file,desktop_scripts_install,$(LIBO_BIN_FOLDER)/gdbtrace,gdbtrace))
$(eval $(call gb_Package_add_file,desktop_scripts_install,$(LIBO_BIN_FOLDER)/unopkg,unopkg.sh))

endif

ifneq ($(OS),WNT)

$(eval $(call gb_Package_add_file,desktop_scripts_install,$(LIBO_BIN_FOLDER)/unoinfo,$(if $(filter MACOSX,$(OS)),unoinfo-mac.sh,unoinfo.sh)))

endif

# vim: set ts=4 sw=4 noet:
