# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,vcl))

$(eval $(call gb_AllLangResTarget_set_reslocation,vcl,vcl))

$(eval $(call gb_AllLangResTarget_add_srs,vcl,\
    vcl/source/src \
))

$(eval $(call gb_SrsTarget_SrsTarget,vcl/source/src))

$(eval $(call gb_SrsTarget_use_packages,vcl/source/src,\
	svl_inc \
))

$(eval $(call gb_SrsTarget_set_include,vcl/source/src,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_SrsTarget_add_files,vcl/source/src,\
    vcl/source/src/btntext.src \
    vcl/source/src/helptext.src \
    vcl/source/src/images.src \
    vcl/source/src/menu.src \
    vcl/source/src/print.src \
    vcl/source/src/stdtext.src \
    vcl/source/src/units.src \
    vcl/source/src/fpicker.src \
    vcl/source/edit/textundo.src \
))


# vim: set noet sw=4 ts=4:
