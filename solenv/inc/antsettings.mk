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


.IF "$(SOLAR_JAVA)"!=""

ANT_LIB*:=$(ANT_HOME)/lib

ANT_CLASSPATH:=$(ANT_LIB)/xercesImpl.jar$(PATH_SEPERATOR)$(ANT_LIB)/xml-apis.jar$(PATH_SEPERATOR)$(ANT_LIB)/ant.jar

# PATH_SEPERATOR won't work here as it is defined
# as ; for wondows (all shells)
#PATH!:=$(ANT_HOME)/bin$(PATH_SEPERATOR)$(PATH)
PATH!:=$(ANT_HOME)/bin:$(PATH)

ANT*:=$(ANT_HOME)/bin/ant
ANT_BUILDFILE*=build.xml

.IF "$(ANT_COMPILER_FLAGS)"==""
.IF "$(JAVACISGCJ)" == "yes"
ANT_COMPILER_FLAGS=-Dbuild.compiler=gcj
.ENDIF
.ENDIF

.IF "$(ANT_JAVA_VER_FLAGS)"==""
.IF "$(JDK)" != "gcj" && $(JAVACISKAFFE) != "yes"
ANT_JAVA_VER_FLAGS=-Dant.build.javac.source=$(JAVA_SOURCE_VER) -Dant.build.javac.target=$(JAVA_TARGET_VER)
.ENDIF
.ENDIF

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

.IF "$(VERBOSE)" == "TRUE"
    ANT_VERBOSE=-v
.ELSE
    ANT_VERBOSE=-q
.ENDIF

.IF "$(JDK)"=="gcj"
JAVA_HOME=
.EXPORT : JAVA_HOME
.ENDIF

ANT_FLAGS!:=$(ANT_COMPILER_FLAGS) -Dprj=$(PRJ) -Dprjname=$(PRJNAME) $(ANT_JAVA_VER_FLAGS) \
 -Ddebug=$(ANT_DEBUG) -Doptimize=$(ANT_OPT) -Dtarget=$(TARGET) -Dsolar.update=on \
 -Dout=$(OUT) -Dinpath=$(INPATH) -Dproext="$(PROEXT)" -Dsolar.bin=$(SOLARBINDIR) \
 -Dsolar.jar=$(SOLARBINDIR) -Dsolar.doc=$(SOLARDOCDIR) -Dcommon.jar=$(SOLARCOMMONBINDIR) \
 -Dcommon.doc=$(SOLARCOMMONDOCDIR) -Dsolar.sourceversion=$(SOURCEVERSION) \
 -Dsolar.lastminor=$(LAST_MINOR) -Dsolar.build=$(BUILD) -f $(ANT_BUILDFILE) $(ANT_FLAGS) \
 -emacs $(ANT_VERBOSE)
.ELSE # No java
ANT=
ANT_FLAGS=
.ENDIF

.IF "$(WITH_LANG)"!=""
ANT_FLAGS+=-Dsolar.langs="$(WITH_LANG)" -Dsolar.localized="true"
.ENDIF			# "$(WITH_LANG)"!=""
 
