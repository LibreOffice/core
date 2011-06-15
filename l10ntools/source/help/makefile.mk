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
PACKAGE = com$/sun$/star$/help
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

CFLAGS+= $(SYSTEM_DB_CFLAGS)

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
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

APP1STDLIBS+=$(SALLIB) $(BERKELEYLIB) $(XSLTLIB) $(EXPATASCII3RDLIB)

SHL1TARGET	=$(LIBBASENAME)$(DLLPOSTFIX)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1IMPLIB	=i$(LIBBASENAME)
SHL1DEF		=$(MISC)$/$(SHL1TARGET).def
SHL1STDLIBS =$(SALLIB) $(BERKELEYLIB) $(XSLTLIB) $(EXPATASCII3RDLIB)
SHL1USE_EXPORTS	=ordinal

DEF1NAME	=$(SHL1TARGET) 
DEFLIB1NAME	=$(TARGET)

JAVAFILES = \
    HelpIndexerTool.java			        \
    HelpFileDocument.java


JAVACLASSFILES = \
    $(CLASSDIR)$/$(PACKAGE)$/HelpIndexerTool.class			        \
    $(CLASSDIR)$/$(PACKAGE)$/HelpFileDocument.class

.IF "$(SYSTEM_LUCENE)" == "YES"
EXTRAJARFILES += $(LUCENE_CORE_JAR) $(LUCENE_ANALYZERS_JAR)
.ELSE
JARFILES += lucene-core-2.3.jar lucene-analyzers-2.3.jar
.ENDIF
JAVAFILES = $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES)))

JARCLASSDIRS	   = $(PACKAGE)/*
JARTARGET	       = HelpIndexerTool.jar
JARCOMPRESS        = TRUE 
 
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
