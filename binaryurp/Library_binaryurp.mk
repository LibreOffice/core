# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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
#       Matúš Kukan <matus.kukan@gmail.com>
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

$(eval $(call gb_Library_Library,binaryurp))

$(eval $(call gb_Library_set_componentfile,binaryurp,binaryurp/source/binaryurp))

$(eval $(call gb_Library_use_sdk_api,binaryurp))

$(eval $(call gb_Library_use_libraries,binaryurp,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,binaryurp,\
	binaryurp/source/binaryany \
	binaryurp/source/bridge \
	binaryurp/source/bridgefactory \
	binaryurp/source/currentcontext \
	binaryurp/source/incomingrequest \
	binaryurp/source/lessoperators \
	binaryurp/source/marshal \
	binaryurp/source/outgoingrequests \
	binaryurp/source/proxy \
	binaryurp/source/reader \
	binaryurp/source/unmarshal \
	binaryurp/source/writer \
))

# vim: set noet sw=4 ts=4:
