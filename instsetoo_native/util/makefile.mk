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

# The help target belongs after the inclusion of target.mk to not become the default target.
help .PHONY :
    @echo "known targets:"
    @echo "    openoffice             builds the default installation packages for the platform"
    @echo "    aoo_srcrelease         packs the source release package"
    @echo "    updatepack"
    @echo "    openofficedev          devloper snapshot"
    @echo "    openofficewithjre"
    @echo "    ooolanguagepack"
    @echo "    ooodevlanguagepack"
    @echo "    sdkoo"
    @echo "    sdkoodev"
    @echo 
    @echo "experimental targets:"
    @echo "    patch_create           create a patch for updating an installed office (Windows only)"
    @echo "    patch_apply            apply a previously created patch"
    @echo 
    @echo "Most targets (all except aoo_srcrelease and updatepack) accept suffixes"
    @echo "    add _<language> to build a target for one language only"
    @echo "        the default set of languages is alllangiso=$(alllangiso)"
    @echo "    add .<package_format> to build a target for one package format only"
    @echo "        the default set of package formats is archive and PKGFORMAT=$(PKGFORMAT)"


LOCALPYFILES=			\
    $(BIN)$/uno.py		\
    $(BIN)$/unohelper.py	\
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
ALLTAR : openoffice sdkoo_en-US
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
#openoffice_$(defaultlangiso) : $$@.archive

.IF "$(VERBOSE)"=="TRUE"
VERBOSESWITCH=-verbose
.ELIF "$(VERBOSE)"=="FALSE"
VERBOSESWITCH=-quiet
.ENDIF

.IF "$(release:U)"=="T"
RELEASE_SWITCH=-release
$(foreach,i,$(alllangiso) openoffice_$i.msi) : prepare_release_build
.ELSE
RELEASE_SWITCH=
.ENDIF

prepare_release_build .PHONY:
    @$(PERL) -w $(SOLARENV)$/bin$/release_prepare.pl 	\
        --lst-file $(PRJ)$/util$/openoffice.lst 	\
        --product-name Apache_OpenOffice		\
        --output-path $(OUT) 				\
        $(alllangiso)

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

MSIOFFICETEMPLATESOURCE=$(PRJ)$/inc_openoffice$/windows$/msi_templates
MSILANGPACKTEMPLATESOURCE=$(PRJ)$/inc_ooolangpack$/windows$/msi_templates
MSISDKOOTEMPLATESOURCE=$(PRJ)$/inc_sdkoo$/windows$/msi_templates

NOLOGOSPLASH:=$(BIN)$/intro.zip
DEVNOLOGOSPLASH:=$(BIN)$/dev$/intro.zip
MSIOFFICETEMPLATEDIR=$(MISC)$/openoffice$/msi_templates
MSILANGPACKTEMPLATEDIR=$(MISC)$/ooolangpack$/msi_templates
MSISDKOOTEMPLATEDIR=$(MISC)$/sdkoo$/msi_templates

ADDDEPS=adddeps
adddeps .PHONY : $(NOLOGOSPLASH) $(DEVNOLOGOSPLASH)

.IF "$(OS)" == "WNT"
adddeps : msitemplates
.ENDIF

.IF "$(LOCALPYFILES)"!=""
local_python_files .PHONY : $(LOCALPYFILES)
adddeps : local_python_files
updatepack : local_python_files
.ENDIF			# "$(LOCALPYFILES)"!=""


$(foreach,i,$(alllangiso) openoffice_$i) : $(ADDDEPS)
openoffice_$(defaultlangiso).archive : $(ADDDEPS)

$(foreach,i,$(alllangiso) openofficedev_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) openofficewithjre_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) ooolanguagepack_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) ooodevlanguagepack_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) sdkoo_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) sdkoodev_$i) : $(ADDDEPS)

$(foreach,i,$(alllangiso) openoffice_$i) : $$@{$(PKGFORMAT:^".")}
$(foreach,i,$(alllangiso) openofficewithjre_$i) : $$@{$(PKGFORMAT:^".")}
$(foreach,i,$(alllangiso) openofficedev_$i) : $$@{$(PKGFORMAT:^".")}
$(foreach,i,$(alllangiso) ooolanguagepack_$i) : $$@{$(PKGFORMAT:^".")}
$(foreach,i,$(alllangiso) ooodevlanguagepack_$i) : $$@{$(PKGFORMAT:^".")}
$(foreach,i,$(alllangiso) sdkoo_$i) : $$@{$(PKGFORMAT:^".")}
$(foreach,i,$(alllangiso) sdkoodev_$i) : $$@{$(PKGFORMAT:^".")}


# This macro makes calling the make_installer.pl script a bit easier.
# Just add -p and -msitemplate switches.
MAKE_INSTALLER_COMMAND=					\
    @$(PERL) -w $(SOLARENV)$/bin$/make_installer.pl \
        -f $(PRJ)$/util$/openoffice.lst 	\
        -l $(subst,$(@:s/_/ /:1)_, $(@:b)) 	\
        -u $(OUT) 				\
        -buildid $(BUILD) 			\
        -msilanguage $(MISC)$/win_ulffiles	\
        -format $(@:e:s/.//) 			\
        $(VERBOSESWITCH)

# This macro makes calling gen_update_info.pl a bit easier
# Just add --product switches, and xml input file and redirect output.
GEN_UPDATE_INFO_COMMAND=					\
    @$(PERL) -w $(SOLARENV)$/bin$/gen_update_info.pl	\
        --buildid $(BUILD)				\
        --arch "$(RTL_ARCH)"				\
        --os "$(RTL_OS)"				\
        --lstfile $(PRJ)$/util$/openoffice.lst		\
        --languages $(subst,$(@:s/_/ /:1)_, $(@:b))

openoffice_%{$(PKGFORMAT:^".")} :
    $(MAKE_INSTALLER_COMMAND) 			\
        -p Apache_OpenOffice			\
        -msitemplate $(MSIOFFICETEMPLATEDIR)	\
        $(RELEASE_SWITCH)
    $(GEN_UPDATE_INFO_COMMAND)		\
        --product Apache_OpenOffice	\
        $(PRJ)$/util$/update.xml	\
        > $(MISC)/$(@:b)_$(RTL_OS)_$(RTL_ARCH)$(@:e).update.xml

openoffice_%{.archive} :
    $(MAKE_INSTALLER_COMMAND) 		\
        -p Apache_OpenOffice		\
        -msitemplate $(MSIOFFICETEMPLATEDIR)
    $(GEN_UPDATE_INFO_COMMAND)		\
        --product Apache_OpenOffice	\
        $(PRJ)$/util$/update.xml	\
        > $(MISC)/$(@:b)_$(RTL_OS)_$(RTL_ARCH)$(@:e).update.xml

openofficewithjre_%{$(PKGFORMAT:^".")} :
    $(MAKE_INSTALLER_COMMAND) -p Apache_OpenOffice_wJRE -msitemplate $(MSIOFFICETEMPLATEDIR)

openofficedev_%{$(PKGFORMAT:^".")} :
    $(MAKE_INSTALLER_COMMAND)		\
        -p Apache_OpenOffice_Dev	\
        -msitemplate $(MSIOFFICETEMPLATEDIR)
    $(GEN_UPDATE_INFO_COMMAND)			\
        --product Apache_OpenOffice_Dev 	\
        $(PRJ)$/util$/update.xml 		\
        > $(MISC)/$(@:b)_$(RTL_OS)_$(RTL_ARCH)$(@:e).update.xml

ooolanguagepack_%{$(PKGFORMAT:^".")} :
    $(MAKE_INSTALLER_COMMAND)			\
        -p Apache_OpenOffice			\
        -msitemplate $(MSILANGPACKTEMPLATEDIR)	\
        -languagepack

ooodevlanguagepack_%{$(PKGFORMAT:^".")} :
    $(MAKE_INSTALLER_COMMAND) -p Apache_OpenOffice_Dev -msitemplate $(MSILANGPACKTEMPLATEDIR) -languagepack

sdkoo_%{$(PKGFORMAT:^".")} :
    $(MAKE_INSTALLER_COMMAND) -p Apache_OpenOffice_SDK -msitemplate $(MSISDKOOTEMPLATEDIR) -dontstrip

sdkoodev_%{$(PKGFORMAT:^".")} :
    $(MAKE_INSTALLER_COMMAND) -p Apache_OpenOffice_Dev_SDK -msitemplate $(MSISDKOOTEMPLATEDIR) -dontstrip

.ELSE			# "$(alllangiso)"!=""
openoffice:
    @echo cannot pack nothing...

.ENDIF			# "$(alllangiso)"!=""

$(BIN)$/%.py : $(SOLARSHAREDBIN)$/pyuno$/%.py
    $(COPY) $< $@

$(BIN)$/intro.zip : $(SOLARCOMMONPCKDIR)$/intro.zip
    $(COPY) $< $@

$(BIN)$/dev$/intro.zip : $(SOLARCOMMONPCKDIR)$/openoffice_dev$/intro.zip
    @-$(MKDIR) $(@:d)
    $(COPY) $< $@


.IF "$(OS)" == "WNT"
patch_create .PHONY : $(PRJ)$/data
    perl -I $(SOLARENV)$/bin/modules $(SOLARENV)$/bin$/patch_create.pl	\
        --product-name Apache_OpenOffice				\
        --output-path $(OUT)						\
        --data-path $(PRJ)$/data					\
        --lst-file $(PRJ)$/util$/openoffice.lst
patch_apply .PHONY :
    perl -I $(SOLARENV)$/bin/modules $(SOLARENV)$/bin$/patch_apply.pl \
        ../wntmsci12.pro/Apache_OpenOffice/msp/v-4-0-1_v-4-1-0/en-US/openoffice.msp

$(PRJ)$/data :
    mkdir $@
.ELSE
patch .PHONY :
    @echo "patches can only be created on Windows at the moment"
.ENDIF


msitemplates .PHONY: msi_template_files msi_langpack_template_files msi_sdk_template_files

MSI_OFFICE_TEMPLATE_FILES=		\
    ActionTe.idt			\
    AdminExe.idt			\
    AdminUIS.idt			\
    AdvtExec.idt			\
    AppSearc.idt			\
    Binary.idt			\
    CheckBox.idt			\
    Control.idt			\
    ControlC.idt			\
    ControlE.idt			\
    CustomAc.idt			\
    Dialog.idt			\
    Error.idt			\
    EventMap.idt			\
    InstallE.idt			\
    InstallU.idt			\
    LaunchCo.idt			\
    ListBox.idt			\
    Property.idt			\
    RadioBut.idt			\
    RegLocat.idt			\
    Signatur.idt			\
    TextStyl.idt			\
    UIText.idt			\
    _Validat.idt			\
    codes.txt			\
    codes_broo.txt			\
    codes_broodev.txt		\
    codes_ooodev.txt		\
    components.txt			\
    upgradecode_remove_ooo.txt	\
    Binary/Banner.bmp		\
    Binary/Image.bmp		\
    Binary/caution.ico		\
    Binary/dontinstall.ico		\
    Binary/install.ico		\
    Binary/installfirstuse.ico	\
    Binary/installpartial.ico	\
    Binary/installstatemenu.ico	\
    Binary/networkinstall.ico	\
    Binary/newfolder.ico		\
    Binary/openfolder.ico		\
    Binary/setup.ico		\
    Binary/setupcomplete.ico	\
    Binary/setuppartial.ico		\
    Binary/setuprepair.ico		\
    Binary/trashcan.ico		\
    Binary/up.ico

MSI_LANGPACK_TEMPLATE_FILES=		\
    ActionTe.idt			\
    AdminExe.idt			\
    AdminUIS.idt			\
    AdvtExec.idt			\
    Binary.idt			\
    CheckBox.idt			\
    Control.idt			\
    ControlC.idt			\
    ControlE.idt			\
    CustomAc.idt			\
    Dialog.idt			\
    Error.idt			\
    EventMap.idt			\
    InstallE.idt			\
    InstallU.idt			\
    LaunchCo.idt			\
    ListBox.idt			\
    Property.idt			\
    RadioBut.idt			\
    TextStyl.idt			\
    UIText.idt			\
    _Validat.idt			\
    bro_patchcodes.txt		\
    brodev_patchcodes.txt		\
    codes.txt			\
    codes_broo.txt			\
    codes_ooodev.txt		\
    components.txt			\
    ooo_patchcodes.txt		\
    ooodev_patchcodes.txt		\
    Binary/Banner.bmp

MSI_SDK_TEMPLATE_FILES=			\
    ActionTe.idt			\
    AdminExe.idt			\
    AdminUIS.idt			\
    AdvtExec.idt			\
    AppSearc.idt			\
    Binary.idt			\
    CheckBox.idt			\
    Control.idt			\
    ControlC.idt			\
    ControlE.idt			\
    CustomAc.idt			\
    Dialog.idt			\
    Error.idt			\
    EventMap.idt			\
    InstallE.idt			\
    InstallU.idt			\
    LaunchCo.idt			\
    ListBox.idt			\
    Property.idt			\
    RadioBut.idt			\
    RegLocat.idt			\
    Signatur.idt			\
    TextStyl.idt			\
    UIText.idt			\
    _Validat.idt			\
    codes.txt			\
    components.txt			\
    Binary/Banner.bmp		\
    Binary/Image.bmp		\
    Binary/caution.ico		\
    Binary/dontinstall.ico		\
    Binary/install.ico		\
    Binary/installfirstuse.ico	\
    Binary/installpartial.ico	\
    Binary/installstatemenu.ico	\
    Binary/networkinstall.ico	\
    Binary/newfolder.ico		\
    Binary/openfolder.ico		\
    Binary/setup.ico		\
    Binary/setupcomplete.ico	\
    Binary/setuppartial.ico		\
    Binary/setuprepair.ico		\
    Binary/trashcan.ico		\
    Binary/up.ico

msi_template_files .PHONY:					\
    $(MSIOFFICETEMPLATEDIR)					\
    $(MSIOFFICETEMPLATEDIR)$/Binary 			\
    $(MSIOFFICETEMPLATEDIR)$/{$(MSI_OFFICE_TEMPLATE_FILES)}
$(MSIOFFICETEMPLATEDIR) $(MSIOFFICETEMPLATEDIR)$/Binary :
    -$(MKDIRHIER) $@
$(MSIOFFICETEMPLATEDIR)$/% : $(MSIOFFICETEMPLATESOURCE)$/%
    $(GNUCOPY) $< $@

msi_langpack_template_files .PHONY :				\
    $(MSILANGPACKTEMPLATEDIR)				\
    $(MSILANGPACKTEMPLATEDIR)$/Binary			\
    $(MSILANGPACKTEMPLATEDIR)$/{$(MSI_LANGPACK_TEMPLATE_FILES)}
$(MSILANGPACKTEMPLATEDIR) $(MSILANGPACKTEMPLATEDIR)$/Binary  :
    -$(MKDIRHIER) $@
$(MSILANGPACKTEMPLATEDIR)$/% : $(MSILANGPACKTEMPLATESOURCE)$/%
    $(GNUCOPY) $< $@


msi_sdk_template_files .PHONY :					\
    $(MSISDKOOTEMPLATEDIR)					\
    $(MSISDKOOTEMPLATEDIR)$/Binary				\
    $(MSISDKOOTEMPLATEDIR)$/{$(MSI_SDK_TEMPLATE_FILES)}
$(MSISDKOOTEMPLATEDIR) $(MSISDKOOTEMPLATEDIR)$/Binary :
    -$(MKDIRHIER) $@
$(MSISDKOOTEMPLATEDIR)/% : $(MSISDKOOTEMPLATESOURCE)$/%
    $(GNUCOPY) $< $@


# Local Variables:
# tab-width: 8
# End:
