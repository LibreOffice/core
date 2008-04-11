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
# $Revision: 1.5 $
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
PRJ=..$/..$/..

PRJNAME=oovbapi

TARGET=constants
PACKAGE=org$/openoffice$/constants

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_VBA)"!="YES"
dummy:
        @echo "not building vba..."
.ELSE

# ------------------------------------------------------------------------

# I tried to use the IDLFILES target but it failed dismally
MY_GEN_IDL_PATH=$(MISC)$/idl
MY_GEN_UCR_PATH=$(OUT)$/ucr$/$(PACKAGE)
MYTMPIDLFILES=$(shell @ls $(MY_GEN_IDL_PATH))
#IDLFILES=$(foreach,i,$(MYTMPIDLFILES) $(MY_GEN_IDL_PATH)$/$(i))
MYIDLFILES=$(foreach,i,$(MYTMPIDLFILES) $(MY_GEN_IDL_PATH)$/$(i))
MYURDFILES=$(foreach,i,$(MYIDLFILES) $(MY_GEN_UCR_PATH)$/$(i:b).urd)
MYDBTARGET=$(OUT)$/ucr/constants.db


.INCLUDE :  target.mk

ALLTAR: $(MYDBTARGET)

$(MY_GEN_UCR_PATH)$/%.urd: $(MY_GEN_IDL_PATH)$/%.idl
    $(IDLC) -C -O $(MY_GEN_UCR_PATH) -I $(MY_GEN_IDL_PATH) -I $(SOLARIDLDIR) $?

$(MYDBTARGET) : $(MYURDFILES)  $(MYIDLFILES)
    $(REGMERGE) $(OUT)$/ucr/constants.db /UCR @$(mktmp $(MYURDFILES))

.ENDIF

