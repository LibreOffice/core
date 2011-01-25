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

# RES is the name of the resource; its file name will be RES-LOCALE.res
# MODULE is the name of the code module the makefile is located in

$(eval $(call gb_AllLangResTarget_AllLangResTarget,RES))

# declaration of images folder (e.g. default_images/MODULE/res)
$(eval $(call gb_AllLangResTarget_set_reslocation,RES,MODULE))

# declaration of a resource intermediate file (srs)
$(eval $(call gb_AllLangResTarget_add_srs,RES, RES/res))
$(eval $(call gb_SrsTarget_SrsTarget,RES/res))

# add any additional include paths here
$(eval $(call gb_SrsTarget_set_include,RES/res,\
    $$(INCLUDE) \
))

# add src files here (complete path relative to repository root)
$(eval $(call gb_SrsTarget_add_files,RES/res,\
))

# vim: set noet sw=4 ts=4:
