# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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

# vim: set noet ts=4 sw=4:
