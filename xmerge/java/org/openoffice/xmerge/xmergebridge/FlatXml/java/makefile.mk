#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: aidan $ $Date: 2002-07-17 15:03:25 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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


NEWCLASS:=$(foreach,i,$(JARFILES) $(null,$(shell $(FIND) $(JARDIR) -name $i) $(SOLARBINDIR)$/$i $(shell $(FIND) $(JARDIR) -name $i)))

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
    +cd $(CLASSDIR) && jar -cvf $(TARGETJARFILE) ConverterBridge.class ConverterBridge$$_ConverterBridge.class com$/sun$/star$/documentconversion$/XConverterBridge.class
    @echo +java -classpath $(CLASSPATH) com.sun.star.tools.uno.RegComp $(UNOUCRRDB) register ConverterBridge.jar com.sun.star.loader.Java2
    +java -classpath $(CLASSPATH) com.sun.star.tools.uno.RegComp $(UNOUCRRDB) register ConverterBridge.jar com.sun.star.loader.Java2

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
