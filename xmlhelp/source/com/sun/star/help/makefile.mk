#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.32 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 07:46:38 $
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

PRJ		= ..$/..$/..$/..$/..
PRJNAME = xmlhelp
TARGET  = HelpLinker
LIBBASENAME = helplinker
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE : helplinker.pmk
 
.IF "$(SYSTEM_LIBXSLT)" == "YES"
CFLAGS+= $(LIBXSLT_CFLAGS)
.ELSE
LIBXSLTINCDIR=external$/libxslt
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXSLTINCDIR)
.ENDIF

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF

OBJFILES=\
        $(OBJ)$/HelpLinker.obj \
        $(OBJ)$/HelpCompiler.obj
SLOFILES=\
        $(SLO)$/HelpLinker.obj \
        $(SLO)$/HelpCompiler.obj

EXCEPTIONSFILES=\
        $(OBJ)$/HelpLinker.obj \
        $(OBJ)$/HelpCompiler.obj \
        $(SLO)$/HelpLinker.obj \
        $(SLO)$/HelpCompiler.obj


APP1TARGET= $(TARGET)
APP1OBJS=\
      $(OBJ)$/HelpLinker.obj \
      $(OBJ)$/HelpCompiler.obj

APP1STDLIBS+=$(SALLIB) $(BERKELEYLIB) $(ICUUCLIB) $(XSLTLIB) $(EXPATASCII3RDLIB)

SHL1TARGET	=$(LIBBASENAME)$(DLLPOSTFIX)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1IMPLIB	=i$(LIBBASENAME)
SHL1DEF		=$(MISC)$/$(SHL1TARGET).def
SHL1STDLIBS =$(SALLIB) $(BERKELEYLIB) $(ICUUCLIB) $(XSLTLIB) $(EXPATASCII3RDLIB)
SHL1USE_EXPORTS	=ordinal

DEF1NAME	=$(SHL1TARGET) 
DEFLIB1NAME	=$(TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
