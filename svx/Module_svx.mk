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

$(eval $(call gb_Module_Module,svx))

$(eval $(call gb_Module_add_targets,svx,\
    Library_svx \
    Library_svxcore \
    Library_textconversiondlgs \
    AllLangResTarget_svx \
    AllLangResTarget_ofa \
    AllLangResTarget_gal \
    AllLangResTarget_textconversiondlgs \
    Package_globlmn_hrc \
    Package_inc \
    Package_sdi \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,svx,\
    Executable_gengal.bin \
    Package_gengal \
))
endif

$(eval $(call gb_Module_add_subsequentcheck_targets,svx,\
    JunitTest_svx_unoapi \
))

#todo: noopt for EnhanceCustomShapesFunctionParser.cxx on Solaris Sparc and MacOSX
#todo: -DBOOST_SPIRIT_USE_OLD_NAMESPACE only in CustomShapes ?
#todo: -DUNICODE and -D_UNICODE on WNT for source/dialog
#todo: globlmn.hrc
#todo: component file
# vim: set noet sw=4 ts=4:
