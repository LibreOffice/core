#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
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
PRJ=..
PRJNAME=instsetoo_native
TARGET=util
GEN_HID2=TRUE

.INCLUDE:  settings.mk

.IF "$(GUI)"=="WNT"
.IF "$(USE_SHELL)"=="4nt"
PYTHONPATH:=$(PWD)$/$(BIN);$(SOLARLIBDIR);$(SOLARLIBDIR)$/python;$(SOLARLIBDIR)$/python$/lib-dynload
.ELSE			# "$(USE_SHELL)"=="4nt"
PYTHONPATH:=$(shell $(WRAPCMD) echo $(PWD)$/$(BIN):$(SOLARLIBDIR):$(SOLARLIBDIR)$/python:$(SOLARLIBDIR)$/python$/lib-dynload)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ELSE			# "$(GUI)"=="WNT"
PYTHONPATH:=$(PWD)$/$(BIN):$(SOLARLIBDIR):$(SOLARLIBDIR)$/python:$(SOLARLIBDIR)$/python$/lib-dynload
.ENDIF			# "$(GUI)"=="WNT"
.EXPORT: PYTHONPATH

.EXPORT: LAST_MINOR
.EXPORT: PRJ
.EXPORT: PRJNAME

.IF "$(OS)"=="LINUX"
DONT_REMOVE_PACKAGE:=TRUE
.EXPORT: DONT_REMOVE_PACKAGE
.ENDIF          # "$(OS)"=="LINUX"	

.INCLUDE .IGNORE: $(SRC_ROOT)$/cwsname.mk

SHARED_COM_SDK_PATH*:=.

INSTALLDIR=$(OUT)

.INCLUDE: target.mk

LOCALPYFILES= \
    $(BIN)$/uno.py \
    $(BIN)$/unohelper.py \
    $(BIN)$/pythonloader.py \
    $(BIN)$/pythonscript.py

# PKGFORMAT taken from environment. See possible
# values below.
#
# epm supports the following formats:
# aix - AIX software distribution
# bsd - FreeBSD, NetBSD, or OpenBSD software distribution
# depot or swinstall - HP-UX software distribution
# deb - Debian software distribution
# inst or tardist - IRIX software distribution
# osx - MacOS X software distribution
# pkg - Solaris software distribution
# rpm - RedHat software distribution
# setld - Tru64 (setld) software distribution
# native - "Native" software distribution for the platform
# portable - Portable software distribution

.IF "$(GUI)"!="WNT" && "$(EPM)"=="NO" && "$(USE_PACKAGER)"==""
ALLTAR : $(LOCALPYFILES)
    @echo "No EPM: do no packaging at this stage"
.ELSE			# "$(GUI)"!="WNT" && "$(EPM)"=="NO" && "$(USE_PACKAGER)"==""
.IF "$(UPDATER)"=="" || "$(USE_PACKAGER)"==""
ALLTAR : openoffice ure_en-US
.ELSE			# "$(UPDATER)"=="" || "$(USE_PACKAGER)"==""
ALLTAR : updatepack
.ENDIF			# "$(UPDATER)"=="" || "$(USE_PACKAGER)"==""
.ENDIF			# "$(GUI)"!="WNT" && "$(EPM)"=="NO" && "$(USE_PACKAGER)"==""

.IF "$(MAKETARGETS:e)"!=""
PKGFORMAT+=$(MAKETARGETS:e:s/.//)
.ENDIF			# "$(MAKETARGETS:e)"!=""

.IF "$(PKGFORMAT)"!=""
PKGFORMATSWITCH=-format xxx
.ENDIF			# "$(PKGFORMAT)"!=""

updatepack:
    +$(PERL) -w $(SOLARENV)$/bin$/packager.pl

.IF "$(alllangiso)"!=""

openoffice: $(foreach,i,$(alllangiso) openoffice_$i)

openofficewithjre: $(foreach,i,$(alllangiso) openofficewithjre_$i)

ooolanguagepack : $(foreach,i,$(alllangiso) ooolanguagepack_$i)

MSIOFFICETEMPLATESOURCE=$(PRJ)$/inc_openoffice$/windows$/msi_templates
MSILANGPACKTEMPLATESOURCE=$(PRJ)$/inc_ooolangpack$/windows$/msi_templates
.IF "$(BUILD_SPECIAL)"!=""
MSIOFFICETEMPLATEDIR=$(MSIOFFICETEMPLATESOURCE)
MSILANGPACKTEMPLATEDIR=$(MSILANGPACKTEMPLATESOURCE)
.ELSE			# "$(BUILD_SPECIAL)"!=""
NOLOGOSPLASH:=$(BIN)$/intro.bmp
MSIOFFICETEMPLATEDIR=$(MISC)$/openoffice$/msi_templates
MSILANGPACKTEMPLATEDIR=$(MISC)$/ooolangpack$/msi_templates
ADDDEPS=$(NOLOGOSPLASH) hack_msitemplates

$(foreach,i,$(alllangiso) openoffice_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) openofficewithjre_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) ooolanguagepack_$i) : $(ADDDEPS)

.IF "$(MAKETARGETS)"!=""
$(MAKETARGETS) : $(ADDDEPS)
.ENDIF			# "$(MAKETARGETS)"!=""
.ENDIF			# "$(BUILD_SPECIAL)"!=""

.IF "$(PKGFORMAT)"!=""
$(foreach,i,$(alllangiso) openoffice_$i) : $$@{$(PKGFORMAT:^".")}
.IF "$(MAKETARGETS)"!="" && "$(PKGFORMAT)"!=""
.IF "$(MAKETARGETS:e)"=="" && "$(MAKETARGETS:s/_//)"!="$(MAKETARGETS)"
$(MAKETARGETS) : $$@{$(PKGFORMAT:^".")}
$(MAKETARGETS){$(PKGFORMAT:^".")} : $(ADDDEPS)
.ENDIF			# "$(MAKETARGETS:e)"=="" && "$(MAKETARGETS:s/_//)"!="$(MAKETARGETS)"
.ENDIF			# "$(MAKETARGETS)"!="" && "$(PKGFORMAT)"!=""
openoffice_%{$(PKGFORMAT:^".")} :
.ELSE			# "$(PKGFORMAT)"!=""
openoffice_% :
.ENDIF			# "$(PKGFORMAT)"!=""
    +$(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p OpenOffice -packagelist $(PRJ)$/inc_openoffice$/unix$/packagelist.txt -u $(OUT) -buildid $(BUILD) -msitemplate $(MSIOFFICETEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -addsystemintegration $(subst,xxx,$(@:e:s/.//) $(PKGFORMATSWITCH))

.IF "$(PKGFORMAT)"!=""
$(foreach,i,$(alllangiso) openofficewithjre_$i) : $$@{$(PKGFORMAT:^".")}
openofficewithjre_%{$(PKGFORMAT:^".")} :
.ELSE			# "$(PKGFORMAT)"!=""
openofficewithjre_% :
.ENDIF			# "$(PKGFORMAT)"!=""
    +$(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p OpenOffice_wJRE -packagelist $(PRJ)$/inc_openoffice$/unix$/packagelist.txt -u $(OUT) -buildid $(BUILD) -msitemplate $(MSIOFFICETEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -addchildprojects -addsystemintegration $(subst,xxx,$(@:e:s/.//) $(PKGFORMATSWITCH))

.IF "$(PKGFORMAT)"!=""
$(foreach,i,$(alllangiso) ooolanguagepack_$i) : $$@{$(PKGFORMAT:^".")}
ooolanguagepack_%{$(PKGFORMAT:^".")} :
.ELSE			# "$(PKGFORMAT)"!=""
ooolanguagepack_% :
.ENDIF			# "$(PKGFORMAT)"!=""
    +$(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p OpenOffice -packagelist $(PRJ)$/inc_openoffice$/unix$/packagelist_language.txt -u $(OUT) -buildid $(BUILD) -msitemplate $(MSILANGPACKTEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -languagepack $(subst,xxx,$(@:e:s/.//) $(PKGFORMATSWITCH))

.ELSE			# "$(alllangiso)"!=""
openoffice:
    @+echo cannot pack nothing...

.ENDIF			# "$(alllangiso)"!=""

.IF "$(PKGFORMAT)"!=""
ure_en-US: ure_en-US{$(PKGFORMAT:^".")}
ure_en-US{$(PKGFORMAT:^".")} : $(MISC)$/ure$/services.rdb
.ELSE			# "$(PKGFORMAT)"!=""
ure_en-US: $(MISC)$/ure$/services.rdb
.ENDIF			# "$(PKGFORMAT)"!=""
    +$(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst \
        -l en-US -p URE -u $(OUT) -buildid $(BUILD) $(subst,xxx,$(@:e:s/.//) $(PKGFORMATSWITCH)) \
        -packagelist $(PRJ)$/inc_ure$/unix$/packagelist.txt \
        -msitemplate $(PRJ)$/inc_ure$/windows$/msi_templates \
        -msilanguage $(COMMONMISC)$/win_ulffiles

.IF "$(USE_SHELL)"!="4nt"
MY_Q = '
.ELSE
MY_Q =
.ENDIF
.IF "$(OS)" == "WNT"
MY_URL = $(strip $(subst,\,/ file:///$(shell +$(WRAPCMD) echo $(SOLARBINDIR))))
.ELSE
MY_URL = file://$(SOLARLIBDIR)
.ENDIF
$(MISC)$/ure$/services.rdb .ERRREMOVE:
    - $(MKDIR) $(@:d)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/acceptor.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/bridgefac.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/connector.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/implreg.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/introspection.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/invocadapt.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/invocation.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
.IF "$(SOLAR_JAVA)"!=""
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/javaloader.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/javavm.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
.ENDIF
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/namingservice.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/nestedreg.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/proxyfac.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/reflection.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/regtypeprov.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/remotebridge.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/security.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/servicemgr.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/shlibloader.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/simplereg.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/streams.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/textinstream.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/textoutstream.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/typeconverter.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/typemgr.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/uriproc.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)
    + $(REGCOMP) -register -r $@ -c \
        $(MY_Q)vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/uuresolver.uno$(DLLPOST)$(MY_Q) \
        -env:URE_INTERNAL_LIB_DIR=$(MY_URL)

.IF "$(LOCALPYFILES)"!=""
.IF "$(PKGFORMAT)"==""
$(foreach,i,$(alllangiso) openoffice_$i openofficewithjre_$i ooolanguagepack_$i) updatepack : $(LOCALPYFILES) $(BIN)$/cp1251.py
.ELSE			# "$(PKGFORMAT)"==""
$(foreach,i,$(alllangiso) openoffice_$i{$(PKGFORMAT:^".")} openofficewithjre_$i{$(PKGFORMAT:^".")} ooolanguagepack_$i{$(PKGFORMAT:^".")}) updatepack : $(LOCALPYFILES) $(BIN)$/cp1251.py
.ENDIF			# "$(PKGFORMAT)"==""
.ENDIF			# "$(LOCALPYFILES)"!=""

$(BIN)$/%.py : $(SOLARSHAREDBIN)$/pyuno$/%.py
    @+$(COPY) $< $@

.IF "$(SYSTEM_PYTHON)" != "YES"
$(BIN)$/cp1251.py : $(SOLARLIBDIR)$/python$/encodings$/cp1251.py
    @+$(COPY) $< $@
.ELSE
$(BIN)$/cp1251.py : 
    @+echo "Using system python - nothing more to do here"
.ENDIF

$(BIN)$/intro.bmp : $(SOLARCOMMONPCKDIR)$/openoffice$/nologointro.bmp
    +$(COPY) $< $@

hack_msitemplates .PHONY:
    +-$(MKDIRHIER) $(MSIOFFICETEMPLATEDIR)
    +-$(MKDIRHIER) $(MSILANGPACKTEMPLATEDIR)
    $(GNUCOPY) -ua $(MSIOFFICETEMPLATESOURCE) $(MSIOFFICETEMPLATEDIR:d:d)
    $(GNUCOPY) -ua $(MSILANGPACKTEMPLATESOURCE) $(MSILANGPACKTEMPLATEDIR:d:d)
    +$(RM) $(MSIOFFICETEMPLATEDIR)$/Binary$/Image.bmp
    +$(RM) $(MSILANGPACKTEMPLATEDIR)$/Binary$/Image.bmp
    +$(COPY) $(PRJ)$/res$/nologoinstall.bmp $(MSIOFFICETEMPLATEDIR)$/Binary$/Image.bmp
    +$(COPY) $(PRJ)$/res$/nologoinstall.bmp $(MSILANGPACKTEMPLATEDIR)$/Binary$/Image.bmp

