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
# $Revision: 1.9 $
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

#
# build /test first, then /test/alignment
#

PRJ=..$/..
PRJNAME=cppu
TARGET=alignment
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=$(BIN)$/testcppu.rdb
UNOUCRRDB=$(BIN)$/testcppu.rdb
UNOUCROUT=$(INCCOM)$/test$/alignment
INCPRE+=$(INCCOM)$/test -I$(INCCOM)$/test$/alignment -I$(PRJ)$/test$/alignment

.IF "$(src_env)" == ""
merge_rdb=$(SOLARBINDIR)$/udkapi.rdb
.ELSE
merge_rdb=$(SOLARBINDIR)$/applicat.rdb
.ENDIF

DEPOBJFILES= \
    $(OBJ)$/pass1.obj	\
    $(OBJ)$/pass2.obj

APP1OBJS = $(OBJ)$/pass1.obj
APP1STDLIBS += $(CPPUHELPERLIB) $(CPPULIB) $(REGLIB) $(SALHELPERLIB) $(SALLIB)
APP1TARGET = pass1

APP2OBJS = $(OBJ)$/pass2.obj
APP2STDLIBS = $(SALLIB)
APP2TARGET = pass2

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALLTAR: execute_pass2
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE :  target.mk

$(MISC)$/pass2.cxx: $(APP1TARGETN)
    regmerge $(UNOUCRRDB) / $(merge_rdb)
    cppumaker @$(mktmp $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) $(foreach,c,$(shell @$(APP1TARGETN) -env:UNO_TYPES={$(subst,\,\\ $(UNOUCRRDB))} $(subst,\,\\ $(MISC)$/pass2.cxx) dump_types) -T$c) $(UNOUCRRDB))

execute_pass2: $(APP2TARGETN)
    $(APP2TARGETN)

