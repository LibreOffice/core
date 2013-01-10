#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..
PRJNAME=instsetoo_native
TARGET=util

.INCLUDE:  settings.mk
.INCLUDE: $(SOLARINCDIR)$/rtlbootstrap.mk

# PERL:=@echo

# watch for the path delimiter
.IF "$(GUI)"=="WNT"
PYTHONPATH:=$(PWD)$/$(BIN);$(SOLARLIBDIR);$(SOLARLIBDIR)$/python;$(SOLARLIBDIR)$/python$/lib-dynload
.ELSE			# "$(GUI)"=="WNT"
PYTHONPATH:=$(PWD)$/$(BIN):$(SOLARLIBDIR):$(SOLARLIBDIR)$/python:$(SOLARLIBDIR)$/python$/lib-dynload
.ENDIF			# "$(GUI)"=="WNT"
.EXPORT: PYTHONPATH

.IF "$(CWS_WORK_STAMP)"=="" || "$(UPDATER)"!=""
ENABLE_DOWNLOADSETS*=TRUE
.ENDIF			# "$(CWS_WORK_STAMP)"=="" || "$(UPDATER)"!=""
.IF "$(FORCE_DOWNLOADSETS)"!=""
ENABLE_DOWNLOADSETS=TRUE
.ENDIF			# "$(FORCE_DOWNLOADSETS)"!=""

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

LOCALPYFILES= \
    $(BIN)$/uno.py \
    $(BIN)$/unohelper.py \
    $(BIN)$/pythonloader.py \
    $(BIN)$/pythonscript.py \
    $(BIN)$/officehelper.py \
    $(BIN)$/mailmerge.py

xxxx:
    echo $(PERL) -w $(SOLARENV)$/bin$/gen_update_info.pl --buildid $(BUILD) --arch "$(RTL_ARCH)" --os "$(RTL_OS)" --lstfile $(PRJ)$/util$/openoffice.lst --product OpenOffice --languages $(subst,$(@:s/_/ /:1)_, $(@:b)) $(PRJ)$/util$/update.xml

.IF "$(GUI)"!="WNT" && "$(EPM)"=="NO" && "$(USE_PACKAGER)"==""
ALLTAR  : $(LOCALPYFILES)
    @echo "No EPM: do no packaging at this stage"
.ELSE			# "$(GUI)"!="WNT" && "$(EPM)"=="NO" && "$(USE_PACKAGER)"==""
.IF "$(UPDATER)"=="" || "$(USE_PACKAGER)"==""
.IF "$(BUILD_TYPE)"=="$(BUILD_TYPE:s/ODK//)"
ALLTAR : openoffice
.ELSE
ALLTAR : openoffice sdkoo_en-US ure_en-US
.ENDIF
.ELSE			# "$(UPDATER)"=="" || "$(USE_PACKAGER)"==""
ALLTAR : updatepack
.ENDIF			# "$(UPDATER)"=="" || "$(USE_PACKAGER)"==""
.ENDIF			# "$(GUI)"!="WNT" && "$(EPM)"=="NO" && "$(USE_PACKAGER)"==""

.IF "$(FORCE2ARCHIVE)" == "TRUE"
PKGFORMAT = archive
.END

.IF "$(MAKETARGETS:e)"!=""
PKGFORMAT+=$(MAKETARGETS:e:s/.//)
.ENDIF			# "$(MAKETARGETS:e)"!=""

# Independent of PKGFORMAT, always build a default-language openoffice product
# also in archive format, so that tests that require an OOo installation (like
# smoketestoo_native) have one available:
openoffice_$(defaultlangiso) : $$@.archive

.IF "$(VERBOSE)"=="TRUE"
VERBOSESWITCH=-verbose
.ELIF "$(VERBOSE)"=="FALSE"
VERBOSESWITCH=-quiet
.ENDIF

.IF "$(VERBOSE_INSTALLER)"=="TRUE"
VERBOSESWITCH+=-log
.ENDIF

# New target to prepare a source release
SRC_RELEASE_OUT_DIR=$(shell cd $(OUT) && pwd)$/AOO_SRC_Release

aoo_srcrelease: $(SOLARENV)$/bin$/srcrelease.xml
    @-$(MKDIR) $(OUT)$/AOO_SRC_Release
    $(ANT) -f $(SOLARENV)$/bin$/srcrelease.xml -q -Dbasedir=$(SOURCE_ROOT_DIR) -Dout.dir=$(SRC_RELEASE_OUT_DIR)

updatepack:
    $(PERL) -w $(SOLARENV)$/bin$/packager.pl

.IF "$(alllangiso)"!=""

openoffice: $(foreach,i,$(alllangiso) openoffice_$i)

openofficedev: $(foreach,i,$(alllangiso) openofficedev_$i)

openofficewithjre: $(foreach,i,$(alllangiso) openofficewithjre_$i)

ooolanguagepack : $(foreach,i,$(alllangiso) ooolanguagepack_$i)

ooodevlanguagepack: $(foreach,i,$(alllangiso) ooodevlanguagepack_$i)

sdkoo: $(foreach,i,$(alllangiso) sdkoo_$i)

sdkoodev: $(foreach,i,$(alllangiso) sdkoodev_$i)

ure: $(foreach,i,$(alllangiso) ure_$i)

MSIOFFICETEMPLATESOURCE=$(PRJ)$/inc_openoffice$/windows$/msi_templates
MSILANGPACKTEMPLATESOURCE=$(PRJ)$/inc_ooolangpack$/windows$/msi_templates
MSIURETEMPLATESOURCE=$(PRJ)$/inc_ure$/windows$/msi_templates
MSISDKOOTEMPLATESOURCE=$(PRJ)$/inc_sdkoo$/windows$/msi_templates

.IF "$(BUILD_SPECIAL)"!=""
MSIOFFICETEMPLATEDIR=$(MSIOFFICETEMPLATESOURCE)
MSILANGPACKTEMPLATEDIR=$(MSILANGPACKTEMPLATESOURCE)
MSIURETEMPLATEDIR=$(MSIURETEMPLATESOURCE)
MSISDKOOTEMPLATEDIR=$(MSISDKOOTEMPLATESOURCE)
.ELSE			# "$(BUILD_SPECIAL)"!=""
NOLOGOSPLASH:=$(BIN)$/intro.zip
DEVNOLOGOSPLASH:=$(BIN)$/dev$/intro.zip
MSIOFFICETEMPLATEDIR=$(MISC)$/openoffice$/msi_templates
MSILANGPACKTEMPLATEDIR=$(MISC)$/ooolangpack$/msi_templates
MSIURETEMPLATEDIR=$(MISC)$/ure$/msi_templates
MSISDKOOTEMPLATEDIR=$(MISC)$/sdkoo$/msi_templates

ADDDEPS=$(NOLOGOSPLASH) $(DEVNOLOGOSPLASH)

.IF "$(OS)" == "WNT"
ADDDEPS+=hack_msitemplates
.ENDIF

$(foreach,i,$(alllangiso) openoffice_$i) : $(ADDDEPS)
openoffice_$(defaultlangiso).archive : $(ADDDEPS)

$(foreach,i,$(alllangiso) openofficedev_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) openofficewithjre_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) ooolanguagepack_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) ooodevlanguagepack_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) sdkoo_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) sdkoodev_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) ure_$i) : $(ADDDEPS)

.IF "$(MAKETARGETS)"!=""
$(MAKETARGETS) : $(ADDDEPS)
.ENDIF			# "$(MAKETARGETS)"!=""

.ENDIF			# "$(BUILD_SPECIAL)"!=""

$(foreach,i,$(alllangiso) openoffice_$i) : $$@{$(PKGFORMAT:^".")}
.IF "$(MAKETARGETS)"!=""
.IF "$(MAKETARGETS:e)"=="" && "$(MAKETARGETS:s/_//)"!="$(MAKETARGETS)"
$(MAKETARGETS) : $$@{$(PKGFORMAT:^".")}
$(MAKETARGETS){$(PKGFORMAT:^".")} : $(ADDDEPS)
.ENDIF			# "$(MAKETARGETS:e)"=="" && "$(MAKETARGETS:s/_//)"!="$(MAKETARGETS)"
.ENDIF			# "$(MAKETARGETS)"!=""
openoffice_%{$(PKGFORMAT:^".") .archive} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p Apache_OpenOffice -u $(OUT) -buildid $(BUILD) -msitemplate $(MSIOFFICETEMPLATEDIR) -msilanguage $(MISC)$/win_ulffiles -format $(@:e:s/.//) $(VERBOSESWITCH)
    $(PERL) -w $(SOLARENV)$/bin$/gen_update_info.pl --buildid $(BUILD) --arch "$(RTL_ARCH)" --os "$(RTL_OS)" --lstfile $(PRJ)$/util$/openoffice.lst --product Apache_OpenOffice --languages $(subst,$(@:s/_/ /:1)_, $(@:b)) $(PRJ)$/util$/update.xml > $(MISC)/$(@:b)_$(RTL_OS)_$(RTL_ARCH)$(@:e).update.xml

$(foreach,i,$(alllangiso) openofficewithjre_$i) : $$@{$(PKGFORMAT:^".")}
openofficewithjre_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p Aapche_OpenOffice_wJRE -u $(OUT) -buildid $(BUILD) -msitemplate $(MSIOFFICETEMPLATEDIR) -msilanguage $(MISC)$/win_ulffiles -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(alllangiso) openofficedev_$i) : $$@{$(PKGFORMAT:^".")}
openofficedev_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p Apache_OpenOffice_Dev -u $(OUT) -buildid $(BUILD) -msitemplate $(MSIOFFICETEMPLATEDIR) -msilanguage $(MISC)$/win_ulffiles -format $(@:e:s/.//) $(VERBOSESWITCH)
    $(PERL) -w $(SOLARENV)$/bin$/gen_update_info.pl --buildid $(BUILD) --arch "$(RTL_ARCH)" --os "$(RTL_OS)" --lstfile $(PRJ)$/util$/openoffice.lst --product Apache_OpenOffice_Dev --languages $(subst,$(@:s/_/ /:1)_, $(@:b)) $(PRJ)$/util$/update.xml > $(MISC)/$(@:b)_$(RTL_OS)_$(RTL_ARCH)$(@:e).update.xml

$(foreach,i,$(alllangiso) ooolanguagepack_$i) : $$@{$(PKGFORMAT:^".")}
ooolanguagepack_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p Apache_OpenOffice -u $(OUT) -buildid $(BUILD) -msitemplate $(MSILANGPACKTEMPLATEDIR) -msilanguage $(MISC)$/win_ulffiles -languagepack -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(alllangiso) ooodevlanguagepack_$i) : $$@{$(PKGFORMAT:^".")}
ooodevlanguagepack_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p Apache_OpenOffice_Dev -u $(OUT) -buildid $(BUILD) -msitemplate $(MSILANGPACKTEMPLATEDIR) -msilanguage $(MISC)$/win_ulffiles -languagepack -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(alllangiso) sdkoo_$i) : $$@{$(PKGFORMAT:^".")}
sdkoo_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p Apache_OpenOffice_SDK -u $(OUT) -buildid $(BUILD) -msitemplate $(MSISDKOOTEMPLATEDIR) -msilanguage $(MISC)$/win_ulffiles -dontstrip -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(alllangiso) sdkoodev_$i) : $$@{$(PKGFORMAT:^".")}
sdkoodev_%{$(PKGFORMAT:^".")} :
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p Apache_OpenOffice_Dev_SDK -u $(OUT) -buildid $(BUILD) -msitemplate $(MSISDKOOTEMPLATEDIR) -msilanguage $(MISC)$/win_ulffiles -dontstrip -format $(@:e:s/.//) $(VERBOSESWITCH)

$(foreach,i,$(alllangiso) ure_$i) : $$@{$(PKGFORMAT:^".")}
ure_%{$(PKGFORMAT:^".")} :
.IF "$(OS)" == "MACOSX"
    @echo 'for now, there is no standalone URE for Mac OS X'
.ELSE
    $(PERL) -w $(SOLARENV)$/bin$/make_installer.pl -f $(PRJ)$/util$/openoffice.lst \
        -l $(subst,$(@:s/_/ /:1)_, $(@:b)) -p URE -u $(OUT) -buildid $(BUILD) -format $(@:e:s/.//) $(VERBOSESWITCH) \
        -msitemplate $(MSIURETEMPLATEDIR) \
        -msilanguage $(MISC)$/win_ulffiles
.ENDIF

.IF "$(MAKETARGETS)"!=""
.IF "$(MAKETARGETS:e)"=="" && "$(MAKETARGETS:s/_//)"!="$(MAKETARGETS)"
$(MAKETARGETS) : $$@{$(PKGFORMAT:^".")}
$(MAKETARGETS){$(PKGFORMAT:^".")} : $(ADDDEPS)
.ENDIF			# "$(MAKETARGETS:e)"=="" && "$(MAKETARGETS:s/_//)"!="$(MAKETARGETS)"
.ENDIF			# "$(MAKETARGETS)"!=""

.ELSE			# "$(alllangiso)"!=""
openoffice:
    @echo cannot pack nothing...

.ENDIF			# "$(alllangiso)"!=""

.IF "$(LOCALPYFILES)"!=""
$(foreach,i,$(alllangiso) openoffice_$i{$(PKGFORMAT:^".") .archive} openofficewithjre_$i{$(PKGFORMAT:^".")} openofficedev_$i{$(PKGFORMAT:^".")} sdkoo_$i{$(PKGFORMAT:^".")}) updatepack : $(LOCALPYFILES)
.ENDIF			# "$(LOCALPYFILES)"!=""

$(BIN)$/%.py : $(SOLARSHAREDBIN)$/pyuno$/%.py
    @$(COPY) $< $@

$(BIN)$/intro.zip : $(SOLARCOMMONPCKDIR)$/openoffice_nologo$/intro.zip
    $(COPY) $< $@

$(BIN)$/dev$/intro.zip : $(SOLARCOMMONPCKDIR)$/openoffice_dev_nologo$/intro.zip
    @-$(MKDIR) $(@:d)
    $(COPY) $< $@

hack_msitemplates .PHONY:
    -$(MKDIRHIER) $(MSIOFFICETEMPLATEDIR)
    -$(MKDIRHIER) $(MSILANGPACKTEMPLATEDIR)
    -$(MKDIRHIER) $(MSIURETEMPLATEDIR)
    -$(MKDIRHIER) $(MSISDKOOTEMPLATEDIR)
    -$(MKDIRHIER) $(MSIOFFICETEMPLATEDIR)$/Binary
    -$(MKDIRHIER) $(MSILANGPACKTEMPLATEDIR)$/Binary
    -$(MKDIRHIER) $(MSIURETEMPLATEDIR)$/Binary
    -$(MKDIRHIER) $(MSISDKOOTEMPLATEDIR)$/Binary
    $(GNUCOPY) $(MSIOFFICETEMPLATESOURCE)$/*.* $(MSIOFFICETEMPLATEDIR)
    $(GNUCOPY) $(MSILANGPACKTEMPLATESOURCE)$/*.* $(MSILANGPACKTEMPLATEDIR)
    $(GNUCOPY) $(MSIURETEMPLATESOURCE)$/*.* $(MSIURETEMPLATEDIR)
    $(GNUCOPY) $(MSISDKOOTEMPLATESOURCE)$/*.* $(MSISDKOOTEMPLATEDIR)
    $(GNUCOPY) $(MSIOFFICETEMPLATESOURCE)$/Binary$/*.* $(MSIOFFICETEMPLATEDIR)$/Binary
    $(GNUCOPY) $(MSILANGPACKTEMPLATESOURCE)$/Binary$/*.* $(MSILANGPACKTEMPLATEDIR)$/Binary
    $(GNUCOPY) $(MSIURETEMPLATESOURCE)$/Binary$/*.* $(MSIURETEMPLATEDIR)$/Binary
    $(GNUCOPY) $(MSISDKOOTEMPLATESOURCE)$/Binary$/*.* $(MSISDKOOTEMPLATEDIR)$/Binary
    $(RM) $(MSIOFFICETEMPLATEDIR)$/Binary$/Image.bmp
    $(RM) $(MSILANGPACKTEMPLATEDIR)$/Binary$/Image.bmp
    $(RM) $(MSIURETEMPLATEDIR)$/Binary$/Image.bmp
    $(RM) $(MSISDKOOTEMPLATEDIR)$/Binary$/Image.bmp
    $(COPY) $(PRJ)$/res$/nologoinstall.bmp $(MSIOFFICETEMPLATEDIR)$/Binary$/Image.bmp
    $(COPY) $(PRJ)$/res$/nologoinstall.bmp $(MSILANGPACKTEMPLATEDIR)$/Binary$/Image.bmp
    $(COPY) $(PRJ)$/res$/nologoinstall.bmp $(MSIURETEMPLATEDIR)$/Binary$/Image.bmp
    $(COPY) $(PRJ)$/res$/nologoinstall.bmp $(MSISDKOOTEMPLATEDIR)$/Binary$/Image.bmp


