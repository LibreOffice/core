#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: jl $ $Date: 2001-06-07 10:13:30 $
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

PRJNAME=	sal
TARGET=		unloading
TARGET1=samplelib1
TARGET2=samplelib2
#LIBTARGET=NO
TARGETTYPE=CUI
COMP1TYPELIST=$(TARGET1)

NO_BSYMBOLIC=	TRUE
ENABLE_EXCEPTIONS=TRUE

USE_DEFFILE=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

CPPUMAKERFLAGS =

.IF "$(BOOTSTRAP_SERVICE)" == "TRUE"
UNOUCROUT=	$(OUT)$/inc$/bootstrap
INCPRE+=	$(OUT)$/inc$/bootstrap
.ELSE
.IF "$(COM)" == "MSC"
CPPUMAKERFLAGS = -L
.ENDIF
UNOUCROUT=	$(OUT)$/inc$/light
INCPRE+=	$(OUT)$/inc$/light
.ENDIF


#SLOFILES=	\
#		$(SLO)$/samplelib1.obj	\
#		$(SLO)$/samplelib2.obj

LIB1TARGET=$(SLB)$/$(TARGET1).lib
LIB1OBJFILES= \
        $(SLO)$/samplelib1.obj

SHL1TARGET=	$(TARGET1)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)



SHL1DEPN=
#SHL1IMPLIB=	i$(TARGET1)
SHL1LIBS=	$(SLB)$/$(TARGET1).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1EXPORTFILE=	exports.dxp
DEF1NAME=	$(SHL1TARGET)
#-------------------------------------------------------

LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2OBJFILES= \
        $(SLO)$/samplelib2.obj

SHL2TARGET=	$(TARGET2)

SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL2DEPN=
SHL2LIBS=	$(SLB)$/$(TARGET2).lib
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2EXPORTFILE=	exports.dxp
DEF2NAME=	$(SHL2TARGET)

# generate exports ------------------------------------------------
#DEF1DEPN=      $(MISC)$/$(SHL1TARGET).flt
#DEFLIB1NAME=   $(TARGET)
#-----------------------------------------------------------------



# ------------------------------------------------------------------

APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)

APP1OBJS=	$(OBJ)$/unloadTest.obj

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)




LIBCIMT=MSVCPRTD.lib


.IF "$(GUI)"=="WNT"
#APP1STDLIBS += $(LIBCIMT)
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

#$(MISC)$/$(SHL1TARGET).flt : makefile.mk
#       +echo   _TI2       >$@
#       +echo   _TI1      >>$@

