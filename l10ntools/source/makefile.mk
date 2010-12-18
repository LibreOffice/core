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

APP2STDLIBS+=$(SALLIB) $(EXPATASCII3RDLIB) $(TOOLSLIB)

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
            $(SALLIB)
.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
.ENDIF

# encoding converter for *.gsi
APP4TARGET= gsiconv
APP4OBJS=   $(OBJ)$/utf8conv.obj $(OBJ)$/gsiconv.obj
APP4STDLIBS+= \
            $(TOOLSLIB) \
            $(SALLIB)

# tag checker for *.gsi
APP5TARGET= gsicheck
APP5OBJS=   $(OBJ)$/gsicheck.obj $(OBJ)$/tagtest.obj
APP5STDLIBS+= \
            $(TOOLSLIB) \
            $(SALLIB)

# extractor and merger for *.cfg
APP6TARGET= cfgex
APP6OBJS=   $(OBJ)$/cfgmerge.obj $(OBJ)$/cfg_yy_wrapper.obj  $(OBJ)$/merge.obj $(OBJ)$/export2.obj $(OBJ)$/utf8conv.obj

.IF "$(OS)"!="MACOSX"
#APP6STDLIBS+= $(BTSTRPLIB)
.ENDIF

APP6STDLIBS+= \
            $(TOOLSLIB) \
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
            $(SALLIB)

.IF "$(OS)"=="MACOSX"
# static libs at end for OS X
.ENDIF

# 
#APP8TARGET= treeconfig
#APP8OBJS=   $(OBJ)$/treeconfig.obj $(OBJ)$/inireader.obj $(OBJ)$/export2.obj
#APP8STDLIBS=$(TOOLSLIB) $(SALLIB)  $(ICUINLIB) $(STLPORT)

# localizer for l10n framework
APP9TARGET= localize_sl
EXCEPTIONSFILES=                            \
                    $(OBJ)$/localize.obj
APP9OBJS=   $(OBJ)$/localize.obj $(OBJ)$/utf8conv.obj $(OBJ)$/srciter.obj $(OBJ)$/export2.obj $(OBJ)$/file.obj $(OBJ)$/directory.obj $(OBJ)$/treeconfig.obj $(OBJ)$/inireader.obj

APP9STDLIBS+= \
            $(TOOLSLIB) \
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

