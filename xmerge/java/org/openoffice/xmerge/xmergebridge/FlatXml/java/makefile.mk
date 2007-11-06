#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 16:03:27 $
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

PRJ	= ..$/..$/..
PRJNAME = odk
TARGET  = examples_java_converterbridge

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

JARFILES 		= sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar ridl.jar
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
