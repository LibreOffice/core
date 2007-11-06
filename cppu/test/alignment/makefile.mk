#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 16:02:09 $
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

