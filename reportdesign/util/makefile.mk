#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.21.2.2 $
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

PRJ=..
PRJNAME=reportdesign
TARGET=rpt
TARGET2=$(TARGET)ui
# USE_DEFFILE=TRUE
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

EXTENSION_VERSION_BASE=1.0.9
.IF "$(PRODUCT)" != ""
    EXTENSION_VERSION=$(EXTENSION_VERSION_BASE)
.ELSE
    EXTENSION_VERSION=$(EXTENSION_VERSION_BASE).$(BUILD)
.ENDIF

# --- Settings ----------------------------------
.INCLUDE :  makefile.pmk

# ------------------------------------------------------------------
# calready set in util$/makefile.pmk
# EXTENSIONNAME:=sun-report-builder
EXTENSION_ZIPNAME:=sun-report-builder

# --- reportdesign core (rpt) -----------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
        $(SLB)$/api.lib				\
        $(SLB)$/coreshared.lib		\
        $(SLB)$/core_resource.lib	\
        $(SLB)$/core_sdr.lib        \
        $(SLB)$/core_misc.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(SVXLIB)				\
        $(FWELIB)				\
        $(SFXLIB)				\
        $(TOOLSLIB) 			\
        $(SVLLIB)				\
        $(SVTOOLLIB)			\
        $(UNOTOOLSLIB)			\
        $(DBTOOLSLIB)			\
        $(COMPHELPERLIB) 		\
        $(CPPUHELPERLIB) 		\
        $(CPPULIB)				\
        $(VCLLIB)				\
        $(TKLIB)				\
        $(VOSLIB)				\
        $(SALLIB)
.IF "$(GUI)"!="WNT" || "$(COM)"=="GCC"
SHL1STDLIBS+= \
        -ldbu$(DLLPOSTFIX)
.ELSE
SHL1STDLIBS+= \
        idbu.lib
.ENDIF

SHL1IMPLIB=i$(TARGET)
SHL1USE_EXPORTS=name
SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)

# --- .res file ----------------------------------------------------------

RES1FILELIST=\
    $(SRS)$/core_strings.srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RES1FILELIST)


# --- reportdesign UI core (rptui) -----------------------------------
LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2FILES=\
        $(SLB)$/uimisc.lib			\
        $(SLB)$/uidlg.lib			\
        $(SLB)$/uishared.lib		\
        $(SLB)$/ui_inspection.lib	\
        $(SLB)$/report.lib

SHL2TARGET=$(TARGET2)$(DLLPOSTFIX)

SHL2STDLIBS= \
        $(SVXLIB)				\
        $(SFXLIB)				\
        $(SVTOOLLIB)			\
        $(TKLIB)				\
        $(VCLLIB)				\
        $(SVLLIB)				\
        $(SOTLIB)				\
        $(UNOTOOLSLIB)			\
        $(TOOLSLIB)				\
        $(DBTOOLSLIB)			\
        $(COMPHELPERLIB)		\
        $(CPPUHELPERLIB)		\
        $(CPPULIB)				\
        $(FWELIB)				\
        $(SO2LIB)				\
        $(I18NISOLANGLIB)		\
        $(SALLIB)
.IF "$(GUI)"!="WNT" || "$(COM)"=="GCC"
SHL2STDLIBS+= \
        -ldbu$(DLLPOSTFIX) \
        -l$(TARGET)$(DLLPOSTFIX)
.ELSE
SHL2STDLIBS+= \
        idbu.lib				\
        i$(TARGET).lib
.ENDIF

.IF "$(GUI)"!="WNT" || "$(COM)"=="GCC"
SHL2STDLIBS+= \
        -lfor$(DLLPOSTFIX) \
        -lforui$(DLLPOSTFIX)
.ELSE
SHL2STDLIBS+= \
        ifor.lib \
        iforui.lib
.ENDIF


SHL2DEPN=$(SHL1TARGETN)
SHL2LIBS=$(LIB2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=$(SHL2TARGET)
SHL2VERSIONMAP=$(TARGET2).map

# --- .res file ----------------------------------------------------------

RES2FILELIST=\
    $(SRS)$/uidlg.srs				\
    $(SRS)$/ui_inspection.srs		\
    $(SRS)$/report.srs


RESLIB2NAME=$(TARGET2)
RESLIB2IMAGES=$(PRJ)$/res
RESLIB2SRSFILES=$(RES2FILELIST)

# ------------------- rptxml -------------------
TARGET3=rptxml
# --- Library -----------------------------------
LIB3TARGET=$(SLB)$/rpt_flt.lib
LIB3FILES=\
        $(SLB)$/xmlshared.lib	\
        $(SLB)$/$(TARGET3).lib

SHL3TARGET=$(TARGET3)$(DLLPOSTFIX)

SHL3STDLIBS=\
    $(SVXLIB)			\
    $(XMLOFFLIB)		\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(DBTOOLSLIB)		\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(UCBHELPERLIB)		\
    $(SFXLIB)			\
    $(SVLLIB)			\
    $(SOTLIB)			\
    $(SO2LIB)			\
    $(SALLIB)
.IF "$(GUI)"!="WNT" || "$(COM)"=="GCC"
SHL3STDLIBS+= \
        -l$(TARGET)$(DLLPOSTFIX)
.ELSE
SHL3STDLIBS+= \
    irpt.lib
.ENDIF


SHL3DEPN=$(SHL1TARGETN)
SHL3LIBS=$(LIB3TARGET)
SHL3IMPLIB=	i$(SHL3TARGET)
SHL3VERSIONMAP=rptui.map
SHL3DEF=	$(MISC)$/$(SHL3TARGET).def

DEF3NAME=$(SHL3TARGET)

# create Extension -----------------------------

.IF "$(SOLAR_JAVA)"!=""

XMLFILES =  $(EXTENSIONDIR)$/META-INF$/manifest.xml

# DESCRIPTION_SRC is the source file which is copied into the extension
# It is defaulted to "description.xml", but we want to pre-process it, so we use an intermediate
# file
DESCRIPTION_SRC = $(MISC)$/description.xml

COMPONENT_MERGED_XCU= \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Setup.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/DataAccess.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/ReportDesign.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/ExtendedColorScheme.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Embedding.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Paths.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Accelerators.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/TypeDetection$/Filter.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/TypeDetection$/Types.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/UI$/ReportCommands.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/UI$/Controller.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/UI$/DbReportWindowState.xcu \
            $(EXTENSIONDIR)$/registry$/schema$/org$/openoffice$/Office$/UI$/DbReportWindowState.xcs \
            $(EXTENSIONDIR)$/registry$/schema$/org$/openoffice$/Office$/UI$/ReportCommands.xcs \
            $(EXTENSIONDIR)$/registry$/schema$/org$/openoffice$/Office$/ReportDesign.xcs

COMPONENT_OTR_FILES= \
    $(EXTENSIONDIR)$/template$/en-US$/wizard$/report$/default.otr
    
COMPONENT_IMAGES= \
    $(EXTENSIONDIR)$/images$/em42.png \
    $(EXTENSIONDIR)$/images$/em42_hc.png

COMPONENT_HTMLFILES = $(EXTENSIONDIR)$/THIRDPARTYREADMELICENSE.html \
            $(EXTENSIONDIR)$/readme_en-US.html \
            $(EXTENSIONDIR)$/readme_en-US.txt

COMPONENT_JARFILES = \
    $(EXTENSIONDIR)$/sun-report-builder.jar

COMPONENT_HELP= \
    $(EXTENSIONDIR)$/help$/component.txt

# .jar files from solver
COMPONENT_EXTJARFILES = \
    $(EXTENSIONDIR)$/sun-report-builder.jar 					\
    $(EXTENSIONDIR)$/reportbuilderwizard.jar
.IF "$(SYSTEM_JFREEREPORT)" != "YES"
COMPONENT_EXTJARFILES += \
    $(EXTENSIONDIR)$/flute-1.3-jfree-20061107.jar				\
    $(EXTENSIONDIR)$/jcommon-1.0.10.jar							\
    $(EXTENSIONDIR)$/jcommon-serializer-0.2.0.jar				\
    $(EXTENSIONDIR)$/libfonts-0.3.3.jar							\
    $(EXTENSIONDIR)$/libformula-0.1.14.jar						\
    $(EXTENSIONDIR)$/liblayout-0.2.8.jar						\
    $(EXTENSIONDIR)$/libloader-0.3.6.jar						\
    $(EXTENSIONDIR)$/librepository-0.1.4.jar					\
    $(EXTENSIONDIR)$/libxml-0.9.9.jar							\
    $(EXTENSIONDIR)$/pentaho-reporting-flow-engine-0.9.2.jar 	\
    $(EXTENSIONDIR)$/sac.jar
.ENDIF

COMPONENT_MANIFEST_GENERIC:=TRUE
COMPONENT_MANIFEST_SEARCHDIR:=registry

# make sure to add your custom files here
EXTENSION_PACKDEPS=$(COMPONENT_EXTJARFILES) $(COMPONENT_HTMLFILES) $(COMPONENT_OTR_FILES) $(COMPONENT_HELP) $(COMPONENT_IMAGES)

# --- Targets ----------------------------------

.INCLUDE : extension_pre.mk
.INCLUDE : target.mk
.INCLUDE : extension_post.mk

$(EXTENSIONDIR)$/%.jar : $(SOLARBINDIR)$/%.jar
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(EXTENSIONDIR)$/readme_en-US.% : $(PRJ)$/license$/readme_en-US.%
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(EXTENSIONDIR)$/images$/%.png : $(PRJ)$/images$/%.png
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(EXTENSIONDIR)$/THIRDPARTYREADMELICENSE.html : $(PRJ)$/license$/THIRDPARTYREADMELICENSE.html
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
    
$(COMPONENT_HELP) : $$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(DESCRIPTION_SRC): description.xml
    +-$(RM) $@
    $(TYPE) description.xml | $(SED) "s/#VERSION#/$(EXTENSION_VERSION)/" > $@

.ELSE			# "$(SOLAR_JAVA)"!=""
.INCLUDE : target.mk
.ENDIF			# "$(SOLAR_JAVA)"!=""

