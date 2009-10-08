#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: antsettings.mk,v $
#
# $Revision: 1.7.166.1 $
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


.IF "$(SOLAR_JAVA)"!=""

.IF "$(ANT_HOME)" == ""
ANT_HOME*:=$(COMMON_BUILD_TOOLS)$/apache-ant-1.7.0
.EXPORT : ANT_HOME
.ENDIF
ANT_LIB*:=$(ANT_HOME)$/lib

ANT_CLASSPATH:=$(ANT_LIB)$/xercesImpl.jar$(PATH_SEPERATOR)$(ANT_LIB)$/xml-apis.jar$(PATH_SEPERATOR)$(ANT_LIB)$/ant.jar
PATH!:=$(ANT_HOME)$/bin$(PATH_SEPERATOR)$(PATH)

ANT*:=$(ANT_HOME)$/bin$/ant
ANT_BUILDFILE*=build.xml

.IF "$(ANT_DEBUG)"==""
.IF "$(debug)"==""
ANT_DEBUG=off
.ELSE
ANT_DEBUG=on
.ENDIF
.ENDIF

.IF "$(ANT_OPT)"==""
.IF "$(optimize)"==""
ANT_OPT=off
.ELSE
ANT_OPT=on
.ENDIF
.ENDIF

.IF "$(JDK)"=="gcj"
JAVA_HOME=
.EXPORT : JAVA_HOME
.ENDIF

.IF "$(JAVACISGCJ)" == "yes"
ANT_FLAGS!:=-Dbuild.compiler=gcj -Dprj=$(PRJ) -Dprjname=$(PRJNAME) -Ddebug=$(ANT_DEBUG) \
 -Doptimize=$(ANT_OPT) -Dtarget=$(TARGET) -Dsolar.update=on -Dout=$(OUT) -Dinpath=$(INPATH) \
 -Dproext="$(PROEXT)" -Dsolar.bin=$(SOLARBINDIR) -Dsolar.jar=$(SOLARBINDIR) \
 -Dsolar.doc=$(SOLARDOCDIR) -Dcommon.jar=$(SOLARCOMMONBINDIR) \
 -Dcommon.doc=$(SOLARCOMMONDOCDIR) -Dsolar.sourceversion=$(SOURCEVERSION) \
 -Dsolar.lastminor=$(LAST_MINOR) -Dsolar.build=$(BUILD) -f $(ANT_BUILDFILE) $(ANT_FLAGS) -emacs
.ELSE
ANT_FLAGS!:=-Dprj=$(PRJ) -Dprjname=$(PRJNAME) -Ddebug=$(ANT_DEBUG) -Doptimize=$(ANT_OPT) \
 -Dtarget=$(TARGET) -Dsolar.update=on -Dout=$(OUT) -Dinpath=$(INPATH) -Dproext="$(PROEXT)" \
 -Dsolar.bin=$(SOLARBINDIR) -Dsolar.jar=$(SOLARBINDIR) -Dsolar.doc=$(SOLARDOCDIR) \
 -Dcommon.jar=$(SOLARCOMMONBINDIR) -Dcommon.doc=$(SOLARCOMMONDOCDIR) \
 -Dsolar.sourceversion=$(SOURCEVERSION) -Dsolar.lastminor=$(LAST_MINOR) \
 -Dsolar.build=$(BUILD) -f $(ANT_BUILDFILE) $(ANT_FLAGS) -emacs
.ENDIF
.ELSE # No java
ANT=
ANT_FLAGS=
.ENDIF

.IF "$(WITH_LANG)"!=""
ANT_FLAGS+=-Dsolar.langs="$(WITH_LANG)" -Dsolar.localized="true"
.ENDIF			# "$(WITH_LANG)"!=""
 
