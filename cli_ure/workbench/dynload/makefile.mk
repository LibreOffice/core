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

PRJ = ..$/..
PRJNAME = cli_ure

# for dummy
TARGET = dynload

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk

.IF "$(BUILD_FOR_CLI)" != ""

ALLTAR : \
    $(OUT)$/bin$/dynload.exe

CSFILES = \
    dynload.cs				

$(OUT)$/bin$/dynload.exe : $(CSFILES) 
        $(CSC) $(CSCFLAGS) \
        -target:exe \
        -out:$@ \
        -reference:$(OUT)$/bin$/cli_ure.dll \
        -reference:$(OUT)$/bin$/cli_types.dll \
        -reference:$(OUT)$/bin$/cli_cppuhelper.dll \
        -reference:System.dll \
        $(CSFILES) 

#$(OUT)$/bin$/dynload.ini : $(OUT)$/bin$/types.rdb
#	$(GNUCOPY) -p dynload.ini $(OUT)$/bin

#$(OUT)$/bin$/types.rdb $(OUT)$/bin$/services.rdb 

#$(BIN)$/types.rdb : $(SOLARBINDIR)$/types.rdb
#	$(GNUCOPY) -p $? $@



.ENDIF
