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
	extensions/uiconfig/spropctrlr/ui/browserline \
	extensions/uiconfig/spropctrlr/ui/browserpage \
	extensions/uiconfig/spropctrlr/ui/colorlistbox \
	extensions/uiconfig/spropctrlr/ui/combobox \
	extensions/uiconfig/spropctrlr/ui/controlfontdialog \
	extensions/uiconfig/spropctrlr/ui/datefield \
	extensions/uiconfig/spropctrlr/ui/datatypedialog \
	extensions/uiconfig/spropctrlr/ui/datetimefield \
	extensions/uiconfig/spropctrlr/ui/formlinksdialog \
	extensions/uiconfig/spropctrlr/ui/formattedcontrol \
	extensions/uiconfig/spropctrlr/ui/formattedsample \
	extensions/uiconfig/spropctrlr/ui/formproperties \
	extensions/uiconfig/spropctrlr/ui/hyperlinkfield \
	extensions/uiconfig/spropctrlr/ui/labelselectiondialog \
	extensions/uiconfig/spropctrlr/ui/listbox \
	extensions/uiconfig/spropctrlr/ui/listselectdialog \
	extensions/uiconfig/spropctrlr/ui/multiline \
	extensions/uiconfig/spropctrlr/ui/numericfield \
	extensions/uiconfig/spropctrlr/ui/taborder \
	extensions/uiconfig/spropctrlr/ui/textfield \
	extensions/uiconfig/spropctrlr/ui/timefield \
	extensions/uiconfig/spropctrlr/ui/urlcontrol \
))

# vim: set noet sw=4 ts=4:
