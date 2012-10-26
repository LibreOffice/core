# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UI_UI,cui))

$(eval $(call gb_UI_add_uifiles,cui,\
	cui/uiconfig/ui/charnamepage \
	cui/uiconfig/ui/effectspage \
	cui/uiconfig/ui/hyphenate \
	cui/uiconfig/ui/insertfloatingframe \
	cui/uiconfig/ui/insertoleobject \
	cui/uiconfig/ui/insertplugin \
	cui/uiconfig/ui/positionpage \
	cui/uiconfig/ui/specialcharacters \
	cui/uiconfig/ui/thesaurus \
	cui/uiconfig/ui/twolinespage \
	cui/uiconfig/ui/zoomdialog \
))

# vim: set noet sw=4 ts=4:
