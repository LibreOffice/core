#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: ant.mk,v $
#
#   $Revision: 1.35 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:30:01 $
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


# --- ANT build environment  ---------------------------------------

.INCLUDE : antsettings.mk

.INCLUDE : target.mk

CLASSPATH!:=$(CLASSPATH)$(PATH_SEPERATOR)$(ANT_CLASSPATH)$(PATH_SEPERATOR)$(JAVA_HOME)$/lib$/tools.jar
.EXPORT : CLASSPATH
.EXPORT : PATH

# --- TARGETS -----------------------------------------------------

$(CLASSDIR)$/solar.properties : $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/minormkchanged.flg $(SOLARENV)$/inc/ant.properties
    @echo Making: $@
    @echo solar.build=$(BUILD) > $@
    @echo solar.rscversion=$(USQ)$(RSCVERSION)$(USQ) >> $@
    @echo solar.rscrevision=$(USQ)$(RSCREVISION)$(USQ) >> $@
    @echo solar.minor=$(LAST_MINOR) >> $@
    @echo solar.sourceversion=$(SOURCEVERSION) >> $@
    @echo solar.udkstamp=$(UDKSTAMP) >> $@
    @echo solar.extstamp=$(EXTSTAMP) >> $@
    @cat $(DMAKEROOT)$/../ant.properties >> $@

ANTBUILD .PHONY:
    $(ANT) $(ANT_FLAGS)

clean  .PHONY:
    $(ANT) $(ANT_FLAGS) $@

prepare .PHONY:
    $(ANT) $(ANT_FLAGS) $@

main: .PHONY:
    $(ANT) $(ANT_FLAGS) $@

info: .PHONY
    $(ANT) $(ANT_FLAGS) $@

jar .PHONY:
    $(ANT) $(ANT_FLAGS) $@

compile .PHONY:
    $(ANT) $(ANT_FLAGS) $@

depend .PHONY:
    $(ANT) $(ANT_FLAGS) $@

javadoc .PHONY:
    $(ANT) $(ANT_FLAGS) $@

config .PHONY:
    $(ANT) $(ANT_FLAGS) $@

test .PHONY:
    $(ANT) $(ANT_FLAGS) $@


