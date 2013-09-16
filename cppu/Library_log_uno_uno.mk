# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,log_uno_uno))

$(eval $(call gb_Library_use_external,log_uno_uno,boost_headers))

$(eval $(call gb_Library_use_udk_api,log_uno_uno))

$(eval $(call gb_Library_use_libraries,log_uno_uno,\
	cppu \
	purpenvhelper \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,log_uno_uno,\
	cppu/source/LogBridge/LogBridge \
))

# vim: set noet sw=4 ts=4:
