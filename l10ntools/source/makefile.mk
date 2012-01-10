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



PRJ=..

INCPRE=$(MISC)

PRJNAME=l10ntools
TARGET=transex
TARGETTYPE=CUI
LIBTARGET=no
# --- Settings -----------------------------------------------------
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  settings.mk
CDEFS+= -DYY_NEVER_INTERACTIVE=1
#CDEFS+= -pg

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF

    
# --- Files --------------------------------------------------------

OBJFILES=   			\
    $(OBJ)$/export.obj	\
    $(OBJ)$/export2.obj	\
    $(OBJ)$/merge.obj   \
    $(OBJ)$/srciter.obj		\
    $(OBJ)$/utf8conv.obj	\
    $(OBJ)$/xmlparse.obj    \
    $(OBJ)$/helpmerge.obj   \
    $(OBJ)$/helpex.obj      \
    $(OBJ)$/file.obj        \
    $(OBJ)$/directory.obj   


LIB1TARGET= $(LB)$/$(TARGET).lib
LIB1ARCHIV= $(LB)$/libtransex.a
#LIB1FILES=  $(LB)$/transex3.lib
LIB1OBJFILES=        $(OBJ)$/export.obj      \
        $(OBJ)$/export2.obj     \
        $(OBJ)$/merge.obj   \
        $(OBJ)$/srciter.obj             \
        $(OBJ)$/file.obj \
        $(OBJ)$/directory.obj     \
        $(OBJ)$/utf8conv.obj    
   

APP1VERSIONMAP=exports.map

# extractor and merger for *.src and *.hrc
APP1TARGET=  transex3
#APP1OBJS=   $(OBJ)$/src_yy.obj
APP1OBJS=   $(OBJ)$/src_yy_wrapper.obj

APP1STDLIBS+= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)

.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
.ENDIF

APP1LIBS+=	$(LB)$/$(TARGET).lib
APP1DEPN=   $(OBJ)$/src_yy_wrapper.obj $(LB)$/$(TARGET).lib

APP2TARGET= helpex
APP2OBJS= $(OBJ)$/helpmerge.obj  $(OBJ)$/xmlparse.obj $(OBJ)$/export2.obj $(OBJ)$/utf8conv.obj $(OBJ)$/merge.obj $(OBJ)$/helpex.obj 
APP2RPATH= NONE

.IF "$(OS)"!="MACOSX"
.ENDIF

APP2STDLIBS+=$(SALLIB) $(EXPATASCII3RDLIB) $(TOOLSLIB) $(VOSLIB)

.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
.ENDIF

# extractor and merger for *.lng and *.lng
APP3TARGET= ulfex
APP3OBJS=   $(OBJ)$/lngmerge.obj $(OBJ)$/merge.obj $(OBJ)$/export2.obj $(OBJ)$/lngex.obj $(OBJ)$/utf8conv.obj
APP3RPATH=  NONE

.IF "$(OS)"!="MACOSX"
#APP3STDLIBS+= $(BTSTRPLIB)
.ENDIF
APP3STDLIBS+= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)
.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
.ENDIF

# encoding converter for *.gsi
APP4TARGET= gsiconv
APP4OBJS=   $(OBJ)$/utf8conv.obj $(OBJ)$/gsiconv.obj
APP4STDLIBS+= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)

# tag checker for *.gsi
APP5TARGET= gsicheck
APP5OBJS=   $(OBJ)$/gsicheck.obj $(OBJ)$/tagtest.obj
APP5STDLIBS+= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)

# extractor and merger for *.cfg
APP6TARGET= cfgex
APP6OBJS=   $(OBJ)$/cfgmerge.obj $(OBJ)$/cfg_yy_wrapper.obj  $(OBJ)$/merge.obj $(OBJ)$/export2.obj $(OBJ)$/utf8conv.obj

.IF "$(OS)"!="MACOSX"
#APP6STDLIBS+= $(BTSTRPLIB)
.ENDIF

APP6STDLIBS+= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)

.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
.ENDIF

# extractor and merger for *.xrm
APP7TARGET= xrmex
APP7OBJS=   $(OBJ)$/xrmmerge.obj $(OBJ)$/xrm_yy_wrapper.obj $(OBJ)$/merge.obj $(OBJ)$/export2.obj $(OBJ)$/utf8conv.obj
APP7RPATH=  NONE

.IF "$(OS)"!="MACOSX"
.ENDIF

APP7STDLIBS+= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB)

.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
.ENDIF

# 
#APP8TARGET= treeconfig
#APP8OBJS=   $(OBJ)$/treeconfig.obj $(OBJ)$/inireader.obj $(OBJ)$/export2.obj
#APP8STDLIBS=$(TOOLSLIB) $(SALLIB) $(VOSLIB) $(ICUINLIB) $(STLPORT) 

# localizer for l10n framework
APP9TARGET= localize_sl
EXCEPTIONSFILES=                            \
                    $(OBJ)$/localize.obj
APP9OBJS=   $(OBJ)$/localize.obj $(OBJ)$/utf8conv.obj $(OBJ)$/srciter.obj $(OBJ)$/export2.obj $(OBJ)$/file.obj $(OBJ)$/directory.obj $(OBJ)$/treeconfig.obj $(OBJ)$/inireader.obj

APP9STDLIBS+= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(ICUINLIB) \
            $(ICUUCLIB) \
            $(STLPORTLIB) \
            $(SALLIB)

DEPOBJFILES=$(APP1OBJS) $(APP2OBJS) $(APP3OBJS) $(APP4OBJS) $(APP5OBJS) $(APP6OBJS) $(APP7OBJS) $(APP8OBJS) $(APP9OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/%_yy.c : %lex.l
    flex -l -w -8 -o$@ $<

# Helper to suppress warnings in lex generated c code, see #i57362#

$(OBJ)$/src_yy_wrapper.obj: $(MISC)$/src_yy.c
$(OBJ)$/cfg_yy_wrapper.obj: $(MISC)$/cfg_yy.c
$(OBJ)$/xrm_yy_wrapper.obj: $(MISC)$/xrm_yy.c

