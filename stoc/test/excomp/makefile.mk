#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 08:26:20 $
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
PRJ=..$/..

PRJNAME=	stoc
TARGET=		excomp
TARGET1=	excomp1
TARGET2=	excomp2
TARGETTYPE=CUI
#LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=	TRUE
NO_BSYMBOLIC=	TRUE
COMP1TYPELIST=$(TARGET1)
COMP2TYPELIST=$(TARGET2)

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# ------------------------------------------------------------------

UNOUCRDEP=	$(BIN)$/excomp.rdb
UNOUCRRDB=	$(BIN)$/excomp.rdb
UNOUCROUT=	$(OUT)$/inc
INCPRE+=	$(OUT)$/inc

UNOTYPES=$($(TARGET1)_XML2CMPTYPES)
UNOTYPES+=$($(TARGET2)_XML2CMPTYPES)
UNOTYPES+=  com.sun.star.registry.XImplementationRegistration \
            com.sun.star.lang.XComponent

# --- Application excomp ------------------------------------------------
APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJ)$/excomp.obj 

APP1STDLIBS= \
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) 	\
        $(SALHELPERLIB) 	\
        $(SALLIB)

# ---- objects ----
SLOFILES= \
        $(SLO)$/excomp1.obj	\
          $(SLO)$/$(COMP1TYPELIST)_description.obj	\
        $(SLO)$/excomp2.obj	\
          $(SLO)$/$(COMP2TYPELIST)_description.obj

# ---- excomp1 ------
SHL1TARGET=	$(TARGET1)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALHELPERLIB)		\
        $(SALLIB)	

SHL1DEPN=
SHL1LIBS=
SHL1OBJS=  	$(SLO)$/excomp1.obj	\
              $(SLO)$/$(COMP1TYPELIST)_description.obj
SHL1IMPLIB=	i$(TARGET1)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# ---- excomp2 ------
SHL2TARGET=	$(TARGET2)

SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALHELPERLIB)		\
        $(SALLIB)

SHL2DEPN=
SHL2LIBS=
SHL2OBJS=  	$(SLO)$/excomp2.obj	\
              $(SLO)$/$(COMP2TYPELIST)_description.obj	
SHL2IMPLIB=	i$(TARGET2)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def

DEF2NAME=	$(SHL2TARGET)
DEF2EXPORTFILE=	exports.dxp

ALLIDLFILES:= example$/XTest.idl example$/ExampleComponent1.idl example$/ExampleComponent2.idl

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL : 		$(BIN)$/excomp.rdb	\
        ALLTAR 
.ELSE
ALL: 		ALLDEP
.ENDIF

.INCLUDE :	target.mk

$(BIN)$/excomp.rdb: $(ALLIDLFILES)
    +idlc -I$(PRJ) -I$(SOLARIDLDIR) -O$(MISC)$/excomp $?
    +regmerge $@ /UCR $(MISC)$/excomp$/{$(?:f:s/.idl/.urd/)}
    +regmerge $@ / $(SOLARBINDIR)$/udkapi.rdb
    touch $@

