#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2004-07-23 14:52:18 $
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
#   Copyright: 2003 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#*************************************************************************

PRJ := ..$/..$/..
PRJNAME := bridges

TARGET := test_javauno_nativethreadpool
PACKAGE := test$/javauno$/nativethreadpool

ENABLE_EXCEPTIONS := TRUE

#TODO:
.IF "$(OS)" == "LINUX"
JVM_LIB_URL := file:///net/grande.germany/develop6/update/dev/Linux_JDK_1.4.1_03/jre/lib/i386/client/libjvm.so
.ELSE
ERROR -- missing platform
.ENDIF

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files
INCPRE += $(MISC)$/$(TARGET)$/inc

SHL1TARGET = $(TARGET)_client.uno
SHL1OBJS = $(SLO)$/testnativethreadpoolclient.obj
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(TARGET)_client

SHL2TARGET = $(TARGET)_server.uno
SHL2OBJS = $(SLO)$/testnativethreadpoolserver.obj
SHL2STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL2VERSIONMAP = version.map
SHL2IMPLIB = i$(TARGET)_server

SLOFILES = $(SHL1OBJS) $(SHL2OBJS)

JAVAFILES = Relay.java
JARFILES = java_uno.jar juh.jar jurt.jar ridl.jar

.INCLUDE: target.mk

ALLTAR: test

EXEC_CLASSPATH_TMP = $(foreach,i,$(JARFILES) $(SOLARBINDIR)$/$i)
EXEC_CLASSPATH = \
    $(strip $(subst,!,$(PATH_SEPERATOR) $(EXEC_CLASSPATH_TMP:s/ /!/)))

$(MISC)$/$(TARGET)$/types.rdb: types.idl
    - rm $@
    - $(MKDIR) $(MISC)$/$(TARGET)
    - $(MKDIR) $(MISC)$/$(TARGET)$/inc
    idlc -I$(SOLARIDLDIR) -O$(MISC)$/$(TARGET) $<
    regmerge $@ /UCR $(MISC)$/$(TARGET)$/types.urd
    cppumaker -BUCR -C -O$(MISC)$/$(TARGET)$/inc $@ -X$(SOLARBINDIR)$/types.rdb
    javamaker -BUCR -nD -O$(CLASSDIR) $@ -X$(SOLARBINDIR)$/types.rdb

$(SLOFILES) $(JAVACLASSFILES): $(MISC)$/$(TARGET)$/types.rdb

$(BIN)$/$(TARGET).uno.jar: $(JAVACLASSFILES) relay.manifest
    jar cfm $@ relay.manifest -C $(CLASSDIR) test/javauno/nativethreadpool

$(BIN)$/$(TARGET).rdb .ERRREMOVE: $(MISC)$/$(TARGET)$/types.rdb \
        $(BIN)$/$(TARGET).uno.jar
    cp $(MISC)$/$(TARGET)$/types.rdb $@
    regmerge $@ / $(SOLARBINDIR)$/types.rdb
    regcomp -register -r $@ -c acceptor.uno$(DLLPOST) \
        -c bridgefac.uno$(DLLPOST) -c connector.uno$(DLLPOST) \
        -c remotebridge.uno$(DLLPOST) -c uuresolver.uno$(DLLPOST) \
        -c javaloader.uno$(DLLPOST) -c javavm.uno$(DLLPOST)
    cp $(SOLARBINDIR)$/types.rdb $(MISC)$/$(TARGET)$/bootstrap.rdb
    regcomp -register -r $(MISC)$/$(TARGET)$/bootstrap.rdb \
        -c javaloader.uno$(DLLPOST) -c javavm.uno$(DLLPOST)
.IF "$(GUI)" == "WNT"
    ERROR -- missing platform
.ELSE # GUI, WNT
    + setenv OO_JAVA_PROPERTIES RuntimeLib=$(JVM_LIB_URL) ; \
        regcomp -register -r $@ -c file://$(PWD)/$(BIN)$/$(TARGET).uno.jar \
        -br $(MISC)$/$(TARGET)$/bootstrap.rdb -classpath $(EXEC_CLASSPATH)
.ENDIF # GUI, WNT

test .PHONY: $(SHL1TARGETN) $(BIN)$/$(TARGET).uno.jar $(BIN)$/$(TARGET).rdb
.IF "$(GUI)" == "WNT"
    ERROR -- missing platform
.ELSE # GUI, WNT
    uno -c test.javauno.nativethreadpool.server -l $(SHL2TARGETN) \
        -ro $(BIN)$/$(TARGET).rdb \
        -u 'uno:socket,host=localhost,port=3830;urp;test' --singleaccept &
    + setenv OO_JAVA_PROPERTIES RuntimeLib=$(JVM_LIB_URL) ; \
        setenv CLASSPATH \
        $(EXEC_CLASSPATH)$(PATH_SEPERATOR)$(BIN)$/$(TARGET).uno.jar ; \
        uno -c test.javauno.nativethreadpool.client -l $(SHL1TARGETN) \
        -ro $(BIN)$/$(TARGET).rdb
.ENDIF # GUI, WNT
