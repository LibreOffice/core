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

PRJ=..$/..

PRJNAME=testtools
TARGET=bridgetest
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""
DLLPRE = # no leading "lib" on .so files

.IF "$(GUI)"=="WNT"
BATCH_SUFFIX=.bat
GIVE_EXEC_RIGHTS=@echo
MY_URE_INTERNAL_JAVA_DIR=$(strip $(subst,\,/ file:///$(shell @$(WRAPCMD) echo $(SOLARBINDIR))))
MY_LOCAL_CLASSDIR=$(strip $(subst,\,/ file:///$(shell $(WRAPCMD) echo $(PWD)/$(CLASSDIR)/)))
.ELIF "$(GUI)"=="OS2"
BATCH_SUFFIX=.cmd
GIVE_EXEC_RIGHTS=@echo
MY_URE_INTERNAL_JAVA_DIR=$(strip $(subst,\,/ file:///$(shell @$(WRAPCMD) echo $(SOLARBINDIR))))
MY_LOCAL_CLASSDIR=$(strip $(subst,\,/ file:///$(shell $(WRAPCMD) echo $(PWD)/$(CLASSDIR)/)))
.ELSE
GIVE_EXEC_RIGHTS=chmod +x 
MY_URE_INTERNAL_JAVA_DIR=file://$(SOLARBINDIR)
MY_LOCAL_CLASSDIR=file://$(PWD)/$(CLASSDIR)/
.ENDIF

my_components = bridgetest constructors cppobj
.IF "$(SOLAR_JAVA)" != ""
my_components += testComponent
.END

.IF "$(GUI)"=="WNT"
.IF "$(compcheck)" != ""
CFLAGSCXX += -DCOMPCHECK
.ENDIF
.ENDIF

SLOFILES = \
    $(SLO)$/bridgetest.obj \
    $(SLO)$/cppobj.obj \
    $(SLO)$/currentcontextchecker.obj \
    $(SLO)$/multi.obj

# ---- test ----

LIB1TARGET=$(SLB)$/cppobj.lib
LIB1OBJFILES= \
        $(SLO)$/cppobj.obj $(SLO)$/currentcontextchecker.obj $(SLO)$/multi.obj

SHL1TARGET = cppobj.uno
SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
SHL1VERSIONMAP = $(SOLARENV)/src/component.map

# ---- test object ----

LIB2TARGET=$(SLB)$/bridgetest.lib
LIB2OBJFILES= \
        $(SLO)$/bridgetest.obj \
        $(SLO)$/currentcontextchecker.obj \
        $(SLO)$/multi.obj

SHL2TARGET = bridgetest.uno
SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL2LIBS=	$(LIB2TARGET)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)
SHL2VERSIONMAP = $(SOLARENV)/src/component.map

SHL3TARGET = constructors.uno
SHL3OBJS = $(SLO)$/constructors.obj
SHL3STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB)
SHL3VERSIONMAP = $(SOLARENV)/src/component.map
SHL3IMPLIB = i$(SHL3TARGET)
DEF3NAME = $(SHL3TARGET)

.IF "$(SOLAR_JAVA)" != ""
JARFILES = java_uno.jar jurt.jar ridl.jar
JAVATARGETS=\
    $(DLLDEST)$/bridgetest_javaserver$(BATCH_SUFFIX) \
    $(DLLDEST)$/bridgetest_inprocess_java$(BATCH_SUFFIX)
.ENDIF

# --- Targets ------------------------------------------------------
.ENDIF # L10N_framework

.INCLUDE :	target.mk
.IF "$(L10N_framework)"==""
ALLTAR: \
        runtest \
        $(DLLDEST)/services.rdb \
        $(DLLDEST)$/uno_types.rdb \
        $(DLLDEST)$/uno_services.rdb \
        $(DLLDEST)$/bridgetest_server$(BATCH_SUFFIX) \
        $(DLLDEST)$/bridgetest_client$(BATCH_SUFFIX) \
        $(JAVATARGETS)

#################################################################

runtest : $(DLLDEST)$/uno_types.rdb $(DLLDEST)$/uno_services.rdb makefile.mk \
        $(SHL1TARGETN) $(SHL2TARGETN) $(SHL3TARGETN)
.IF "$(COM)$(OS)$(CPU)" == "GCCMACOSXP"
    @echo "Mac OSX PPC GCC fails this test!, likely broken UNO bridge. Fix me."
.ELSE
        cd $(DLLDEST) && $(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/uno \
        -ro uno_services.rdb -ro uno_types.rdb \
        -s com.sun.star.test.bridge.BridgeTest -- \
        com.sun.star.test.bridge.CppTestObject
.ENDIF

$(DLLDEST)/services.rdb :
    $(COPY) $(SOLARXMLDIR)/ure/services.rdb $@

$(DLLDEST)$/uno_types.rdb : $(SOLARBINDIR)$/udkapi.rdb
    echo $(DLLDEST)
    $(GNUCOPY) $(SOLARBINDIR)$/udkapi.rdb $@
    $(REGMERGE) $@ / $(BIN)$/bridgetest.rdb

$(DLLDEST)$/bridgetest_client$(BATCH_SUFFIX) .ERRREMOVE: makefile.mk
    echo '$(AUGMENT_LIBRARY_PATH)' '$(SOLARBINDIR)'/uno -ro services.rdb \
        -ro uno_services.rdb -ro uno_types.rdb \
        -s com.sun.star.test.bridge.BridgeTest -- \
        -u \''uno:socket,host=127.0.0.1,port=2002;urp;test'\' > $@
    $(GIVE_EXEC_RIGHTS) $@

$(DLLDEST)$/bridgetest_server$(BATCH_SUFFIX) .ERRREMOVE: makefile.mk
    echo '$(AUGMENT_LIBRARY_PATH)' '$(SOLARBINDIR)'/uno -ro uno_services.rdb -ro uno_types.rdb \
        -s com.sun.star.test.bridge.CppTestObject \
        -u \''uno:socket,host=127.0.0.1,port=2002;urp;test'\' --singleaccept \
        > $@
    $(GIVE_EXEC_RIGHTS) $@


.IF "$(SOLAR_JAVA)" != ""
# jar-files, which regcomp needs so that it can use java
MY_JARS=java_uno.jar ridl.jar jurt.jar juh.jar

# CLASSPATH, which regcomp needs to be run
MY_CLASSPATH_TMP=$(foreach,i,$(MY_JARS) $(SOLARBINDIR)$/$i)$(PATH_SEPERATOR)$(XCLASSPATH)
MY_CLASSPATH=$(strip $(subst,!,$(PATH_SEPERATOR) $(MY_CLASSPATH_TMP:s/ /!/)))$(PATH_SEPERATOR)..$/class

# Use "127.0.0.1" instead of "localhost", see #i32281#:
$(DLLDEST)$/bridgetest_javaserver$(BATCH_SUFFIX) : makefile.mk
    -rm -f $@
    echo java -classpath "$(MY_CLASSPATH)$(PATH_SEPERATOR)..$/class$/testComponent.jar" \
        com.sun.star.comp.bridge.TestComponentMain \""uno:socket,host=127.0.0.1,port=2002;urp;test"\" singleaccept > $@
    $(GIVE_EXEC_RIGHTS) $@

$(DLLDEST)$/bridgetest_inprocess_java$(BATCH_SUFFIX) .ERRREMOVE: makefile.mk
    echo '$(AUGMENT_LIBRARY_PATH)' '$(SOLARBINDIR)'/uno -ro services.rdb \
        -ro uno_services.rdb -ro uno_types.rdb \
        -s com.sun.star.test.bridge.BridgeTest \
        -env:URE_INTERNAL_JAVA_DIR=$(MY_URE_INTERNAL_JAVA_DIR) \
        -env:MY_CLASSDIR_URL=$(MY_LOCAL_CLASSDIR) \
        -- com.sun.star.test.bridge.JavaTestObject noCurrentContext > $@
    $(GIVE_EXEC_RIGHTS) $@
.ENDIF

$(DLLDEST)$/uno_services.rdb .ERRREMOVE : $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/uno_services.input $(my_components:^"$(MISC)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(PWD)/$(MISC)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/uno_services.input

$(MISC)/uno_services.input :
    echo \
        '<list>$(my_components:^"<filename>":+".component</filename>")</list>' \
        > $@

$(MISC)/bridgetest.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        bridgetest.component
    $(XSLTPROC) --nonet --stringparam uri './$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt bridgetest.component

$(MISC)/constructors.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt constructors.component
    $(XSLTPROC) --nonet --stringparam uri './$(SHL3TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt constructors.component

$(MISC)/cppobj.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        cppobj.component
    $(XSLTPROC) --nonet --stringparam uri './$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt cppobj.component

$(MISC)/testComponent.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt testComponent.component
    $(XSLTPROC) --nonet --stringparam uri \
        'vnd.sun.star.expand:$${{MY_CLASSDIR_URL}}testComponent.jar' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt testComponent.component

.ENDIF # L10N_framework
