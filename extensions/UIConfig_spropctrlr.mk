# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,modules/spropctrlr))

$(eval $(call gb_UIConfig_add_uifiles,modules/spropctrlr,\
	extensions/uiconfig/spropctrlr/ui/controlfontdialog \
	extensions/uiconfig/spropctrlr/ui/datatypedialog \
	extensions/uiconfig/spropctrlr/ui/fieldlinkrow \
	extensions/uiconfig/spropctrlr/ui/formlinksdialog \
	extensions/uiconfig/spropctrlr/ui/labelselectiondialog \
	extensions/uiconfig/spropctrlr/ui/listselectdialog \
	extensions/uiconfig/spropctrlr/ui/taborder \
))

# vim: set noet sw=4 ts=4:
