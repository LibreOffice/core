# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
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
# 	Peter Foley <pefoley2@verizon.net>
# Portions created by the Initial Developer are Copyright (C) 2011 the
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
#

$(eval $(call gb_Executable_Executable,xml2cmp))

$(eval $(call gb_Executable_add_exception_objects,xml2cmp,\
	xml2cmp/source/xcd/main \
	xml2cmp/source/support/cmdline \
	xml2cmp/source/support/heap \
	xml2cmp/source/support/sistr \
	xml2cmp/source/support/syshelp \
	xml2cmp/source/support/badcast \
	xml2cmp/source/xcd/cr_html \
	xml2cmp/source/xcd/cr_metho \
	xml2cmp/source/xcd/filebuff \
	xml2cmp/source/xcd/parse \
	xml2cmp/source/xcd/xmlelem \
	xml2cmp/source/xcd/xmltree \
	xml2cmp/source/xcd/dependy \
))

# vim: set noet sw=4 ts=4:
