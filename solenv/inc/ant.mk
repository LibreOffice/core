#*************************************************************************
#
#   $RCSfile: ant.mk,v $
#
#   $Revision: 1.26 $
#
#   last change: $Author: obo $ $Date: 2005-01-25 15:15:02 $
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
ANT_FLAGS!:=-Dprj=$(PRJ) -Dprjname=$(PRJNAME) -Ddebug=$(ANT_DEBUG) -Doptimize=$(ANT_OPT) \
 -Dtarget=$(TARGET) -Dsolar.update=on -Dout=$(OUT) -Dinpath=$(INPATH) -Dproext="$(PROEXT)" \
 -Dsolar.bin=$(SOLARBINDIR) -Dsolar.jar=$(SOLARBINDIR) -Dsolar.doc=$(SOLARDOCDIR) \
 -Dcommon.jar=$(SOLARCOMMONBINDIR) -Dcommon.doc=$(SOLARCOMMONDOCDIR) \
 -f $(ANT_BUILDFILE) $(ANT_FLAGS) -emacs -Dbuild.compiler=gcj
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
    $(WRAPCMD) $(ANT) $(ANT_FLAGS)

clean  .PHONY:
    $(WRAPCMD) $(ANT) $(ANT_FLAGS) $@

prepare .PHONY:
    $(WRAPCMD) $(ANT) $(ANT_FLAGS) $@

main: .PHONY:
    $(WRAPCMD) $(ANT) $(ANT_FLAGS) $@

info: .PHONY
    $(WRAPCMD) $(ANT) $(ANT_FLAGS) $@

jar .PHONY:
     $(WRAPCMD) $(ANT) $(ANT_FLAGS) $@

compile .PHONY:
    $(WRAPCMD) $(ANT) $(ANT_FLAGS) $@

depend .PHONY:
    $(WRAPCMD) $(ANT) $(ANT_FLAGS) $@

javadoc .PHONY:
    $(WRAPCMD) $(ANT) $(ANT_FLAGS) $@

config .PHONY:
    $(WRAPCMD) $(ANT) $(ANT_FLAGS) $@

test .PHONY:
    $(WRAPCMD) $(ANT) $(ANT_FLAGS) $@


