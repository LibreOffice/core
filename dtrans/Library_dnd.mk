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

$(eval $(call gb_Library_Library,dnd))

$(eval $(call gb_Library_set_componentfile,dnd,dtrans/util/dnd))

$(eval $(call gb_Library_set_include,dnd,\
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,dnd))

$(eval $(call gb_Library_use_libraries,dnd,\
	cppu \
	cppuhelper \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_system_win32_libs,dnd,\
	advapi32 \
	gdi32 \
	ole32 \
	oleaut32 \
	shell32 \
	uuid \
))

$(eval $(call gb_Library_add_standard_system_libs,dnd))

$(eval $(call gb_Library_use_static_libraries,dnd,\
	dtobj \
))

$(eval $(call gb_Library_add_exception_objects,dnd,\
	dtrans/source/win32/dnd/dndentry \
	dtrans/source/win32/dnd/globals \
	dtrans/source/win32/dnd/idroptarget \
	dtrans/source/win32/dnd/sourcecontext \
	dtrans/source/win32/dnd/source \
	dtrans/source/win32/dnd/target \
	dtrans/source/win32/dnd/targetdragcontext \
	dtrans/source/win32/dnd/targetdropcontext \
))

# vim: set noet sw=4 ts=4:
