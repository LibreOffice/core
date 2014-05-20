# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,desktop))

$(eval $(call gb_UIConfig_add_uifiles,desktop,\
    desktop/uiconfig/ui/cmdlinehelp \
    desktop/uiconfig/ui/extensionmanager \
    desktop/uiconfig/ui/licensedialog \
    desktop/uiconfig/ui/showlicensedialog \
    desktop/uiconfig/ui/updatedialog \
))

# vim: set noet sw=4 ts=4:
