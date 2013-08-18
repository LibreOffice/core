# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,sd_web,$(SRCDIR)/sd/res))

$(eval $(call gb_Package_set_outdir,sd_web,$(INSTDIR)))

$(eval $(call gb_Package_add_files,sd_web,$(LIBO_SHARE_FOLDER)/config/webcast,\
	webview/common.inc \
	webview/common.pl \
	webview/edit.asp \
	webview/editpic.asp \
	webview/editpic.pl \
	webview/edit.pl \
	webview/index.pl \
	webview/poll.asp \
	webview/poll.pl \
	webview/savepic.asp \
	webview/savepic.pl \
	webview/show.asp \
	webview/show.pl \
	webview/webcast.asp \
	webview/webcast.pl \
))

$(eval $(call gb_Package_add_files,sd_web,$(LIBO_SHARE_FOLDER)/config/wizard/web/buttons,\
	buttons/glas-blue.zip \
	buttons/glas-green.zip \
	buttons/glas-red.zip \
	buttons/round-gorilla.zip \
	buttons/round-white.zip \
	buttons/simple.zip \
	buttons/square-blue.zip \
	buttons/square-gray.zip \
	buttons/square-green.zip \
	buttons/square-red.zip \
	buttons/square-yellow.zip \
))

# vim: set noet sw=4 ts=4:
