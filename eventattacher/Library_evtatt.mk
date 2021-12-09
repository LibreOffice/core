# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,evtatt))

$(eval $(call gb_Library_set_componentfile,evtatt,eventattacher/source/evtatt,services))

$(eval $(call gb_Library_use_libraries,evtatt,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
))

$(eval $(call gb_Library_add_exception_objects,evtatt,\
	eventattacher/source/eventattacher \
))

$(eval $(call gb_Library_use_api,evtatt,\
	udkapi \
))

# vim: set ts=4 sw=4 et:
