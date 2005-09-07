#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 19:30:09 $
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

PRJNAME= jvmfwk

TARGET = plugin

ENABLE_EXCEPTIONS=TRUE
BOOTSTRAP_SERVICE=FALSE

LIBTARGET=NO

UNOCOMPONENT1=sunjavaplugin

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

#.INCLUDE :  ..$/cppumaker.mk
.IF "$(SOLAR_JAVA)"!=""

UNOTYPES = \
    com.sun.star.uno.TypeClass \
    com.sun.star.uno.XInterface 

SLOFILES= \
    $(SLO)$/sunversion.obj \
    $(SLO)$/sunjavaplugin.obj \
    $(SLO)$/vendorbase.obj \
    $(SLO)$/util.obj \
    $(SLO)$/sunjre.obj \
    $(SLO)$/gnujre.obj \
    $(SLO)$/vendorlist.obj \
    $(SLO)$/otherjre.obj 

LIB1OBJFILES= $(SLOFILES)



LIB1TARGET=$(SLB)$/$(UNOCOMPONENT1).lib

SHL1TARGET=	$(UNOCOMPONENT1)  


SHL1STDLIBS= \
        $(CPPULIB) \
        $(CPPUHELPER) \
        $(SALLIB) \
        $(SALHELPERLIB)
        

.IF "$(GUI)" == "WNT"
.IF "$(COM)"!="GCC"
SHL1STDLIBS += uwinapi.lib advapi32.lib
.ELSE
SHL1STDLIBS += -luwinapi -ladvapi32 
.ENDIF # GCC
.ENDIF #WNT

SHL1VERSIONMAP = sunjavaplugin.map
SHL1DEPN=
SHL1IMPLIB=	i$(UNOCOMPONENT1)
SHL1LIBS=	$(LIB1TARGET) 
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)


JAVACLASSFILES= \
    $(CLASSDIR)$/JREProperties.class					

JAVAFILES = $(subst,$(CLASSDIR)$/, $(subst,.class,.java $(JAVACLASSFILES))) 

.ENDIF # SOLAR_JAVA



# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(GUI)"=="WNT"
BOOTSTRAPFILE=$(BIN)$/sunjavaplugin.ini
.ELSE
BOOTSTRAPFILE=$(BIN)$/sunjavapluginrc
.ENDIF


$(BOOTSTRAPFILE): sunjavapluginrc
    +-$(COPY) $< $@


ALLTAR: \
    $(BOOTSTRAPFILE)

