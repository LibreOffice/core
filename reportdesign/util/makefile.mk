#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: kz $ $Date: 2008-04-04 14:07:45 $
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

PRJ=..
PRJNAME=reportdesign
TARGET=rpt
TARGET2=$(TARGET)ui
# USE_DEFFILE=TRUE
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
# ???
no_common_build_zip=true

# --- Settings ----------------------------------
.INCLUDE :  settings.mk
# ------------------------------------------------------------------

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

ZIP1TARGET=sun-report-builder
ZIP1FLAGS=-r
ZIP1DIR=$(MISC)$/zip
ZIP1LIST=*
ZIP1EXT=.oxt

XMLFILES := $(ZIP1DIR)$/description.xml \
            $(ZIP1DIR)$/META-INF$/manifest.xml
            
XCU_FILES := $(ZIP1DIR)$/registry$/data$/org$/openoffice$/Office$/DataAccess.xcu \
             $(ZIP1DIR)$/registry$/data$/org$/openoffice$/Office$/ReportDesign.xcu

.IF "$(WITH_LANG)"!=""
XCU_TMP := $(MISC)$/merge$/org$/openoffice$/Office$/DataAccess.xcu
.ELSE //"$(WITH_LANG)" != ""
XCU_TMP := $(MISC)$/registry$/data$/org$/openoffice$/Office$/DataAccess.xcu
.ENDIF //"$(WITH_LANG)" != ""

HTMLFILES := $(ZIP1DIR)$/THIRDPARTYREADMELICENSE.html \
            $(ZIP1DIR)$/readme_en-US.html \
            $(ZIP1DIR)$/readme_en-US.txt

.IF "$(GUI)"!="WNT"
TXTFILES:=$(foreach,i,$(alllangiso) $(ZIP1DIR)$/registration$/LICENSE_$i)
LICLINES:=$(foreach,i,$(TXTFILES)  <license-text xlink:href="registration/$(i:f)" lang="$(subst,LICENSE_, $(i:f))" license-id="$(subst,LICENSE_, $(i:f))" />)
.ELSE   # "$(GUI)"!="WNT"
TXTFILES:=$(foreach,i,$(alllangiso) $(ZIP1DIR)$/registration$/license_$i.txt)
LICLINES:=$(foreach,i,$(TXTFILES)  <license-text xlink:href="registration/$(i:f)" lang="$(subst,.txt, $(subst,license_, $(i:f)))" license-id="$(subst,.txt, $(subst,license_, $(i:f)))" />)
.ENDIF  # "$(GUI)"!="WNT"

REPORTJARFILES := \
    $(ZIP1DIR)$/flute-1.3-jfree-20061107.jar							\
    $(ZIP1DIR)$/jcommon-1.0.10.jar										\
    $(ZIP1DIR)$/jcommon-serializer-0.2.0.jar							\
    $(ZIP1DIR)$/libfonts-0.3.3.jar										\
    $(ZIP1DIR)$/libformula-0.1.14.jar									\
    $(ZIP1DIR)$/liblayout-0.2.8.jar										\
    $(ZIP1DIR)$/libloader-0.3.6.jar										\
    $(ZIP1DIR)$/librepository-0.1.4.jar									\
    $(ZIP1DIR)$/libxml-0.9.9.jar										\
    $(ZIP1DIR)$/pentaho-reporting-flow-engine-0.9.2.jar					\
    $(ZIP1DIR)$/sac.jar													\
    $(ZIP1DIR)$/sun-report-builder.jar

# --- Targets ----------------------------------
.INCLUDE : target.mk

.IF "$(ZIP1TARGETN)"!=""
$(ZIP1TARGETN) :  $(TXTFILES) $(XMLFILES) $(HTMLFILES) $(REPORTJARFILES)
.ENDIF          # "$(ZIP1TARGETN)"!="

$(MISC)$/update_report.flag : $(XCU_FILES)
    $(TOUCH) $@

$(ZIP1DIR)$/description.xml : pre.xml post.xml
    @@-$(MKDIRHIER) $(@:d)
    @@-$(RM) $(ZIP1DIR)$/description.xml
    $(TYPE) pre.xml > $@
    $(TYPE) $(mktmp  $(LICLINES)) >> $@
    $(TYPE) post.xml >> $@

$(ZIP1DIR)$/registration$/license_%.txt : $(SOLARBINDIR)$/osl$/license_%.txt
     @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(ZIP1DIR)$/registration$/LICENSE_% : $(SOLARBINDIR)$/osl$/LICENSE_%
     @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(ZIP1DIR)$/%.xml : %.xml
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(ZIP1DIR)$/registry$/data$/org$/openoffice$/Office$/%.xcu : $(MISC)$/registry$/data$/org$/openoffice$/Office$/%.xcu
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(ZIP1DIR)$/merge$/org$/openoffice$/Office$/%.xcu : $(XCU_TMP)
    @@-$(MKDIRHIER) $(@:d)
    echo $(XCU_TMP)
    $(COPY) $< $@

.IF "$(SYSTEM_JFREEREPORT)" == "YES"
$(ZIP1DIR)$/%.jar : $(JFREEREPORT_JAR:d:d)$/%.jar
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
.ELSE
$(ZIP1DIR)$/%.jar : $(SOLARBINDIR)$/%.jar
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
.ENDIF

$(ZIP1DIR)$/%.jar : $(CLASSDIR)$/%.jar
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@    

$(ZIP1DIR)$/META-INF$/manifest.xml : manifest.xml $(MISC)$/update_report.flag
    @@-$(MKDIRHIER) $(@:d)
    $(PERL) $(SOLARENV)$/bin$/makemani.pl $(PRJ)$/util$/manifest.xml $(ZIP1DIR) registry $(@:d:d)

$(ZIP1DIR)$/readme_en-US.% : $(PRJ)$/license$/readme_en-US.%
     @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(ZIP1DIR)$/THIRDPARTYREADMELICENSE.html : $(PRJ)$/license$/THIRDPARTYREADMELICENSE.html
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

.ELSE			# "$(SOLAR_JAVA)"!=""
.INCLUDE : target.mk
.ENDIF			# "$(SOLAR_JAVA)"!=""
#
