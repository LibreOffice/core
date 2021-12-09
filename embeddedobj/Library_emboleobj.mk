# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,emboleobj))

$(eval $(call gb_Library_set_componentfile,emboleobj,embeddedobj/source/msole/emboleobj$(if $(filter WNT,$(OS)),.windows),services))

$(eval $(call gb_Library_set_include,emboleobj,\
	-I$(SRCDIR)/embeddedobj/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_external,emboleobj,boost_headers))

$(eval $(call gb_Library_use_sdk_api,emboleobj))

$(eval $(call gb_Library_use_libraries,emboleobj,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	utl \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,emboleobj,\
	embeddedobj/source/msole/closepreventer \
	embeddedobj/source/msole/oleembed \
	embeddedobj/source/msole/olemisc \
	embeddedobj/source/msole/olepersist \
	embeddedobj/source/msole/olevisual \
	embeddedobj/source/msole/ownview \
	embeddedobj/source/msole/xolefactory \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Library_use_libraries,emboleobj,\
	embobj \
))

$(eval $(call gb_Library_use_system_win32_libs,emboleobj,\
	gdi32 \
	ole32 \
	oleaut32 \
	uuid \
))

$(eval $(call gb_Library_add_exception_objects,emboleobj,\
	embeddedobj/source/msole/advisesink \
	embeddedobj/source/msole/graphconvert \
	embeddedobj/source/msole/olecomponent \
	embeddedobj/source/msole/olewrapclient \
	embeddedobj/source/msole/xdialogcreator \
))

endif

# vim: set noet sw=4 ts=4:
