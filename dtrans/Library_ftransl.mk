# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,ftransl))

$(eval $(call gb_Library_set_componentfile,ftransl,dtrans/util/ftransl))

$(eval $(call gb_Library_set_include,ftransl,\
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,ftransl))

$(eval $(call gb_Library_use_libraries,ftransl,\
	cppu \
	cppuhelper \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_system_win32_libs,ftransl,\
	advapi32 \
	gdi32 \
	ole32 \
))

$(eval $(call gb_Library_use_static_libraries,ftransl,\
	dtobj \
))

$(eval $(call gb_Library_add_exception_objects,ftransl,\
	dtrans/source/win32/ftransl/ftransl \
	dtrans/source/win32/ftransl/ftranslentry \
))

# vim: set noet sw=4 ts=4:
