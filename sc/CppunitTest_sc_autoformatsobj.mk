# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call sc_unoapi_common,autoformatsobj))

.PHONY: restore-default-autoformats
restore-default-autoformats:
	-cp "sc/res/xml/tablestyles.xml" "$(INSTROOT)/$(LIBO_SHARE_FOLDER)/calc/tablestyles.xml"

$(call gb_CppunitTest_get_target,sc_autoformatsobj): restore-default-autoformats

# vim: set noet sw=4 ts=4:
