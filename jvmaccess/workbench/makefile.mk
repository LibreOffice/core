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

PRJ = ..
PRJNAME = jvmaccess
TARGET = workbench

ENABLE_EXCEPTIONS = TRUE

LIBTARGET = NO
TARGETTYPE = CUI

.INCLUDE: settings.mk

APP1TARGET = exceptiontest1
APP1OBJS = $(OBJ)$/exceptiontest1.obj
APP1STDLIBS = \
    $(JVMACCESSLIB) \
    $(SALLIB)

APP2TARGET = exceptiontest2
APP2OBJS = $(OBJ)$/exceptiontest2.obj
APP2STDLIBS = \
    $(JVMACCESSLIB) \
    $(SALLIB)

.INCLUDE: target.mk
