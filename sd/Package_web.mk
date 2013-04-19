# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,sd_web,$(SRCDIR)/sd/res))

$(eval $(call gb_Package_add_file,sd_web,pck/glas-blue.zip,buttons/glas-blue.zip))
$(eval $(call gb_Package_add_file,sd_web,pck/glas-green.zip,buttons/glas-green.zip))
$(eval $(call gb_Package_add_file,sd_web,pck/glas-red.zip,buttons/glas-red.zip))
$(eval $(call gb_Package_add_file,sd_web,pck/round-gorilla.zip,buttons/round-gorilla.zip))
$(eval $(call gb_Package_add_file,sd_web,pck/round-white.zip,buttons/round-white.zip))
$(eval $(call gb_Package_add_file,sd_web,pck/simple.zip,buttons/simple.zip))
$(eval $(call gb_Package_add_file,sd_web,pck/square-blue.zip,buttons/square-blue.zip))
$(eval $(call gb_Package_add_file,sd_web,pck/square-gray.zip,buttons/square-gray.zip))
$(eval $(call gb_Package_add_file,sd_web,pck/square-green.zip,buttons/square-green.zip))
$(eval $(call gb_Package_add_file,sd_web,pck/square-red.zip,buttons/square-red.zip))
$(eval $(call gb_Package_add_file,sd_web,pck/square-yellow.zip,buttons/square-yellow.zip))
$(eval $(call gb_Package_add_file,sd_web,pck/common.inc,webview/common.inc))
$(eval $(call gb_Package_add_file,sd_web,pck/common.pl,webview/common.pl))
$(eval $(call gb_Package_add_file,sd_web,pck/edit.asp,webview/edit.asp))
$(eval $(call gb_Package_add_file,sd_web,pck/editpic.asp,webview/editpic.asp))
$(eval $(call gb_Package_add_file,sd_web,pck/editpic.pl,webview/editpic.pl))
$(eval $(call gb_Package_add_file,sd_web,pck/edit.pl,webview/edit.pl))
$(eval $(call gb_Package_add_file,sd_web,pck/index.pl,webview/index.pl))
$(eval $(call gb_Package_add_file,sd_web,pck/poll.asp,webview/poll.asp))
$(eval $(call gb_Package_add_file,sd_web,pck/poll.pl,webview/poll.pl))
$(eval $(call gb_Package_add_file,sd_web,pck/savepic.asp,webview/savepic.asp))
$(eval $(call gb_Package_add_file,sd_web,pck/savepic.pl,webview/savepic.pl))
$(eval $(call gb_Package_add_file,sd_web,pck/show.asp,webview/show.asp))
$(eval $(call gb_Package_add_file,sd_web,pck/show.pl,webview/show.pl))
# the following two files are really renamed
$(eval $(call gb_Package_add_file,sd_web,pck/webcast.asp,webview/webview.asp))
$(eval $(call gb_Package_add_file,sd_web,pck/webcast.pl,webview/webview.pl))

# vim: set noet sw=4 ts=4:
