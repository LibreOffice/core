# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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

$(eval $(call gb_Executable_Executable,svidl))

$(eval $(call gb_Executable_set_include,svidl,\
	$$(INCLUDE) \
	-I$(SRCDIR)/idl/inc \
))

$(eval $(call gb_Executable_use_sdk_api,svidl))

$(eval $(call gb_Executable_use_libraries,svidl,\
	tl \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_exception_objects,svidl,\
	idl/source/cmptools/hash \
	idl/source/cmptools/lex \
	idl/source/objects/basobj \
	idl/source/objects/bastype \
	idl/source/objects/module \
	idl/source/objects/object \
	idl/source/objects/slot \
	idl/source/objects/types \
	idl/source/prj/command \
	idl/source/prj/database \
	idl/source/prj/globals \
	idl/source/prj/svidl \
))

# vim: set noet sw=4 ts=4:
