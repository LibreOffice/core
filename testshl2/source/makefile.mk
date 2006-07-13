#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: obo $ $Date: 2006-07-13 10:02:57 $
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
PRJ=..

PRJNAME=testshl2
TARGET=testshl2
TARGETTYPE=CUI
PRJINC=.

ENABLE_EXCEPTIONS=TRUE
#USE_LDUMP2=TRUE
#LDUMP2=LDUMP3

# --- Settings -----------------------------------------------------
.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# ------------------------------------------------------------------

COMMON_OBJFILES=\
    $(OBJ)$/getopt.obj \
    $(OBJ)$/filehelper.obj \
    $(OBJ)$/dynamicregister.obj 

TESTSHL_OBJFILES=\
    $(OBJ)$/autoregisterhelper.obj \
    $(OBJ)$/testshl.obj 

VERSIONINFO_OBJFILES=\
    $(OBJ)$/versionhelper.obj \
    $(OBJ)$/versioner.obj 

BMSMA_OBJFILES=\
    $(OBJ)$/bmsma.obj

# for right dependencies
# OBJFILES= $(COMMON_OBJFILES) $(TESTSHL_OBJFILES) $(VERSIONINFO_OBJFILES)

# ------------------------------------------------------------------
APP1TARGET= $(TARGET)
APP1OBJS= $(COMMON_OBJFILES) $(TESTSHL_OBJFILES)

APP1STDLIBS=$(SALLIB)

APP1DEPN= \
    $(LB)$/cppunit_no_regallfkt.lib \
    $(SLB)$/testresult.lib

APP1LIBS= \
    $(LB)$/cppunit_no_regallfkt.lib \
    $(SLB)$/testresult.lib

.IF "$(GUI)"=="WNT" 
APP1LIBS += $(LB)$/winstuff.lib
.ENDIF

#
# ------------------------------------------------------------------
APP2TARGET= versioninfo
APP2OBJS= $(COMMON_OBJFILES) $(VERSIONINFO_OBJFILES)

APP2STDLIBS=$(SALLIB)

APP2DEPN= 
APP2LIBS= 

# ------------------------------------------------------------------
APP3TARGET= $(TARGET)_timeout
APP3OBJS= 	$(OBJ)$/getopt.obj $(OBJ)$/filehelper.obj $(OBJ)$/terminate.obj

APP3STDLIBS=$(SALLIB)

APP3DEPN= 
APP3LIBS= 

.IF "$(GUI)"=="WNT" 
APP3LIBS += $(LB)$/winstuff.lib
.ENDIF


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
