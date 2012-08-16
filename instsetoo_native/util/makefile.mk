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

PRJ=..
PRJNAME=instsetoo_native
TARGET=util

.INCLUDE:  settings.mk

# PERL:=@echo

# watch for the path delimiter
.IF "$(GUI)"=="WNT"
PYTHONPATH:=$(PWD)$/$(BIN);$(SOLARLIBDIR);$(SOLARLIBDIR)$/python;$(SOLARLIBDIR)$/python$/lib-dynload
.ELSE			# "$(GUI)"=="WNT"
PYTHONPATH:=$(PWD)$/$(BIN):$(SOLARLIBDIR):$(SOLARLIBDIR)$/python:$(SOLARLIBDIR)$/python$/lib-dynload
.ENDIF			# "$(GUI)"=="WNT"
.EXPORT: PYTHONPATH

ENABLE_DOWNLOADSETS*=TRUE

.EXPORT: ENABLE_DOWNLOADSETS
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

.IF "$(DISABLE_PYTHON)" != "TRUE"
LOCALPYFILES= \
    $(BIN)$/uno.py \
    $(BIN)$/unohelper.py \
    $(BIN)$/pythonloader.py
.ENDIF

.IF "$(BUILD_TYPE)"=="$(BUILD_TYPE:s/HELP//)"
OOOHELPPACK=
OOODEVHELPPACK=
.ELSE
OOOHELPPACK=ooohelppack
OOODEVHELPPACK=ooodevhelppack
.ENDIF

help_exist:=$(shell @find $(L10N_MODULE)/source/ -type d -name "helpcontent2" 2>/dev/null | sed -e "s|/helpcontent2||" -e "s|^.*/||" ) en-US qtz

# do not build key is pseudolanguage for release builds
.IF "$(ENABLE_RELEASE_BUILD)"=="TRUE"
alllangiso:=$(alllangiso:s/qtz //)
.ENDIF

allhelplangiso:=$(foreach,i,$(alllangiso) $(foreach,j,$(help_exist) $(eq,$i,$j  $i $(NULL))))

xxxx:
    echo $(PERL) -w $(SOLARENV)$/bin$/gen_update_info.pl --buildid $(BUILD) --arch "$(RTL_ARCH)" --os "$(RTL_OS)" --lstfile $(PRJ)$/util$/openoffice.lst --product LibreOffice --languages $(subst,$(@:s/_/ /:1)_, $(@:b)) $(PRJ)$/util$/update.xml

.IF "$(LIBO_DEV_INSTALL)" == "TRUE"
ALLTAR: openoffice_$(defaultlangiso).archive
.ELSE
.IF "$(GUI)"!="WNT" && "$(EPM)"=="NO"
ALLTAR  : $(LOCALPYFILES)
    @echo "No EPM: do no packaging at this stage"
.ELSE			# "$(GUI)"!="WNT" && "$(EPM)"=="NO"
.IF "$(ENABLE_RELEASE_BUILD)"=="TRUE"
.IF "$(BUILD_TYPE)"=="$(BUILD_TYPE:s/ODK//)"
.IF "$(GUI)"=="WNT"
ALLTAR : openofficeall $(OOOHELPPACK)
.ELSE
ALLTAR : openoffice_$(defaultlangiso) ooolanguagepack $(eq,$(OS),MACOSX $(NULL) $(OOOHELPPACK)) $(eq,$(OS),MACOSX $(NULL) lotest_en-US)
.ENDIF
.ELSE
.IF "$(GUI)"=="WNT"
ALLTAR : openofficeall $(OOOHELPPACK) sdkooall
.ELSE
ALLTAR : openoffice_$(defaultlangiso) ooolanguagepack $(eq,$(OS),MACOSX $(NULL) $(OOOHELPPACK)) $(eq,$(OS),MACOSX $(NULL) lotest_en-US) sdkoo_en-US
.ENDIF
.ENDIF
.ELSE # "$(ENABLE_RELEASE_BUILD)"=="TRUE"
.IF "$(BUILD_TYPE)"=="$(BUILD_TYPE:s/ODK//)"
.IF "$(GUI)"=="WNT"
ALLTAR : openofficedevall $(OOODEVHELPPACK)
.ELSE
ALLTAR : openofficedev_$(defaultlangiso) ooodevlanguagepack $(eq,$(OS),MACOSX $(NULL) $(OOODEVHELPPACK)) $(eq,$(OS),MACOSX $(NULL) lodevtest_en-US)
.ENDIF
.ELSE
.IF "$(GUI)"=="WNT"
ALLTAR : openofficedevall $(OOODEVHELPPACK) sdkoodevall
.ELSE
ALLTAR : openofficedev_$(defaultlangiso) ooodevlanguagepack $(eq,$(OS),MACOSX $(NULL) $(OOODEVHELPPACK)) $(eq,$(OS),MACOSX $(NULL) lodevtest_en-US) sdkoodev_en-US
.ENDIF
.ENDIF
.ENDIF # "$(ENABLE_RELEASE_BUILD)"=="TRUE"
.ENDIF			# "$(GUI)"!="WNT" && "$(EPM)"=="NO"
.ENDIF # "$(LIBO_DEV_INSTALL)" == "TRUE"

.IF "$(MAKETARGETS:e)"!=""
PKGFORMAT+=$(MAKETARGETS:e:s/.//)
.ENDIF			# "$(MAKETARGETS:e)"!=""

.IF "$(VERBOSE)"=="TRUE"
VERBOSESWITCH=-verbose
.ENDIF
.IF "$(VERBOSE)"=="FALSE"
VERBOSESWITCH=-quiet
.ENDIF

updatepack:
    $(PERL) -w $(SOLARENV)$/bin$/packager.pl

.IF "$(alllangiso)"!=""

openoffice: $(foreach,i,$(alllangiso) openoffice_$i)

openofficedev: $(foreach,i,$(alllangiso) openofficedev_$i)

ooolanguagepack : $(foreach,i,$(alllangiso) ooolanguagepack_$i)

ooodevlanguagepack: $(foreach,i,$(alllangiso) ooodevlanguagepack_$i)

ooohelppack : $(foreach,i,$(allhelplangiso) ooohelppack_$i)

ooodevhelppack: $(foreach,i,$(allhelplangiso) ooodevhelppack_$i)

sdkoo: $(foreach,i,$(alllangiso) sdkoo_$i)

sdkoodev: $(foreach,i,$(alllangiso) sdkoodev_$i)

lotest: $(foreach,i,$(alllangiso) lotest_$i)

lodevtest: $(foreach,i,$(alllangiso) lodevtest_$i)

ure: $(foreach,i,$(alllangiso) ure_$i)

oxygenoffice: $(foreach,i,$(alllangiso) oxygenoffice_$i)

oxygenofficelanguagepack : $(foreach,i,$(alllangiso) oxygenofficelanguagepack_$i)

oxygenofficehelppack : $(foreach,i,$(allhelplangiso) oxygenofficehelppack_$i)

MSIOFFICETEMPLATESOURCE=$(PRJ)$/inc_openoffice$/windows$/msi_templates
MSILANGPACKTEMPLATESOURCE=$(PRJ)$/inc_ooolangpack$/windows$/msi_templates
MSIHELPPACKTEMPLATESOURCE=$(PRJ)$/inc_ooohelppack$/windows$/msi_templates
MSIURETEMPLATESOURCE=$(PRJ)$/inc_ure$/windows$/msi_templates
MSISDKOOTEMPLATESOURCE=$(PRJ)$/inc_sdkoo$/windows$/msi_templates
MSITESTTEMPLATESOURCE=$(PRJ)$/inc_lotest$/windows$/msi_templates
MSICOMMONTEMPLATESOURCE=$(PRJ)$/inc_common$/windows$/msi_templates

NOLOGOSPLASH:=$(BIN)$/intro.zip
DEVNOLOGOSPLASH:=$(BIN)$/dev$/intro.zip
MSIOFFICETEMPLATEDIR=$(MISC)$/openoffice$/msi_templates
MSILANGPACKTEMPLATEDIR=$(MISC)$/ooolangpack$/msi_templates
MSIHELPPACKTEMPLATEDIR=$(MISC)$/ooohelppack$/msi_templates
MSIURETEMPLATEDIR=$(MISC)$/ure$/msi_templates
MSISDKOOTEMPLATEDIR=$(MISC)$/sdkoo$/msi_templates
MSITESTTEMPLATEDIR=$(MISC)$/lotest$/msi_templates

ADDDEPS=$(NOLOGOSPLASH) $(DEVNOLOGOSPLASH)
.IF "$(OS)" == "WNT"
ADDDEPS+=hack_msitemplates
.ENDIF

$(foreach,i,$(alllangiso) openoffice_$i) : $(ADDDEPS)
openoffice_$(defaultlangiso).archive : $(ADDDEPS)

$(foreach,i,$(alllangiso) openofficedev_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) ooolanguagepack_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) ooodevlanguagepack_$i) : $(ADDDEPS)

$(foreach,i,$(allhelplangiso) ooohelppack_$i) : $(ADDDEPS)

$(foreach,i,$(allhelplangiso) ooodevhelppack_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) sdkoo_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) sdkoodev_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) lotest_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) lodevtest_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) ure_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) oxygenoffice_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) oxygenofficelanguagepack_$i) : $(ADDDEPS)

$(foreach,i,$(allhelplangiso) oxygenofficehelppack_$i) : $(ADDDEPS)

.IF "$(MAKETARGETS)"!=""
$(MAKETARGETS) : $(ADDDEPS)
.ENDIF			# "$(MAKETARGETS)"!=""

$(foreach,i,$(alllangiso) openoffice_$i) : $$@{$(PKGFORMAT:^".")}
.IF "$(MAKETARGETS)"!=""
.IF "$(MAKETARGETS:e)"=="" && "$(MAKETARGETS:s/_//)"!="$(MAKETARGETS)"
$(MAKETARGETS) : $$@{$(PKGFORMAT:^".")}
$(MAKETARGETS){$(PKGFORMAT:^".")} : $(ADDDEPS)
.ENDIF			# "$(MAKETARGETS:e)"=="" && "$(MAKETARGETS:s/_//)"!="$(MAKETARGETS)"
.ENDIF			# "$(MAKETARGETS)"!=""
openoffice_%{$(PKGFORMAT:^".") .archive} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p LibreOffice -u $(OUT) -buildid $(BUILD) -msitemplate $(MSIOFFICETEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -format $(@:e:s/.//) $(VERBOSESWITCH)
    $(PERL) -w $(SOLARENV)$/bin$/gen_update_info.pl --buildid $(BUILD) --arch "$(RTL_ARCH)" --os "$(RTL_OS)" --lstfile $(PRJ)$/util$/openoffice.lst --product LibreOffice --languages $(subst,$(@:s/_/ /:1)_, $(@:b)) $(PRJ)$/util$/update.xml > $(MISC)/`date +%Y%m%d_%H%M`_$(RTL_OS)_$(RTL_ARCH)$(@:e).update.xml

$(foreach,i,$(alllangiso) openofficedev_$i) : $$@{$(PKGFORMAT:^".")}
openofficedev_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p LibreOffice_Dev -u $(OUT) -buildid $(BUILD) -msitemplate $(MSIOFFICETEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -format $(@:e:s/.//) $(VERBOSESWITCH)
    $(PERL) -w $(SOLARENV)$/bin$/gen_update_info.pl --buildid $(BUILD) --arch "$(RTL_ARCH)" --os "$(RTL_OS)" --lstfile $(PRJ)$/util$/openoffice.lst --product LibreOffice_Dev --languages $(subst,$(@:s/_/ /:1)_, $(@:b)) $(PRJ)$/util$/update.xml > $(MISC)/`date +%Y%m%d_%H%M`_$(RTL_OS)_$(RTL_ARCH)$(@:e).update.xml

$(foreach,i,$(alllangiso) ooolanguagepack_$i) : $$@{$(PKGFORMAT:^".")}
ooolanguagepack_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p LibreOffice -u $(OUT) -buildid $(BUILD) -msitemplate $(MSILANGPACKTEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -languagepack -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(alllangiso) ooodevlanguagepack_$i) : $$@{$(PKGFORMAT:^".")}
ooodevlanguagepack_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p LibreOffice_Dev -u $(OUT) -buildid $(BUILD) -msitemplate $(MSILANGPACKTEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -languagepack -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(allhelplangiso) ooohelppack_$i) : $$@{$(PKGFORMAT:^".")}
ooohelppack_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p LibreOffice -u $(OUT) -buildid $(BUILD) -msitemplate $(MSIHELPPACKTEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -helppack -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(allhelplangiso) ooodevhelppack_$i) : $$@{$(PKGFORMAT:^".")}
ooodevhelppack_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p LibreOffice_Dev -u $(OUT) -buildid $(BUILD) -msitemplate $(MSIHELPPACKTEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -helppack -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(alllangiso) sdkoo_$i) : $$@{$(PKGFORMAT:^".")}
sdkoo_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p LibreOffice_SDK -u $(OUT) -buildid $(BUILD) -msitemplate $(MSISDKOOTEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(alllangiso) sdkoodev_$i) : $$@{$(PKGFORMAT:^".")}
sdkoodev_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p LibreOffice_Dev_SDK -u $(OUT) -buildid $(BUILD) -msitemplate $(MSISDKOOTEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(alllangiso) lotest_$i) : $$@{$(PKGFORMAT:^".")}
lotest_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p LibreOffice_Test -u $(OUT) -buildid $(BUILD) -msitemplate $(MSITESTTEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(alllangiso) lodevtest_$i) : $$@{$(PKGFORMAT:^".")}
lodevtest_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p LibreOffice_Dev_Test -u $(OUT) -buildid $(BUILD) -msitemplate $(MSITESTTEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -format $(@:e:s/.//) $(VERBOSESWITCH)


$(foreach,i,$(alllangiso) ure_$i) : $$@{$(PKGFORMAT:^".")}
ure_%{$(PKGFORMAT:^".")} :
.IF "$(OS)" == "MACOSX"
    @echo 'for now, there is no standalone URE for Mac OS X'
.ELSE
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst \
        -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p URE -u $(OUT) -buildid $(BUILD) -format $(@:e:s/.//) $(VERBOSESWITCH) \
        -msitemplate $(MSIURETEMPLATEDIR) \
        -msilanguage $(COMMONMISC)$/win_ulffiles
.ENDIF

$(foreach,i,$(alllangiso) oxygenoffice_$i) : $$@{$(PKGFORMAT:^".")}
.IF "$(MAKETARGETS)"!=""
.IF "$(MAKETARGETS:e)"=="" && "$(MAKETARGETS:s/_//)"!="$(MAKETARGETS)"
$(MAKETARGETS) : $$@{$(PKGFORMAT:^".")}
$(MAKETARGETS){$(PKGFORMAT:^".")} : $(ADDDEPS)
.ENDIF			# "$(MAKETARGETS:e)"=="" && "$(MAKETARGETS:s/_//)"!="$(MAKETARGETS)"
.ENDIF			# "$(MAKETARGETS)"!=""
oxygenoffice_%{$(PKGFORMAT:^".") .archive} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p OxygenOffice -u $(OUT) -buildid $(BUILD) -msitemplate $(MSIOFFICETEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -format $(@:e:s/.//) $(VERBOSESWITCH)
    $(PERL) -w $(SOLARENV)$/bin$/gen_update_info.pl --buildid $(BUILD) --arch "$(RTL_ARCH)" --os "$(RTL_OS)" --lstfile $(PRJ)$/util$/openoffice.lst --product OxygenOffice --languages $(subst,$(@:s/_/ /:1)_, $(@:b)) $(PRJ)$/util$/update.xml > $(MISC)/`date +%Y%m%d_%H%M`_$(RTL_OS)_$(RTL_ARCH)$(@:e).update.xml

$(foreach,i,$(alllangiso) oxygenofficelanguagepack_$i) : $$@{$(PKGFORMAT:^".")}
oxygenofficelanguagepack_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p OxygenOffice -u $(OUT) -buildid $(BUILD) -msitemplate $(MSILANGPACKTEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -languagepack -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(allhelplangiso) oxygenofficehelppack_$i) : $$@{$(PKGFORMAT:^".")}
oxygenofficehelppack_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p OxygenOffice -u $(OUT) -buildid $(BUILD) -msitemplate $(MSIHELPPACKTEMPLATEDIR) -msilanguage $(COMMONMISC)$/win_ulffiles -helppack -format $(@:e:s/.//) $(VERBOSESWITCH)

.ELSE			# "$(alllangiso)"!=""
openoffice:
    @echo cannot pack anything...

.ENDIF			# "$(alllangiso)"!=""

.IF "$(DISABLE_PYTHON)" != "TRUE"
.IF "$(LOCALPYFILES)"!=""
$(foreach,i,$(alllangiso) openoffice_$i{$(PKGFORMAT:^".") .archive} openofficedev_$i{$(PKGFORMAT:^".")} sdkoo_$i{$(PKGFORMAT:^".")} lotest_$i{$(PKGFORMAT:^".")} oxygenoffice_$i{$(PKGFORMAT:^".") .archive}) updatepack : $(LOCALPYFILES)
.ENDIF			# "$(LOCALPYFILES)"!=""

.IF "$(GUI)"!="WNT"
$(BIN)$/%.py : $(OUTDIR)$/lib$/pyuno$/%.py
    $(COPY) $< $@
.ELSE
$(BIN)$/%.py : $(OUTDIR)$/bin$/pyuno$/%.py
    $(COPY) $< $@
.ENDIF
.ENDIF			# "$(DISABLE_PYTHON)" != "TRUE"

$(BIN)$/intro.zip : $(SOLARCOMMONPCKDIR)$/intro.zip
    $(COPY) $< $@

$(BIN)$/dev$/intro.zip : $(SOLARCOMMONPCKDIR)$/brand_dev$/intro.zip
    @-$(MKDIR) $(@:d)
    $(COPY) $< $@

hack_msitemplates .PHONY:
    -$(MKDIRHIER) $(MSIOFFICETEMPLATEDIR)
    -$(MKDIRHIER) $(MSILANGPACKTEMPLATEDIR)
    -$(MKDIRHIER) $(MSIHELPPACKTEMPLATEDIR)
    -$(MKDIRHIER) $(MSIURETEMPLATEDIR)
    -$(MKDIRHIER) $(MSISDKOOTEMPLATEDIR)
    -$(MKDIRHIER) $(MSITESTTEMPLATEDIR)
    -$(MKDIRHIER) $(MSIOFFICETEMPLATEDIR)$/Binary
    -$(MKDIRHIER) $(MSILANGPACKTEMPLATEDIR)$/Binary
    -$(MKDIRHIER) $(MSIHELPPACKTEMPLATEDIR)$/Binary
    -$(MKDIRHIER) $(MSIURETEMPLATEDIR)$/Binary
    -$(MKDIRHIER) $(MSISDKOOTEMPLATEDIR)$/Binary
    -$(MKDIRHIER) $(MSITESTTEMPLATEDIR)$/Binary
    $(GNUCOPY) -u $(MSIOFFICETEMPLATESOURCE)$/*.* $(MSIOFFICETEMPLATEDIR)
    $(GNUCOPY) -u $(MSILANGPACKTEMPLATESOURCE)$/*.* $(MSILANGPACKTEMPLATEDIR)
    $(GNUCOPY) -u $(MSIHELPPACKTEMPLATESOURCE)$/*.* $(MSIHELPPACKTEMPLATEDIR)
    $(GNUCOPY) -u $(MSIURETEMPLATESOURCE)$/*.* $(MSIURETEMPLATEDIR)
    $(GNUCOPY) -u $(MSISDKOOTEMPLATESOURCE)$/*.* $(MSISDKOOTEMPLATEDIR)
    $(GNUCOPY) -u $(MSICOMMONTEMPLATESOURCE)$/Binary$/*.* $(MSIOFFICETEMPLATEDIR)$/Binary
    $(GNUCOPY) -u $(MSICOMMONTEMPLATESOURCE)$/Binary$/*.* $(MSILANGPACKTEMPLATEDIR)$/Binary
    $(GNUCOPY) -u $(MSICOMMONTEMPLATESOURCE)$/Binary$/*.* $(MSIHELPPACKTEMPLATEDIR)$/Binary
    $(GNUCOPY) -u $(MSICOMMONTEMPLATESOURCE)$/Binary$/*.* $(MSIURETEMPLATEDIR)$/Binary
    $(GNUCOPY) -u $(MSICOMMONTEMPLATESOURCE)$/Binary$/*.* $(MSISDKOOTEMPLATEDIR)$/Binary
    $(GNUCOPY) -u $(MSICOMMONTEMPLATESOURCE)$/Binary$/*.* $(MSITESTTEMPLATEDIR)$/Binary
    $(COPY) $(PRJ)$/res$/nologoinstall.bmp $(MSIOFFICETEMPLATEDIR)$/Binary$/Image.bmp
    $(COPY) $(PRJ)$/res$/nologoinstall.bmp $(MSILANGPACKTEMPLATEDIR)$/Binary$/Image.bmp
    $(COPY) $(PRJ)$/res$/nologoinstall.bmp $(MSIHELPPACKTEMPLATEDIR)$/Binary$/Image.bmp
    $(COPY) $(PRJ)$/res$/nologoinstall.bmp $(MSIURETEMPLATEDIR)$/Binary$/Image.bmp
    $(COPY) $(PRJ)$/res$/nologoinstall.bmp $(MSISDKOOTEMPLATEDIR)$/Binary$/Image.bmp
    $(COPY) $(PRJ)$/res$/nologoinstall.bmp $(MSITESTTEMPLATEDIR)$/Binary$/Image.bmp
    $(COPY) $(PRJ)$/res$/nologobanner.bmp $(MSIOFFICETEMPLATEDIR)$/Binary$/Banner.bmp
    $(COPY) $(PRJ)$/res$/nologobanner.bmp $(MSILANGPACKTEMPLATEDIR)$/Binary$/Banner.bmp
    $(COPY) $(PRJ)$/res$/nologobanner.bmp $(MSIHELPPACKTEMPLATEDIR)$/Binary$/Banner.bmp
    $(COPY) $(PRJ)$/res$/nologobanner.bmp $(MSIURETEMPLATEDIR)$/Binary$/Banner.bmp
    $(COPY) $(PRJ)$/res$/nologobanner.bmp $(MSISDKOOTEMPLATEDIR)$/Binary$/Banner.bmp
    $(COPY) $(PRJ)$/res$/nologobanner.bmp $(MSITESTTEMPLATEDIR)$/Binary$/Banner.bmp

.IF "$(OS)" == "WNT"

ALLLANGSTRING:=$(alllangiso)

openofficeall: hack_msitemplates $(LOCALPYFILES) openoffice_$(ALLLANGSTRING:s/ /,/)$(PKGFORMAT:^".")

openofficedevall: hack_msitemplates $(LOCALPYFILES) openofficedev_$(ALLLANGSTRING:s/ /,/)$(PKGFORMAT:^".")

sdkooall: hack_msitemplates $(LOCALPYFILES) sdkoo_$(ALLLANGSTRING:s/ /,/)$(PKGFORMAT:^".")

sdkoodevall: hack_msitemplates $(LOCALPYFILES) sdkoodev_$(ALLLANGSTRING:s/ /,/)$(PKGFORMAT:^".")

lotestall: hack_msitemplates $(LOCALPYFILES) lotest_$(ALLLANGSTRING:s/ /,/)$(PKGFORMAT:^".")

lodevtestall: hack_msitemplates $(LOCALPYFILES) lodevtest_$(ALLLANGSTRING:s/ /,/)$(PKGFORMAT:^".")

.ENDIF			# "$(OS)" == "WNT"

