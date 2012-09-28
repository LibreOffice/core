# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_Library_Library,log))

$(eval $(call gb_Library_set_componentfile,log,extensions/source/logging/log))

$(eval $(call gb_Library_use_sdk_api,log))

$(eval $(call gb_Library_add_exception_objects,log,\
	extensions/source/logging/consolehandler \
	extensions/source/logging/csvformatter \
	extensions/source/logging/filehandler \
	extensions/source/logging/log_services \
	extensions/source/logging/log_module \
	extensions/source/logging/logger \
	extensions/source/logging/loggerconfig \
	extensions/source/logging/loghandler \
	extensions/source/logging/logrecord \
	extensions/source/logging/plaintextformatter \
))

$(eval $(call gb_Library_use_libraries,log,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	$(gb_UWINAPI) \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
