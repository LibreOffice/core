#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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

APP1STDLIBS+=$(SALLIB) $(XSLTLIB) $(EXPATASCII3RDLIB)

SHL1TARGET	=$(LIBBASENAME)$(DLLPOSTFIX)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1IMPLIB	=i$(LIBBASENAME)
SHL1DEF		=$(MISC)$/$(SHL1TARGET).def
SHL1STDLIBS =$(SALLIB) $(XSLTLIB) $(EXPATASCII3RDLIB)
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
