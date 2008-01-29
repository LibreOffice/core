#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: antsettings.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: vg $ $Date: 2008-01-29 08:10:51 $
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
 
 
