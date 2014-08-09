# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UIConfig_UIConfig,xmlsec))

$(eval $(call gb_UIConfig_add_uifiles,xmlsec,\
	xmlsecurity/uiconfig/ui/certpage \
	xmlsecurity/uiconfig/ui/certdetails \
	xmlsecurity/uiconfig/ui/certgeneral \
	xmlsecurity/uiconfig/ui/digitalsignaturesdialog \
	xmlsecurity/uiconfig/ui/securitylevelpage \
	xmlsecurity/uiconfig/ui/securitytrustpage \
	xmlsecurity/uiconfig/ui/selectcertificatedialog \
	xmlsecurity/uiconfig/ui/macrosecuritydialog \
	xmlsecurity/uiconfig/ui/viewcertdialog \
))

# vim: set noet sw=4 ts=4:
