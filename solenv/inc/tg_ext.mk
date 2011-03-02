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

.EXPORT : CC CXX

# setup INCLUDE variable for use by VC++
.IF "$(GUI)$(COM)"=="WNTMSC"
INCLUDE!:=. $(SOLARINC)
INCLUDE!:=$(INCLUDE:s/ -I/;/)
.EXPORT : INCLUDE
.ENDIF			# "$(GUI)$(COM)"=="WNTMSC"

.IF "$(OS)"=="MACOSX"
LDFLAGS!:=$(EXTRA_LINKFLAGS) $(LDFLAGS)
.EXPORT : LDFLAGS
.ENDIF

.IF "$(GUI)"=="WNT"
PATH!:=.:$(SOLARBINDIR:^"/cygdrive/":s/://):$(PATH)
.ELSE           # "$(GUI)"=="WNT"
PATH!:=.$(PATH_SEPERATOR)$(SOLARBINDIR)$(PATH_SEPERATOR)$(PATH)
.ENDIF          # "$(GUI)"=="WNT"
.EXPORT : PATH

#override
PACKAGE_DIR=$(MISC)/build
ABS_PACKAGE_DIR:=$(MAKEDIR)/$(MISC)/build

#MUST match with PACKAGE_DIR
BACK_PATH=../../../
#MUST match with reference (currently MISC)
MBACK_PATH=../../
.IF "$(TARFILE_IS_FLAT)" != ""
fake_root_dir=/$(TARFILE_NAME)
#MUST match fake_root_dir in directory levels
fake_back=../
.ENDIF "$(TARFILE_IS_FLAT)" != ""

TARFILE_LOCATION*=$(BACK_PATH)$(fake_back)download
TARFILE_LOCATION2=$(eq,$(BACK_PATH)$(fake_back)download,$(TARFILE_LOCATION) $(PRJ)/download $(TARFILE_LOCATION))

P_CONFIGURE_DIR=$(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$(CONFIGURE_DIR)
P_BUILD_DIR=$(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$(BUILD_DIR)
P_INSTALL_DIR=$(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$(BUILD_DIR)
P_INSTALL_TARGET_DIR=$(MISC)/install

.IF "$(PATCH_FILES)"=="none" ||	"$(PATCH_FILES)"==""
NEW_PATCH_FILE_NAME:=$(TARFILE_NAME)
.ELSE			# "$(PATCH_FILES)"=="none" ||	"$(PATCH_FILES)"==""
NEW_PATCH_FILE_NAME:=$(TARFILE_NAME)-newpatch-rename_me.patch
PATCH_FILE_DEP:=$(PRJ)/$(PATH_IN_MODULE)/{$(PATCH_FILES)}
.ENDIF			# "$(PATCH_FILES)"=="none" ||	"$(PATCH_FILES)"==""

unzip_quiet_switch:=-qq
.IF "$(VERBOSE)"=="TRUE"
tar_verbose_switch=v
unzip_quiet_switch:=
.ENDIF			# "$(VERBOSE)"=="TRUE"

.IF "$(ADDITIONAL_FILES)"!=""
P_ADDITIONAL_FILES=$(foreach,i,$(ADDITIONAL_FILES) $(MISC)/$(TARFILE_ROOTDIR)/$i)
T_ADDITIONAL_FILES=$(foreach,i,$(ADDITIONAL_FILES) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$i)
.ENDIF			# "$(ADDITIONAL_FILES)"!=""

EXTRPATH*=OOO

.IF "$(L10N_framework)"==""

ALLTAR : \
    $(PACKAGE_DIR)/$(UNTAR_FLAG_FILE) \
    $(PACKAGE_DIR)/$(BUILD_FLAG_FILE) \
    $(PACKAGE_DIR)/$(INSTALL_FLAG_FILE) \
    $(PACKAGE_DIR)/$(CONFIGURE_FLAG_FILE) \
    $(PACKAGE_DIR)/$(ADD_FILES_FLAG_FILE) \
    $(PACKAGE_DIR)/$(PATCH_FLAG_FILE) \
    $(PACKAGE_DIR)/$(PREDELIVER_FLAG_FILE)

clean:
    $(COMMAND_ECHO)cd $(P_BUILD_DIR) && $(BUILD_ACTION) $(BUILD_FLAGS) clean
    $(COMMAND_ECHO)$(RM) $(PACKAGE_DIR)/$(BUILD_FLAG_FILE)

$(MISC)/%.unpack : $(TARFILE_LOCATION2)/%.tar.bz2
    @-$(RM) $@
.IF "$(GUI)"=="UNX"
    @noop $(assign UNPACKCMD := sh -c "bzip2 -cd $(TARFILE_LOCATION)/$(TARFILE_MD5)-$(TARFILE_NAME).tar.bz2 $(TARFILE_FILTER) | $(GNUTAR) --no-same-owner -x$(tar_verbose_switch)f - ")
.ELSE			# "$(GUI)"=="UNX"
    @noop $(assign UNPACKCMD := bzip2 -cd $(TARFILE_LOCATION)/$(TARFILE_MD5)-$(TARFILE_NAME).tar.bz2 $(TARFILE_FILTER) | $(GNUTAR) --no-same-owner -x$(tar_verbose_switch)f - )
.ENDIF			# "$(GUI)"=="UNX"
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

$(MISC)/%.unpack : $(TARFILE_LOCATION2)/%.tar.Z
    @-$(RM) $@
.IF "$(GUI)"=="UNX"
    @noop $(assign UNPACKCMD := sh -c "uncompress -c $(TARFILE_LOCATION)/$(TARFILE_MD5)-$(TARFILE_NAME).tar.Z | $(GNUTAR) --no-same-owner -x$(tar_verbose_switch)f - ")
.ELSE			# "$(GUI)"=="UNX"
    @noop $(assign UNPACKCMD := uncompress -c $(TARFILE_LOCATION)/$(TARFILE_MD5)-$(TARFILE_NAME).tar.Z | $(GNUTAR) --no-same-owner -x$(tar_verbose_switch)f - )
.ENDIF			# "$(GUI)"=="UNX"
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

$(MISC)/%.unpack : $(TARFILE_LOCATION2)/%.tar.gz
    @-$(RM) $@
    @noop $(assign UNPACKCMD := gzip -d -c $(subst,\,/ $(TARFILE_LOCATION)/$(TARFILE_MD5)-$(TARFILE_NAME).tar.gz) $(TARFILE_FILTER) | $(GNUTAR) --no-same-owner -x$(tar_verbose_switch)f - )
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

$(MISC)/%.unpack : $(TARFILE_LOCATION2)/%.tgz
    @-$(RM) $@
    @noop $(assign UNPACKCMD := gzip -d -c $(subst,\,/ $(TARFILE_LOCATION)/$(TARFILE_MD5)-$(TARFILE_NAME).tgz) $(TARFILE_FILTER) | $(GNUTAR) --no-same-owner -x$(tar_verbose_switch)f - )
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

$(MISC)/%.unpack : $(TARFILE_LOCATION2)/%.tar
    @-$(RM) $@
    @noop $(assign UNPACKCMD := $(GNUTAR) --no-same-owner -x$(tar_verbose_switch)f $(TARFILE_LOCATION)/$(TARFILE_MD5)-$(TARFILE_NAME).tar)
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

$(MISC)/%.unpack : $(TARFILE_LOCATION2)/%.zip
    @-$(RM) $@
    @noop $(assign UNPACKCMD := unzip $(unzip_quiet_switch)  -o $(TARFILE_LOCATION)/$(TARFILE_MD5)-$(TARFILE_NAME).zip)
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

$(MISC)/%.unpack : $(TARFILE_LOCATION2)/%.jar
    @-$(RM) $@
.IF "$(OS)"=="SOLARIS"
    @noop $(assign UNPACKCMD := jar xf $(TARFILE_LOCATION)/$(TARFILE_MD5)-$(TARFILE_NAME).jar)
.ELSE			# "$(OS)"=="SOLARIS"
    @noop $(assign UNPACKCMD := unzip $(unzip_quiet_switch)  -o $(TARFILE_LOCATION)/$(TARFILE_MD5)-$(TARFILE_NAME).jar)
.ENDIF			# "$(OS)"=="SOLARIS"
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

#do unpack
$(PACKAGE_DIR)/$(UNTAR_FLAG_FILE) : $(PRJ)/$(ROUT)/misc/$(TARFILE_MD5)-$(TARFILE_NAME).unpack $(PATCH_FILE_DEP)
    $(IFEXIST) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR).exists $(THEN) $(RENAME:s/+//) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)_removeme $(FI)
    $(COMMAND_ECHO)-rm -rf $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)_removeme
    @-$(MKDIRHIER) $(PACKAGE_DIR)$(fake_root_dir)
    @$(TOUCH) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR).exists # $(IFEXIST) only works with files
    $(COMMAND_ECHO)cd $(PACKAGE_DIR)$(fake_root_dir) && ( $(shell @$(TYPE) $(PRJ)/$(ROUT)/misc/$(TARFILE_MD5)-$(TARFILE_NAME).unpack)) && $(TOUCH) $(UNTAR_FLAG_FILE)
    @echo make writeable...
    @-cd $(PACKAGE_DIR) && chmod -R +rw $(TARFILE_ROOTDIR) && $(TOUCH) $(UNTAR_FLAG_FILE)
    @-cd $(PACKAGE_DIR) && find $(TARFILE_ROOTDIR) -type d -print0 | xargs -0 chmod a+x

#add new files to patch
$(PACKAGE_DIR)/$(ADD_FILES_FLAG_FILE) : $(PACKAGE_DIR)/$(UNTAR_FLAG_FILE) $(T_ADDITIONAL_FILES:+".dummy")
.IF "$(GUI)"=="WNT"
    @$(TOUCH) $@
.ELSE			# "$(GUI)"=="WNT"
    @$(TOUCH) $@
.ENDIF			# "$(GUI)"=="WNT"

#patch
$(PACKAGE_DIR)/$(PATCH_FLAG_FILE) : $(PACKAGE_DIR)/$(ADD_FILES_FLAG_FILE)
.IF "$(PATCH_FILES)"=="none" ||	"$(PATCH_FILES)"==""
    @echo no patch needed...
    $(COMMAND_ECHO)$(TOUCH) $@
.ELSE			# "$(PATCH_FILES)"=="none" ||	"$(PATCH_FILES)"==""
.IF "$(GUI)"=="WNT"
    $(COMMAND_ECHO)cd $(PACKAGE_DIR) && $(TYPE:s/+//) $(BACK_PATH)$(PATH_IN_MODULE)/{$(PATCH_FILES)} | tr -d "\015" | patch $(PATCHFLAGS) -p2 && $(TOUCH) $(PATCH_FLAG_FILE)
.ELSE           # "$(GUI)"=="WNT"
.IF "$(BSCLIENT)"=="TRUE"
    $(COMMAND_ECHO)cd $(PACKAGE_DIR) && $(TYPE) $(BACK_PATH)$(PATH_IN_MODULE)/{$(PATCH_FILES)} | $(GNUPATCH) -f $(PATCHFLAGS) -p2 && $(TOUCH) $(PATCH_FLAG_FILE)
.ELSE           # "$(BSCLIENT)"!=""
    $(COMMAND_ECHO)cd $(PACKAGE_DIR) && $(TYPE) $(BACK_PATH)$(PATH_IN_MODULE)/{$(PATCH_FILES)} | $(GNUPATCH) $(PATCHFLAGS) -p2 && $(TOUCH) $(PATCH_FLAG_FILE)
.ENDIF          # "$(BSCLIENT)"!=""
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF			# "$(PATCH_FILES)"=="none" ||	"$(PATCH_FILES)"==""
.IF "$(T_ADDITIONAL_FILES)"!=""
.IF "$(GUI)"=="WNT"
# Native W32 tools generate only filedates with even seconds, cygwin also with odd seconds
    $(DELAY) 2
.ENDIF # "$(GUI)"=="WNT"
    $(COMMAND_ECHO)$(TOUCH) $(PACKAGE_DIR)/$(PATCH_FLAG_FILE)
.ENDIF          # "$(T_ADDITIONAL_FILES)"!=""

.IF "$(CONVERTFILES)"!=""
$(MISC)/$(TARGET)_convert_unx_flag :  $(PACKAGE_DIR)/$(UNTAR_FLAG_FILE)
    $(COMMAND_ECHO)$(CONVERT) unix $(foreach,i,$(CONVERTFILES) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$i) && $(TOUCH) $(MISC)/$(TARGET)_convert_unx_flag

$(PACKAGE_DIR)/$(PATCH_FLAG_FILE) : $(MISC)/$(TARGET)_convert_unx_flag

$(MISC)/$(TARGET)_convert_dos_flag : $(PACKAGE_DIR)/$(PATCH_FLAG_FILE)
    $(COMMAND_ECHO)$(CONVERT) dos  $(foreach,i,$(CONVERTFILES) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$i) && $(TOUCH) $(MISC)/$(TARGET)_convert_dos_flag

$(PACKAGE_DIR)/$(CONFIGURE_FLAG_FILE) : $(MISC)/$(TARGET)_convert_dos_flag

patch : $(MISC)/$(TARGET)_convert_dos_flag

.ENDIF          # "$(CONVERTFILES)"!=""

$(PACKAGE_DIR)/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)/$(PATCH_FLAG_FILE)
    @@-$(RM) $@
.IF "$(CONFIGURE_ACTION)" == "none" || "$(CONFIGURE_ACTION)"==""
    $(COMMAND_ECHO)$(TOUCH) $(PACKAGE_DIR)/$(CONFIGURE_FLAG_FILE)
.ELSE			# "$(CONFIGURE_ACTION)"=="none" || "$(CONFIGURE_ACTION)"==""
    $(COMMAND_ECHO)-$(MKDIR) $(P_CONFIGURE_DIR)
.IF "$(OS)"=="OS2"
    $(COMMAND_ECHO)cd $(P_CONFIGURE_DIR) && sh -c "$(CONFIGURE_ACTION:s!\!/!) $(CONFIGURE_FLAGS:s!\!/!)" && $(TOUCH) $(CONFIGURE_FLAG_FILE)
.ELSE
    $(COMMAND_ECHO)cd $(P_CONFIGURE_DIR) && $(CONFIGURE_ACTION) $(CONFIGURE_FLAGS) && $(TOUCH) $(CONFIGURE_FLAG_FILE)
.ENDIF
    $(COMMAND_ECHO)mv $(P_CONFIGURE_DIR)/$(CONFIGURE_FLAG_FILE) $(PACKAGE_DIR)/$(CONFIGURE_FLAG_FILE)
.ENDIF			# "$(CONFIGURE_ACTION)"=="none" ||	"$(CONFIGURE_ACTION)"==""


$(PACKAGE_DIR)/$(BUILD_FLAG_FILE) : $(PACKAGE_DIR)/$(CONFIGURE_FLAG_FILE)
    @@-$(RM) $@
.IF "$(eq,x$(BUILD_ACTION:s/none//)x,xx true false)"=="true"
    $(COMMAND_ECHO)$(TOUCH) $(PACKAGE_DIR)/$(BUILD_FLAG_FILE)
.ELSE			# "$(eq,x$(BUILD_ACTION:s/none//)x,xx true false)"=="true"
    $(COMMAND_ECHO)-$(MKDIR) $(P_BUILD_DIR)
    $(COMMAND_ECHO)cd $(P_BUILD_DIR) && $(BUILD_ACTION) $(BUILD_FLAGS) && $(TOUCH) $(ABS_PACKAGE_DIR)/$(BUILD_FLAG_FILE)
.ENDIF			# "$(eq,x$(BUILD_ACTION:s/none//)x,xx true false)"=="true"

$(PACKAGE_DIR)/$(INSTALL_FLAG_FILE) : $(PACKAGE_DIR)/$(BUILD_FLAG_FILE)
    @@-$(RM) $@
.IF "$(INSTALL_ACTION)"=="none" ||	"$(INSTALL_ACTION)"==""
    $(COMMAND_ECHO)$(TOUCH) $(PACKAGE_DIR)/$(INSTALL_FLAG_FILE)
.ELSE			# "$(INSTALL_ACTION)"=="none" ||	"$(INSTALL_ACTION)"==""
    $(COMMAND_ECHO)-$(MKDIR) $(P_INSTALL_DIR)
    $(COMMAND_ECHO)-$(MKDIR) $(P_INSTALL_TARGET_DIR)
    $(COMMAND_ECHO)cd $(P_INSTALL_DIR) && $(INSTALL_ACTION) $(INSTALL_FLAGS) && $(TOUCH) $(INSTALL_FLAG_FILE)
    $(COMMAND_ECHO)mv $(P_INSTALL_DIR)/$(INSTALL_FLAG_FILE) $(PACKAGE_DIR)/$(INSTALL_FLAG_FILE)
.ENDIF			# "$(INSTALL_ACTION)"=="none" ||	"$(INSTALL_ACTION)"==""

$(PACKAGE_DIR)/$(PREDELIVER_FLAG_FILE) : $(PACKAGE_DIR)/$(INSTALL_FLAG_FILE)
.IF "$(OUT2LIB)"!=""
    $(COMMAND_ECHO)$(COPY) $(foreach,i,$(OUT2LIB) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$i) $(LB)
.IF "$(OS)"=="MACOSX"
    $(COMMAND_ECHO)$(PERL) $(SOLARENV)/bin/macosx-change-install-names.pl extshl \
        $(EXTRPATH) \
        $(shell ls $(foreach,j,$(OUT2LIB) $(LB)/$(j:f)) | \
            (grep -v '\.a$$' || test $$? = 1))
.ENDIF
.ENDIF			# "$(OUT2LIB)"!=""
.IF "$(OUT2INC)"!=""
.IF "$(OUT2INC_SUBDIR)"!=""
    $(COMMAND_ECHO)-$(MKDIRHIER) $(INCCOM)/$(OUT2INC_SUBDIR)
    $(COMMAND_ECHO)$(COPY) $(foreach,i,$(OUT2INC) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$i) $(INCCOM)/$(OUT2INC_SUBDIR)
.ELSE          # "$(OUT2INC_SUBDIR)"!=""
    $(COMMAND_ECHO)$(COPY) $(foreach,i,$(OUT2INC) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$i) $(INCCOM)
.ENDIF          # "$(OUT2INC_SUBDIR)"!=""
.ENDIF			# "$(OUT2INC)"!=""
.IF "$(OUTDIR2INC)"!=""
    $(COMMAND_ECHO)$(COPY) $(DEREFERENCE) $(COPYRECURSE) $(foreach,i,$(OUTDIR2INC) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$i) $(INCCOM)
.ENDIF			# "$(OUTDIR2INC)"!=""
.IF "$(OUT2BIN)"!=""
    $(COMMAND_ECHO)$(COPY) $(foreach,i,$(OUT2BIN) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$i) $(BIN)
.IF "$(GUI)$(COM)$(COMEX)"=="WNTMSC12"
    @noop $(foreach,j,$(foreach,k,$(OUT2BIN) \
        $(shell -ls -1 $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$k | $(GREP) .dll)) \
        $(shell @$(IFEXIST) $(j).manifest $(THEN) mt.exe \
        -manifest $(j).manifest -outputresource:$(BIN)/$(j:f)$(EMQ);2 $(FI)))
.ENDIF          # "$(GUI)$(COM)$(COMEX)"=="WNTMSC12"
.ENDIF			# "$(OUT2BIN)"!=""
.IF "$(OUT2CLASS)"!=""
    $(COMMAND_ECHO)$(COPY) $(foreach,i,$(OUT2CLASS) $(PACKAGE_DIR)/$(TARFILE_ROOTDIR)/$i) $(CLASSDIR)
.ENDIF			# "$(OUT2BIN)"!=""
    $(COMMAND_ECHO)$(TOUCH) $(PACKAGE_DIR)/$(PREDELIVER_FLAG_FILE)

$(MISC)/$(TARFILE_ROOTDIR).done : $(MISC)/$(TARFILE_MD5)-$(TARFILE_NAME).unpack $(PATCH_FILES)
    @-mv $(MISC)/$(TARFILE_ROOTDIR) $(MISC)/$(TARFILE_ROOTDIR).old
    @-rm -rf $(MISC)/$(TARFILE_ROOTDIR).old
    @-$(MKDIRHIER) $(MISC)$(fake_root_dir)
    $(COMMAND_ECHO)cd $(MISC)$(fake_root_dir) && $(subst,$(BACK_PATH),$(MBACK_PATH) $(shell @$(TYPE) $(PRJ)/$(ROUT)/misc/$(TARFILE_MD5)-$(TARFILE_NAME).unpack))
.IF "$(P_ADDITIONAL_FILES)"!=""
    noop $(foreach,i,$(P_ADDITIONAL_FILES) $(shell echo dummy > $i))
.ENDIF			 "$(P_ADDITIONAL_FILES)"!=""
.IF "$(PATCH_FILES)"!="none" &&	"$(PATCH_FILES)"!=""
.IF "$(CONVERTFILES)"!=""
    $(CONVERT) unix $(foreach,i,$(CONVERTFILES) $(MISC)/$(TARFILE_ROOTDIR)/$i)
.ENDIF          # "$(CONVERTFILES)"!=""
.IF "$(GUI)"=="WNT"
# hack to make 4nt version 4,01 work and still get propper
# errorcodes for versions < 3,00
#.IF "$(my4ver:s/.//:s/,//)" >= "300"
#	$(COMMAND_ECHO)cd $(MISC) && ( $(TYPE:s/+//) $(BACK_PATH)$(PATH_IN_MODULE)/{$(PATCH_FILES)} | tr -d "\015" | patch $(PATCHFLAGS) -p2 )
#.ELSE			# "$(my4ver:s/.//:s/,//)" >= "300"
    $(COMMAND_ECHO)cd $(MISC) && $(TYPE:s/+//) $(BACK_PATH)$(PATH_IN_MODULE)/{$(PATCH_FILES)} | tr -d "\015" | patch $(PATCHFLAGS) -p2
#.ENDIF			# "$(my4ver:s/.//:s/,//)" >= "300"
.ELSE           # "$(GUI)"=="WNT"
.IF "$(BSCLIENT)"=="TRUE"
    $(COMMAND_ECHO)cd $(MISC) && $(TYPE) $(BACK_PATH)$(PATH_IN_MODULE)/{$(PATCH_FILES)} | $(GNUPATCH) -f $(PATCHFLAGS) -p2
.ELSE           # "$(BSCLIENT)"!=""
    $(COMMAND_ECHO)cd $(MISC) && $(TYPE) $(MBACK_PATH)$(PATH_IN_MODULE)/{$(PATCH_FILES)} | $(GNUPATCH) $(PATCHFLAGS) -p2
.ENDIF          # "$(BSCLIENT)"!=""
.ENDIF          # "$(GUI)"=="WNT"
.IF "$(CONVERTFILES)"!=""
    $(COMMAND_ECHO)$(CONVERT) dos  $(foreach,i,$(CONVERTFILES) $(MISC)/$(TARFILE_ROOTDIR)/$i)
.ENDIF          # "$(CONVERTFILES)"!=""
.ENDIF			# "$(PATCH_FILES)"!="none" && "$(PATCH_FILES)"!="
.IF "$(GUI)"=="UNX"	
    $(COMMAND_ECHO)$(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"

.IF "$(T_ADDITIONAL_FILES)"!=""
$(T_ADDITIONAL_FILES:+".dummy") : $(PACKAGE_DIR)/$(UNTAR_FLAG_FILE)
    @-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)-echo dummy > $@
    $(COMMAND_ECHO)-$(TOUCH) $@
    $(COMMAND_ECHO)-echo dummy > $(@:d)$(@:b)
    $(COMMAND_ECHO)-$(TOUCH) $(@:d)$(@:b)
.ENDIF			 "$(T_ADDITIONAL_FILES)"!=""

create_patch : $(MISC)/$(TARFILE_ROOTDIR).done $(PACKAGE_DIR)/$(PATCH_FLAG_FILE)
    @@-$(MKDIRHIER) $(PRJ)/$(NEW_PATCH_FILE_NAME:d)
    @@-$(RM) $(MISC)/$(NEW_PATCH_FILE_NAME:f).tmp
    @@-$(RM) $(PRJ)/$(PATH_IN_MODULE)/$(NEW_PATCH_FILE_NAME).bak
#ignore returncode of 1 (indicates differences...)
# hard coded again to get the same directory level as before. quite ugly...
    $(COMMAND_ECHO)-cd $(PRJ)/$(ROUT) && diff -ru misc/$(TARFILE_ROOTDIR) misc/build/$(TARFILE_ROOTDIR) | $(PERL) $(SOLARENV)/bin/cleandiff.pl | tr -d "\015" > misc/$(NEW_PATCH_FILE_NAME:f).tmp
    $(COMMAND_ECHO)-mv $(PRJ)/$(PATH_IN_MODULE)/$(NEW_PATCH_FILE_NAME) $(PRJ)/$(PATH_IN_MODULE)/$(NEW_PATCH_FILE_NAME).bak
    $(COMMAND_ECHO)-$(TOUCH) $(PRJ)/$(PATH_IN_MODULE)/$(NEW_PATCH_FILE_NAME).bak
    $(COMMAND_ECHO)$(PERL) $(SOLARENV)/bin/patch_sanitizer.pl $(PRJ)/$(PATH_IN_MODULE)/$(NEW_PATCH_FILE_NAME).bak $(MISC)/$(NEW_PATCH_FILE_NAME:f).tmp $(PRJ)/$(PATH_IN_MODULE)/$(NEW_PATCH_FILE_NAME)
    @@-$(RM) $(MISC)/$(NEW_PATCH_FILE_NAME:f).tmp $(PRJ)/$(PATH_IN_MODULE)/$(NEW_PATCH_FILE_NAME).bak
    $(COMMAND_ECHO)$(MAKECMD) $(MAKEMACROS) patch
    @echo still some problems with win32 generated patches...
    @echo $(USQ)find your new changes in $(NEW_PATCH_FILE_NAME). don't forget to move/rename that patch and insert it in your makefiles PATCH_FILES to activate.$(USQ)

create_clean : $(PACKAGE_DIR)/$(UNTAR_FLAG_FILE)
    @echo done

patch : $(PACKAGE_DIR)/$(PATCH_FLAG_FILE)
    @echo done

.ENDIF			# "$(L10N_framework)"==""
