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

$(eval $(call gb_StaticLibrary_StaticLibrary,dtobj))

$(eval $(call gb_StaticLibrary_use_packages,dtobj,\
	comphelper_inc \
	cppu_odk_headers \
	cppuhelper_odk_headers \
	sal_generated \
	sal_inc \
	sal_odk_headers \
))

$(eval $(call gb_StaticLibrary_use_sdk_api,dtobj))

$(eval $(call gb_StaticLibrary_add_exception_objects,dtobj,\
	dtrans/source/win32/dtobj/APNDataObject \
	dtrans/source/win32/dtobj/DataFmtTransl \
	dtrans/source/win32/dtobj/DOTransferable \
	dtrans/source/win32/dtobj/DtObjFactory \
	dtrans/source/win32/dtobj/DTransHelper \
	dtrans/source/win32/dtobj/Fetc \
	dtrans/source/win32/dtobj/FetcList \
	dtrans/source/win32/dtobj/FmtFilter \
	dtrans/source/win32/dtobj/TxtCnvtHlp \
	dtrans/source/win32/dtobj/XNotifyingDataObject \
	dtrans/source/win32/dtobj/XTDataObject \
	dtrans/source/win32/misc/ImplHelper \
))

# vim: set noet sw=4 ts=4:
