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

PROJECTPCH=parser
PROJECTPCHSOURCE=parser
PRJNAME=rsc
TARGET=rscpar

# --- Settings -----------------------------------------------------

#prjpch=T

ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

.IF "$(BUILD_TYPE)"=="$(BUILD_TYPE:s/DESKTOP//)"
ALL:
    @echo No need for this on non-desktop OSes
.ENDIF

# --- Files --------------------------------------------------------

YACCTARGET= $(INCCOM)$/yyrscyacc.cxx
YACCFILES=  rscyacc.y

OBJFILES=   $(OBJ)$/rscpar.obj   \
            $(OBJ)$/rscyacc.obj  \
            $(OBJ)$/rsclex.obj   \
            $(OBJ)$/erscerr.obj  \
            $(OBJ)$/rsckey.obj   \
            $(OBJ)$/rscinit.obj  \
            $(OBJ)$/rscibas.obj  \
            $(OBJ)$/rscdb.obj    \
            $(OBJ)$/rscicpx.obj

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(OBJ)$/rscibas.obj : $(YACCTARGET)
$(OBJ)$/rscinit.obj : $(YACCTARGET)
$(OBJ)$/rscicpx.obj : $(YACCTARGET)
$(OBJ)$/rsclex.obj  : $(YACCTARGET)
$(OBJ)$/rscyacc.obj : $(YACCTARGET)

