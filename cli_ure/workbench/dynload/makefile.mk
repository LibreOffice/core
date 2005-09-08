#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 02:04:53 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
        +$(CSC) $(CSCFLAGS) \
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
