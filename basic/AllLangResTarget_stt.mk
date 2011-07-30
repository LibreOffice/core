# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,stt))

$(eval $(call gb_AllLangResTarget_add_srs,stt,\
	stt/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,stt/res))

$(eval $(call gb_SrsTarget_set_include,stt/res,\
	$$(INCLUDE) \
))

$(eval $(call gb_SrsTarget_add_files,stt/res,\
	basic/source/app/basic.src \
	basic/source/app/basmsg.src \
	basic/source/app/svtmsg.src \
	basic/source/app/testtool.src \
	basic/source/app/ttmsg.src \
))

# vim: set noet sw=4 ts=4:
