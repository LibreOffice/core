#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 17:30:31 $
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

PRJNAME=soldep
TARGET=bootstrpdt
ENABLE_EXCEPTIONS=true


# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/util$/perl.mk
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES= \
          $(SLO)$/prj.obj	\
          $(SLO)$/XmlBuildList.obj

SLOFILES=\
          $(SLO)$/dep.obj \
          $(SLO)$/minormk.obj \
          $(SLO)$/prodmap.obj \
          $(SLO)$/appdef.obj \
           $(SLO)$/hashtbl.obj		\
          $(SLO)$/prj.obj \
          $(SLO)$/XmlBuildList.obj


SHL1TARGET	=$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB	=$(TARGET)
SHL1LIBS	=$(SLB)$/$(TARGET).lib
SHL1DEF		=$(MISC)$/$(SHL1TARGET).def
SHL1DEPN	=$(SHL1LIBS)
SHL1STDLIBS=\
            $(TOOLSLIB) \
            $(BTSTRPLIB)		\
            $(VOSLIB)			\
            $(SALLIB)   \
               $(PERL_LIB)

DEF1NAME    =$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME	=$(TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo __CT >> $@
