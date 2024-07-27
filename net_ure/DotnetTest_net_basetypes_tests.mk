# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_DotnetTest_DotnetTest,net_basetypes_tests,$(gb_DotnetTest_CS)))

$(eval $(call gb_DotnetTest_add_sources,net_basetypes_tests,\
	net_ure/qa/basetypes/AnyTests \
))

$(eval $(call gb_DotnetTest_link_library,net_basetypes_tests,net_basetypes))

# vim: set noet sw=4 ts=4:
