#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: rt $ $Date: 2004-07-23 14:47:41 $
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

PRJ=..$/..

PRJNAME=testtools
TARGET=bridgetest
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files

.IF "$(GUI)"=="WNT"
BATCH_SUFFIX=.bat
GIVE_EXEC_RIGHTS=@echo
.ELSE
BATCH_INPROCESS=bridgetest_inprocess
GIVE_EXEC_RIGHTS=chmod +x 
.ENDIF

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb 
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

UNOUCROUT=$(OUT)$/inc
INCPRE+=$(OUT)$/inc

UNOTYPES= \
        com.sun.star.uno.TypeClass		\
        com.sun.star.uno.XAggregation		\
        com.sun.star.uno.XWeak			\
        com.sun.star.lang.XTypeProvider		\
        com.sun.star.lang.XServiceInfo		\
        com.sun.star.lang.XSingleServiceFactory	\
        com.sun.star.lang.XMultiServiceFactory	\
        com.sun.star.lang.XComponent		\
        com.sun.star.lang.XMain			\
        com.sun.star.loader.XImplementationLoader \
        com.sun.star.registry.XRegistryKey	\
        com.sun.star.container.XSet		\
        com.sun.star.bridge.XUnoUrlResolver		\
            com.sun.star.lang.XSingleComponentFactory	\
        com.sun.star.uno.XComponentContext          

SLOFILES= \
        $(SLO)$/cppobj.obj	\
        $(SLO)$/bridgetest.obj

# ---- test ----

LIB1TARGET=$(SLB)$/cppobj.lib
LIB1OBJFILES= \
        $(SLO)$/cppobj.obj

SHL1TARGET = cppobj.uno
SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
SHL1VERSIONMAP=component.map

# ---- test object ----

LIB2TARGET=$(SLB)$/bridgetest.lib
LIB2OBJFILES= \
        $(SLO)$/bridgetest.obj

SHL2TARGET = bridgetest.uno
SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL2LIBS=	$(LIB2TARGET)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)
SHL2VERSIONMAP=component.map



.IF "$(SOLAR_JAVA)" != ""
JARFILES = java_uno.jar jurt.jar ridl.jar
JAVATARGETS=\
    $(DLLDEST)$/bridgetest_javaserver$(BATCH_SUFFIX) \
    $(DLLDEST)$/bridgetest_inprocess_java$(BATCH_SUFFIX)
.ENDIF

# --- Targets ------------------------------------------------------
ALL : \
        ALLTAR \
        $(DLLDEST)$/uno_types.rdb \
        $(DLLDEST)$/uno_services.rdb \
        $(DLLDEST)$/bridgetest_inprocess$(BATCH_SUFFIX) \
        $(DLLDEST)$/bridgetest_server$(BATCH_SUFFIX) \
        $(DLLDEST)$/bridgetest_client$(BATCH_SUFFIX) \
        $(JAVATARGETS)

.INCLUDE :	target.mk

#################################################################

$(DLLDEST)$/uno_types.rdb : $(SOLARBINDIR)$/udkapi.rdb
    echo $(DLLDEST)
    $(GNUCOPY) -p $? $@
    $(REGMERGE) $@ / $(BIN)$/bridgetest.rdb

$(DLLDEST)$/bridgetest_inprocess$(BATCH_SUFFIX) : bridgetest_inprocess
    $(GNUCOPY) -p $? $@
    $(GIVE_EXEC_RIGHTS) $@

$(DLLDEST)$/bridgetest_client$(BATCH_SUFFIX) : bridgetest_client
    $(GNUCOPY) -p $? $@
    $(GIVE_EXEC_RIGHTS) $@

$(DLLDEST)$/bridgetest_server$(BATCH_SUFFIX) : bridgetest_server
    $(GNUCOPY) -p $? $@
    $(GIVE_EXEC_RIGHTS) $@


.IF "$(SOLAR_JAVA)" != ""
NULLSTR=
# jar-files, which regcomp needs so that it can use java
MY_JARS=java_uno.jar ridl.jar jurt.jar juh.jar

# CLASSPATH, which regcomp needs to be run
MY_CLASSPATH_TMP=$(foreach,i,$(MY_JARS) $(SOLARBINDIR)$/$i)$(PATH_SEPERATOR)$(XCLASSPATH)
MY_CLASSPATH=$(strip $(subst,!,$(PATH_SEPERATOR) $(MY_CLASSPATH_TMP:s/ /!/)))$(PATH_SEPERATOR)..$/class

$(DLLDEST)$/bridgetest_javaserver$(BATCH_SUFFIX) : makefile.mk
    -rm -f $@
    +echo java -classpath $(MY_CLASSPATH)$(PATH_SEPERATOR)..$/class$/testComponent.jar \
        com.sun.star.comp.bridge.TestComponentMain \
        \""uno:socket,host=localhost,port=2002;urp;test"\" \
        > $@
    $(GIVE_EXEC_RIGHTS) $@

$(DLLDEST)$/bridgetest_inprocess_java$(BATCH_SUFFIX) : makefile.mk
    -rm -f $@
.IF "$(GUI)"=="WNT"
    +echo set CLASSPATH=$(MY_CLASSPATH) >> $@
.ELSE
    +echo "CLASSPATH=$(MY_CLASSPATH); export CLASSPATH" >> $@
.ENDIF
    +echo uno -ro uno_services.rdb -ro uno_types.rdb \
       -s com.sun.star.test.bridge.BridgeTest -- com.sun.star.test.bridge.JavaTestObject >> $@
    $(GIVE_EXEC_RIGHTS) $@
.ENDIF

.IF "$(GUI)" == "WNT"
FILEURLPREFIX = file:///
.ELSE
FILEURLPREFIX = file://
.ENDIF

$(DLLDEST)$/uno_services.rdb .ERRREMOVE: $(DLLDEST)$/uno_types.rdb \
        $(DLLDEST)$/bridgetest.uno$(DLLPOST) $(DLLDEST)$/cppobj.uno$(DLLPOST) \
        $(MISC)$/$(TARGET)$/bootstrap.rdb
    - $(MKDIR) $(@:d)
    $(REGCOMP) -register -br $(DLLDEST)$/uno_types.rdb -r $@ \
        -c acceptor.uno$(DLLPOST) \
        -c bridgefac.uno$(DLLPOST) \
        -c connector.uno$(DLLPOST) \
        -c remotebridge.uno$(DLLPOST) \
        -c uuresolver.uno$(DLLPOST) \
        -c \
    $(subst,$/,/ $(FILEURLPREFIX)$(PWD)$/$(DLLDEST)$/bridgetest.uno$(DLLPOST)) \
        -c \
        $(subst,$/,/ $(FILEURLPREFIX)$(PWD)$/$(DLLDEST)$/cppobj.uno$(DLLPOST))
.IF "$(SOLAR_JAVA)" != ""
    $(REGCOMP) -register -br $(DLLDEST)$/uno_types.rdb -r $@ \
        -c javaloader.uno$(DLLPOST) -c javavm.uno$(DLLPOST)
    $(REGCOMP) -register  -br $(MISC)$/$(TARGET)$/bootstrap.rdb -r $@ -c \
        $(subst,$/,/ $(FILEURLPREFIX)$(PWD)$/$(CLASSDIR)$/testComponent.jar) \
        -classpath $(CLASSPATH)
.ENDIF

$(MISC)$/$(TARGET)$/bootstrap.rdb .ERRREMOVE:
    - $(MKDIR) $(@:d)
    + $(COPY) $(SOLARBINDIR)$/types.rdb $@
.IF "$(SOLAR_JAVA)" != ""
    $(REGCOMP) -register -r $@ -c javaloader.uno$(DLLPOST) \
        -c javavm.uno$(DLLPOST)
.ENDIF
