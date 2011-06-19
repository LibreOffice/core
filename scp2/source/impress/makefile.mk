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

PRJ=..$/..

PRJPCH=

PRJNAME=scp2
TARGET=impress
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

SCP_PRODUCT_TYPE=osl

.IF "$(ENABLE_OPENGL)"=="TRUE"
SCPDEFS+=-DENABLE_OPENGL
.ENDIF

PARFILES= \
        module_impress.par              \
        file_impress.par 

.IF "$(GUI)"=="WNT"
PARFILES += \
        registryitem_impress.par        \
        folderitem_impress.par
.ENDIF

ULFFILES= \
        module_impress.ulf              \
        registryitem_impress.ulf        \
        folderitem_impress.ulf

.IF "$(ENABLE_OPENGL)" == "TRUE"
PARFILES+=\
        module_ogltrans.par
ULFFILES+=\
    module_ogltrans.ulf
.ENDIF

# --- File ---------------------------------------------------------
.INCLUDE :  target.mk
