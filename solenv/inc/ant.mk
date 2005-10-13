#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: ant.mk,v $
#
#   $Revision: 1.29 $
#
#   last change: $Author: hr $ $Date: 2005-10-13 16:48:31 $
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

# --- Force JDK14 switch  ------------------------------------------

.IF "$(USE_JDK_VERSION)" == "140"
JDK_VERSION=140
JAVA_HOME=$(JDK14PATH)

PATH!:=$(JDK14PATH)$/bin$(PATH_SEPERATOR)$(PATH)
XCLASSPATH:=$(JDK14PATH)$/jre/lib/rt.jar
CLASSPATH:=$(XCLASSPATH)
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

SOLARCOMMONDOCDIR=$(SOLARVERSION)$/common$(PROEXT)$/doc$(EXT_UPDMINOR)
SOLARCOMMONBINDIR=$(SOLARVERSION)$/common$(PROEXT)$/bin$(EXT_UPDMINOR)

# --- ANT build environment  ---------------------------------------

.IF "$(SOLAR_JAVA)"!=""

.IF "$(ANT_HOME)" == ""
ANT_HOME*:=$(COMMON_BUILD_TOOLS)$/apache-ant-1.6.1
.EXPORT : ANT_HOME
.ENDIF
ANT_LIB*:=$(ANT_HOME)$/lib

ANT_CLASSPATH:=$(ANT_LIB)$/xercesImpl.jar$(PATH_SEPERATOR)$(ANT_LIB)$/xml-apis.jar$(PATH_SEPERATOR)$(ANT_LIB)$/ant.jar
PATH!:=$(ANT_HOME)$/bin$(PATH_SEPERATOR)$(PATH)

.IF "$(ANT)" == ""
ANT*:=$(ANT_HOME)$/bin$/ant
.ENDIF

.IF "$(ANT_BUILDFILE)"==""
ANT_BUILDFILE=build.xml
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

.IF "$(JDK)"=="gcj"
JAVA_HOME=
.EXPORT : JAVA_HOME
ANT_FLAGS!:=-Dbuild.compiler=gcj -Dprj=$(PRJ) -Dprjname=$(PRJNAME) -Ddebug=$(ANT_DEBUG) \
 -Doptimize=$(ANT_OPT) -Dtarget=$(TARGET) -Dsolar.update=on -Dout=$(OUT) -Dinpath=$(INPATH) \
 -Dproext="$(PROEXT)" -Dsolar.bin=$(SOLARBINDIR) -Dsolar.jar=$(SOLARBINDIR) \
 -Dsolar.doc=$(SOLARDOCDIR) -Dcommon.jar=$(SOLARCOMMONBINDIR) \
 -Dcommon.doc=$(SOLARCOMMONDOCDIR) -f $(ANT_BUILDFILE) $(ANT_FLAGS) -emacs
.ELSE
ANT_FLAGS!:=-Dprj=$(PRJ) -Dprjname=$(PRJNAME) -Ddebug=$(ANT_DEBUG) -Doptimize=$(ANT_OPT) \
 -Dtarget=$(TARGET) -Dsolar.update=on -Dout=$(OUT) -Dinpath=$(INPATH) -Dproext="$(PROEXT)" \
 -Dsolar.bin=$(SOLARBINDIR) -Dsolar.jar=$(SOLARBINDIR) -Dsolar.doc=$(SOLARDOCDIR) \
 -Dcommon.jar=$(SOLARCOMMONBINDIR) -Dcommon.doc=$(SOLARCOMMONDOCDIR) \
 -f $(ANT_BUILDFILE) $(ANT_FLAGS) -emacs
.ENDIF
.ELSE # No java
ANT=
ANT_FLAGS=
.ENDIF
 
 
.INCLUDE : target.mk

CLASSPATH!:=$(CLASSPATH)$(PATH_SEPERATOR)$(ANT_CLASSPATH)$(PATH_SEPERATOR)$(JAVA_HOME)$/lib$/tools.jar
.EXPORT : CLASSPATH
.EXPORT : PATH

# --- TARGETS -----------------------------------------------------

#$(CLASSDIR)$/solar.properties .PHONY:
$(CLASSDIR)$/solar.properties : $(SOLARENV)$/inc/minor.mk $(SOLARENV)$/inc/ant.properties
    @echo Making: $@
.IF "$(GUI)"=="UNX"
    @+echo solar.build="$(BUILD)" > $@
    @+echo solar.rscversion="$(RSCVERSION)" >> $@
    @+echo solar.rscrevision="$(RSCREVISION)" >> $@
    @+echo solar.minor="$(LAST_MINOR)" >> $@
    @+echo solar.sourceversion="$(SOURCEVERSION)" >> $@
    @+echo solar.udkstamp="$(UDKSTAMP)" >> $@
    @+echo solar.extstamp="$(EXTSTAMP)" >> $@
.ELSE
    @+echo solar.build=$(BUILD) > $@
    @+echo solar.rscversion=$(RSCVERSION) >> $@
    @+echo solar.rscrevision=$(RSCREVISION) >> $@
    @+echo solar.minor=$(LAST_MINOR) >> $@
    @+echo solar.sourceversion=$(SOURCEVERSION) >> $@
    @+echo solar.udkstamp=$(UDKSTAMP) >> $@
    @+echo solar.extstamp=$(EXTSTAMP) >> $@
.ENDIF
    @+cat $(DMAKEROOT)$/../ant.properties >> $@

ANTBUILD .PHONY:
    $(WRAPCMD_ENV) $(ANT) $(ANT_FLAGS)

clean  .PHONY:
    $(WRAPCMD_ENV) $(ANT) $(ANT_FLAGS) $@

prepare .PHONY:
    $(WRAPCMD_ENV) $(ANT) $(ANT_FLAGS) $@

main: .PHONY:
    $(WRAPCMD_ENV) $(ANT) $(ANT_FLAGS) $@

info: .PHONY
    $(WRAPCMD_ENV) $(ANT) $(ANT_FLAGS) $@

jar .PHONY:
     $(WRAPCMD_ENV) $(ANT) $(ANT_FLAGS) $@

compile .PHONY:
    $(WRAPCMD_ENV) $(ANT) $(ANT_FLAGS) $@

depend .PHONY:
    $(WRAPCMD_ENV) $(ANT) $(ANT_FLAGS) $@

javadoc .PHONY:
    $(WRAPCMD_ENV) $(ANT) $(ANT_FLAGS) $@

config .PHONY:
    $(WRAPCMD_ENV) $(ANT) $(ANT_FLAGS) $@

test .PHONY:
    $(WRAPCMD_ENV) $(ANT) $(ANT_FLAGS) $@


