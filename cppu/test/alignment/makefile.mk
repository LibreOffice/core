#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: dbo $ $Date: 2001-11-08 16:21:44 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

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
APP1STDLIBS += $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB)
APP1TARGET = pass1

APP2OBJS = $(OBJ)$/pass2.obj
#APP2STDLIBS += $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB)
APP2TARGET = pass2

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALLTAR: execute_pass2
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE :  target.mk

$(MISC)$/pass2.cxx: $(APP1TARGETN)
    +regmerge $(UNOUCRRDB) / $(merge_rdb)
    +cppumaker @$(mktmp $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) $(foreach,c,$(shell $(APP1TARGETN) -env:UNO_TYPES={$(subst,\,\\ $(UNOUCRRDB))} $(subst,\,\\ $(MISC)$/pass2.cxx) dump_types) -T$c) $(UNOUCRRDB))

execute_pass2: $(APP2TARGETN)
    $(APP2TARGETN)

