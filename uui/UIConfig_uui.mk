# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,uui))

$(eval $(call gb_UIConfig_add_uifiles,uui,\
	uui/uiconfig/ui/filterselect \
	uui/uiconfig/ui/authfallback \
	uui/uiconfig/ui/logindialog \
	uui/uiconfig/ui/macrowarnmedium \
	uui/uiconfig/ui/masterpassworddlg \
	uui/uiconfig/ui/password \
	uui/uiconfig/ui/setmasterpassworddlg \
	uui/uiconfig/ui/simplenameclash \
	uui/uiconfig/ui/sslwarndialog \
	uui/uiconfig/ui/unknownauthdialog \
))

# vim: set noet sw=4 ts=4:
