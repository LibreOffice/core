# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UI_UI,modules/swriter))

$(eval $(call gb_UI_add_uifiles,modules/swriter,\
	sw/uiconfig/swriter/ui/autoformattable \
	sw/uiconfig/swriter/ui/bibliographyentry \
	sw/uiconfig/swriter/ui/charurlpage \
	sw/uiconfig/swriter/ui/columnwidth \
	sw/uiconfig/swriter/ui/converttexttable \
	sw/uiconfig/swriter/ui/endnotepage \
	sw/uiconfig/swriter/ui/footnotepage \
	sw/uiconfig/swriter/ui/horizontalrule \
	sw/uiconfig/swriter/ui/indexentry \
	sw/uiconfig/swriter/ui/insertbreak \
	sw/uiconfig/swriter/ui/inserttable \
	sw/uiconfig/swriter/ui/linenumbering \
	sw/uiconfig/swriter/ui/printeroptions \
	sw/uiconfig/swriter/ui/sortdialog \
	sw/uiconfig/swriter/ui/splittable \
	sw/uiconfig/swriter/ui/stringinput \
	sw/uiconfig/swriter/ui/titlepage \
	sw/uiconfig/swriter/ui/wordcount \
))

# vim: set noet sw=4 ts=4:
