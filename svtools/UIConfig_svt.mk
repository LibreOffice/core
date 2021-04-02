# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,svt))

$(eval $(call gb_UIConfig_add_uifiles,svt,\
	svtools/uiconfig/ui/addresstemplatedialog \
	svtools/uiconfig/ui/calendar \
	svtools/uiconfig/ui/checkboxcontrol \
	svtools/uiconfig/ui/combocontrol \
	svtools/uiconfig/ui/datewindow \
	svtools/uiconfig/ui/editcontrol \
	svtools/uiconfig/ui/emptypage \
	svtools/uiconfig/ui/fileviewmenu \
	svtools/uiconfig/ui/fixedimagecontrol \
	svtools/uiconfig/ui/fixedtextcontrol \
	svtools/uiconfig/ui/graphicexport \
	svtools/uiconfig/ui/interimparent \
	svtools/uiconfig/ui/inputbox \
	svtools/uiconfig/ui/javadisableddialog \
	svtools/uiconfig/ui/listcontrol \
	svtools/uiconfig/ui/linewindow \
	svtools/uiconfig/ui/managedtoolbar \
	svtools/uiconfig/ui/placeedit \
	svtools/uiconfig/ui/printersetupdialog \
	svtools/uiconfig/ui/querydeletedialog \
	svtools/uiconfig/ui/restartdialog \
	svtools/uiconfig/ui/spinfieldcontrol \
	svtools/uiconfig/ui/subtoolbar \
	svtools/uiconfig/ui/tabbuttons \
	svtools/uiconfig/ui/tabbuttonsmirrored \
	svtools/uiconfig/ui/tabbaredit \
	svtools/uiconfig/ui/textviewcontrol \
	svtools/uiconfig/ui/thineditcontrol \
))

# vim: set noet sw=4 ts=4:
