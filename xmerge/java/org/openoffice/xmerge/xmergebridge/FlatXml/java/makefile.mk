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

PRJ	= ..$/..$/..
PRJNAME = odk
TARGET  = examples_java_converterbridge

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

JARFILES 		= ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar ridl.jar
JAVAFILES		= $(subst,$(CLASSDIR)$/, $(subst,.class,.java $(JAVACLASSFILES))) 
JARCOMPRESS 	= TRUE
CUSTOMMANIFESTFILE = manifest

# --- Files --------------------------------------------------------

JAVACLASSFILES=\
    $(CLASSDIR)$/ConverterBridge.class
RDB 		= $(SOLARBINDIR)$/udkapi.rdb
JAVADIR 	= $(OUT)$/misc$/java

# --- Targets ------------------------------------------------------

INTERFACES=$(MISC)$/ConverterBridge.rdb

CPPUMAKERFLAGS += -C
UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb $(INTERFACES)
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb $(INTERFACES)

TARGETJARFILE=..$/class$/ConverterBridge.jar
#JAVACLASSFILES=$(CLASSDIR)$/$(PACKAGE)$/ConverterBridge.class
# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc

# adding to inludepath
INCPRE+=$(UNOUCROUT)

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL : $(CLASSDIR)$/$(TARGETJARFILE) ALLTAR 
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE :	target.mk


NEWCLASS:=$(foreach,i,$(JARFILES) $(null,$(shell @$(FIND) $(JARDIR) -name $i) $(SOLARBINDIR)$/$i $(shell @$(FIND) $(JARDIR) -name $i)))

.IF "$(GUI)"=="UNX"
MYCLASSPATH=$(CLASSPATH):$(NEWCLASS:s/ /:/)
.ELSE
MYCLASSPATH=$(CLASSPATH);$(NEWCLASS:s/ /;/)
.ENDIF


GENJAVACLASSFILES= \
    $(CLASSDIR)$/com$/sun$/star$/documentconversion$/XConverterBridge.class

GENJAVAFILES = {$(subst,.class,.java $(subst,$/class, $(GENJAVACLASSFILES)))}
TYPES={$(subst,.class, $(subst,$/,.  $(subst,$(CLASSDIR)$/,-T  $(GENJAVACLASSFILES))))}

$(GENJAVAFILES): $(INTERFACES)
    javamaker -BUCR -O$(OUT) $(TYPES) $(UNOUCRRDB)

$(GENJAVACLASSFILES): $(GENJAVAFILES)
    $(JAVAC) $(JAVACPS) $(MYCLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $<

$(CLASSDIR)$/$(TARGETJARFILE): $(JAVAFILES) $(GENJAVACLASSFILES)
    $(JAVAC) $(JAVACPS) $(MYCLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
    cd $(CLASSDIR) && jar -cvf $(TARGETJARFILE) ConverterBridge.class ConverterBridge$$_ConverterBridge.class com$/sun$/star$/documentconversion$/XConverterBridge.class
    @echo java -classpath $(CLASSPATH) com.sun.star.tools.uno.RegComp $(UNOUCRRDB) register ConverterBridge.jar com.sun.star.loader.Java2
    java -classpath $(CLASSPATH) com.sun.star.tools.uno.RegComp $(UNOUCRRDB) register ConverterBridge.jar com.sun.star.loader.Java2

#$(INTERFACES) : XConverterBridge.idl 
#	idlc $(UNOIDLINC) -O$(OUT)$/ucr$/$(IDLPACKAGE) XConverterBridge.idl 
#	regmerge $(INTERFACES) /UCR $(OUT)$/ucr$/$(IDLPACKAGE)$/XConverterBridge.urd




$(MISC)$/$(SHL1TARGET).def: 
#makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET)														>$@
    @echo EXPORTS																		>>$@
    @echo uno_initEnvironment															>>$@
    @echo uno_ext_getMapping															>>$@
    @echo Java_com_sun_star_comp_bridge_JavaMapper_castIntToObject						>>$@
    @echo Java_com_sun_star_comp_bridge_JavaMapper_castObjectToInt						>>$@
    @echo Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_add			>>$@
    @echo Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_createTicket   >>$@
    @echo Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_dispose		>>$@
    @echo Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_reply			>>$@
    @echo Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_request		>>$@
    @echo Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_retrieve		>>$@
    @echo Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_revoke			>>$@
    @echo Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_stopDispose	>>$@
    @echo Java_com_sun_star_lib_uno_environments_remote_NativeThreadPool_waitOnTicket	>>$@
