# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,uuresolver))

$(eval $(call gb_Library_set_componentfile,uuresolver,remotebridges/source/unourl_resolver/uuresolver,ure/services))

$(eval $(call gb_Library_use_udk_api,uuresolver))

$(eval $(call gb_Library_use_libraries,uuresolver,\
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_add_exception_objects,uuresolver,\
	remotebridges/source/unourl_resolver/unourl_resolver \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
