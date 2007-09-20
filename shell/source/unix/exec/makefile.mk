#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: vg $ $Date: 2007-09-20 14:47:10 $
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
PRJNAME=shell
TARGET=exec

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

COMP1TYPELIST=syssh

TESTAPP1=urltest

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

DLLPRE=

SLOFILES=$(SLO)$/shellexec.obj\
    $(SLO)$/shellexecentry.obj
    
SHL1OBJS=$(SLOFILES) 
    
SHL1TARGET=syssh.uno
.IF "$(GUI)" == "OS2"
SHL1IMPLIB=i$(TARGET)
.ELSE
SHL1IMPLIB=
.ENDIF

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
                    
SHL1STDLIBS=$(CPPULIB)\
            $(CPPUHELPERLIB)\
            $(SALLIB)
SHL1LIBS=
SHL1DEPN=

.IF "$(test)" != "" 

APP1TARGET=$(TESTAPP1)
APP1STDLIBS= $(SHL1STDLIBS)
APP1OBJS= \
    $(SLO)$/shellexec.obj \
    $(SLO)$/$(APP1TARGET).obj

.ENDIF # "$(test)" != "" 


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

run_test : $(BIN)$/$(TESTAPP1).sh
    dmake test=t
    $(BIN)$/$(TESTAPP1) urltest.txt

$(BIN)$/$(TESTAPP1).sh : $$(@:f)
    $(COPY) $< $@
    -chmod +x $@ 
