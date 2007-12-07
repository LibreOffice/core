#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: hjs $ $Date: 2007-12-07 14:17:37 $
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
    $(IDLC) -C -O $(MY_GEN_UCR_PATH) -I $(MY_GEN_IDL_PATH)$(PATH_SEPERATOR)$(SOLARIDLDIR) $?

$(MYDBTARGET) : $(MYURDFILES)  $(MYIDLFILES)
    $(REGMERGE) $(OUT)$/ucr/constants.db /UCR @$(mktmp $(MYURDFILES))

.ENDIF

