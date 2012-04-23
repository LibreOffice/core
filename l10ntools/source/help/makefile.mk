#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ		= ..$/..
PRJNAME = l10ntools
TARGET  = HelpLinker
LIBBASENAME = helplinker
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE : helplinker.pmk

CFLAGS+=-DL10N_DLLIMPLEMENTATION
 
.IF "$(SYSTEM_LIBXSLT)" == "YES"
CFLAGS+= $(LIBXSLT_CFLAGS)
.ELSE
LIBXSLTINCDIR=external$/libxslt
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXSLTINCDIR)
.ENDIF

CFLAGS+= $(SYSTEM_DB_CFLAGS)

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF

.IF "$(SYSTEM_CLUCENE)" == "YES"
CFLAGS+= $(CLUCENE_CFLAGS)
.ENDIF

OBJFILES=\
        $(OBJ)$/HelpLinker.obj \
        $(OBJ)$/HelpCompiler.obj \
        $(OBJ)$/HelpIndexer.obj \
        $(OBJ)$/HelpIndexer_main.obj \
	$(OBJ)$/HelpSearch.obj \
	$(OBJ)$/LuceneHelper.obj

SLOFILES=\
        $(SLO)$/HelpLinker.obj \
        $(SLO)$/HelpCompiler.obj \
	$(SLO)$/LuceneHelper.obj \
        $(SLO)$/HelpIndexer.obj \
	$(SLO)$/HelpSearch.obj

.IF "$(OS)" == "MACOSX" && "$(CPU)" == "P" && "$(COM)" == "GCC"
# There appears to be a GCC 4.0.1 optimization error causing _file:good() to
# report true right before the call to writeOut at HelpLinker.cxx:1.12 l. 954
# but out.good() to report false right at the start of writeOut at
# HelpLinker.cxx:1.12 l. 537:
NOOPTFILES=\
        $(OBJ)$/HelpLinker.obj \
        $(SLO)$/HelpLinker.obj
.ENDIF

APP1TARGET= $(TARGET)
APP1OBJS=\
      $(OBJ)$/HelpLinker.obj \
      $(OBJ)$/HelpCompiler.obj
APP1RPATH = NONE
APP1STDLIBS+=$(SALLIB) $(BERKELEYLIB) $(XSLTLIB) $(EXPATASCII3RDLIB)

APP2TARGET=HelpIndexer
APP2OBJS=\
      $(OBJ)$/LuceneHelper.obj \
      $(OBJ)$/HelpIndexer.obj \
      $(OBJ)$/HelpIndexer_main.obj
APP2RPATH = NONE
APP2STDLIBS+=$(SALLIB) $(CLUCENELIB)

SHL1TARGET	=$(LIBBASENAME)$(DLLPOSTFIX)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
.IF "$(COM)" == "MSC"
SHL1IMPLIB	=i$(LIBBASENAME)
.ELSE
SHL1IMPLIB	=$(LIBBASENAME)$(DLLPOSTFIX)
.ENDIF
SHL1DEF		=$(MISC)$/$(SHL1TARGET).def
SHL1STDLIBS =$(SALLIB) $(BERKELEYLIB) $(XSLTLIB) $(EXPATASCII3RDLIB) $(CLUCENELIB)
SHL1USE_EXPORTS	=ordinal

DEF1NAME	=$(SHL1TARGET) 
DEFLIB1NAME	=$(TARGET)


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
