# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,filter))

$(eval $(call gb_UIConfig_add_uifiles,filter,\
    filter/uiconfig/ui/pdfgeneralpage \
    filter/uiconfig/ui/pdflinkspage \
    filter/uiconfig/ui/pdfoptionsdialog \
    filter/uiconfig/ui/pdfsecuritypage \
    filter/uiconfig/ui/pdfsignpage \
    filter/uiconfig/ui/pdfuserinterfacepage \
    filter/uiconfig/ui/pdfviewpage \
	filter/uiconfig/ui/testxmlfilter \
	filter/uiconfig/ui/xmlfiltersettings \
	filter/uiconfig/ui/xmlfiltertabpagegeneral \
))

# vim: set noet sw=4 ts=4:
