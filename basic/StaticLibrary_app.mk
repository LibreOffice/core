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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_StaticLibrary_StaticLibrary,app))

$(eval $(call gb_StaticLibrary_add_package_headers,app,basic_inc))

$(eval $(call gb_StaticLibrary_add_precompiled_header,app,$(SRCDIR)/basic/inc/pch/precompiled_basic))

$(eval $(call gb_StaticLibrary_set_include,app,\
	$$(INCLUDE) \
	-I$(realpath $(SRCDIR)/basic/inc) \
	-I$(realpath $(SRCDIR)/basic/inc/pch) \
	-I$(realpath $(SRCDIR)/basic/source/inc) \
))

$(eval $(call gb_StaticLibrary_add_api,app,\
	udkapi \
	offapi \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,app,\
	basic/source/app/appbased \
	basic/source/app/app \
	basic/source/app/appedit \
	basic/source/app/apperror \
	basic/source/app/appwin \
	basic/source/app/basicrt \
	basic/source/app/brkpnts \
	basic/source/app/dialogs \
	basic/source/app/msgedit \
	basic/source/app/mybasic \
	basic/source/app/process \
	basic/source/app/processw \
	basic/source/app/status \
	basic/source/app/textedit \
))

# vim: set noet sw=4 ts=4:
