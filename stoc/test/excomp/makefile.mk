#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 15:29:36 $
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
        $(VOSLIB) 	\
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
        $(VOSLIB)		\
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
        $(VOSLIB)		\
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
    +unoidl -I$(PRJ) -I$(SOLARIDLDIR) -Burd -OH$(MISC)$/excomp $?
    +regmerge $@ /UCR $(MISC)$/excomp$/{$(?:f:s/.idl/.urd/)}
    +regmerge $@ / $(SOLARBINDIR)$/applicat.rdb
    touch $@

