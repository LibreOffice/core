#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.8 $
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
TARGET=javafilter
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SOLAR_JAVA)"=="TRUE"
SCP_PRODUCT_TYPE=osl
PARFILES= \
        module_javafilter.par              \
        file_javafilter.par
.ENDIF

.IF "$(SOLAR_JAVA)"=="TRUE"
.IF "$(GUI)"=="WNT"
PARFILES += \
        registryitem_javafilter.par
.ENDIF
.ENDIF

ULFFILES= \
        module_javafilter.ulf              \
        registryitem_javafilter.ulf

# --- File ---------------------------------------------------------
.INCLUDE :  target.mk
