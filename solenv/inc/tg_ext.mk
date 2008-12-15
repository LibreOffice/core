#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: tg_ext.mk,v $
#
# $Revision: 1.91 $
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
.IF "$(EXT_USE_STLPORT)"==""
INCLUDE!:=$(subst,$/stl, $(SOLARINC))
.ELSE			# "$(EXT_USE_STLPORT)"==""
INCLUDE!:=$(SOLARINC)
.ENDIF			# "$(EXT_USE_STLPORT)"==""
INCLUDE!:=$(INCLUDE:s/ -I/;/)
.EXPORT : INCLUDE
.ENDIF			# "$(GUI)$(COM)"=="WNTMSC"

.IF "$(OS)"=="MACOSX"
LDFLAGS!:=$(EXTRA_LINKFLAGS) $(LDFLAGS)
.EXPORT : LDFLAGS
.ENDIF

#override
PACKAGE_DIR=$(MISC)$/build
ABS_PACKAGE_DIR:=$(MAKEDIR)$/$(MISC)$/build
#MUST match with PACKAGE_DIR
BACK_PATH=..$/..$/..$/

# Remove entire package from output directory, for example, if new patches are
# to be applied.
# is this used at all?
#.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
#	REMOVE_PACKAGE_COMMAND=-$(RM) -r $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR) >& $(NULLDEV)
#.ELSE			# "$(GUI)"=="WNT"
#	REMOVE_PACKAGE_COMMAND=-$(RM) /s $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR) >& $(NULLDEV)
#.ENDIF			# "$(GUI)"=="WNT"

P_CONFIGURE_DIR=$(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$(CONFIGURE_DIR)
P_BUILD_DIR=$(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$(BUILD_DIR)
P_INSTALL_DIR=$(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$(BUILD_DIR)
P_INSTALL_TARGET_DIR=$(MISC)$/install

.IF "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""
NEW_PATCH_FILE_NAME:=$(TARFILE_NAME)
.ELSE			# "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""
NEW_PATCH_FILE_NAME:=$(PATCH_FILE_NAME)
PATCH_FILE_DEP:=$(PRJ)$/$(PATH_IN_MODULE)$/$(NEW_PATCH_FILE_NAME)
.ENDIF			# "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""

.IF "$(TAR_EXCLUDES)"!=""
TAR_EXCLUDE_SWITCH=--exclude=$(TAR_EXCLUDES)
.ENDIF          # "$(TAR_EXCLUDES)"!=""

unzip_quiet_switch:=-qq
.IF "$(VERBOSE)"=="TRUE"
tar_verbose_switch=v
unzip_quiet_switch:=
.ENDIF			# "$(VERBOSE)"=="TRUE"

.IF "$(ADDITIONAL_FILES)"!=""
P_ADDITIONAL_FILES=$(foreach,i,$(ADDITIONAL_FILES) $(MISC)$/$(TARFILE_ROOTDIR)$/$i)
T_ADDITIONAL_FILES=$(foreach,i,$(ADDITIONAL_FILES) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i)
.ENDIF			# "$(ADDITIONAL_FILES)"!=""

EXTRPATH*=OOO

.IF "$(L10N_framework)"==""

ALLTAR : \
    $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE) \
    $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE) \
    $(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE) \
    $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) \
    $(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE) \
    $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE) \
    $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)

clean:
    cd $(P_BUILD_DIR) && $(BUILD_ACTION) $(BUILD_FLAGS) clean
    $(RM) $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)

$(MISC)$/%.unpack : $(PRJ)$/download$/%.tar.bz2
    @-$(RM) $@
.IF "$(GUI)"=="UNX"
    @noop $(assign UNPACKCMD := sh -c "bzip2 -cd $(BACK_PATH)download$/$(TARFILE_NAME).tar.bz2 $(TARFILE_FILTER) | tar $(TAR_EXCLUDE_SWITCH) -x$(tar_verbose_switch)f - ")
.ELSE			# "$(GUI)"=="UNX"
    @noop $(assign UNPACKCMD := bzip2 -cd $(BACK_PATH)download$/$(TARFILE_NAME).tar.bz2 $(TARFILE_FILTER) | tar $(TAR_EXCLUDE_SWITCH) -x$(tar_verbose_switch)f - )
.ENDIF			# "$(GUI)"=="UNX"
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

$(MISC)$/%.unpack : $(PRJ)$/download$/%.tar.Z
    @-$(RM) $@
.IF "$(GUI)"=="UNX"
    @noop $(assign UNPACKCMD := sh -c "uncompress -c $(BACK_PATH)download$/$(TARFILE_NAME).tar.Z | tar $(TAR_EXCLUDE_SWITCH) -x$(tar_verbose_switch)f - ")
.ELSE			# "$(GUI)"=="UNX"
    @noop $(assign UNPACKCMD := uncompress -c $(BACK_PATH)download$/$(TARFILE_NAME).tar.Z | tar $(TAR_EXCLUDE_SWITCH) -x$(tar_verbose_switch)f - )
.ENDIF			# "$(GUI)"=="UNX"
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

$(MISC)$/%.unpack : $(PRJ)$/download$/%.tar.gz
    @-$(RM) $@
    @noop $(assign UNPACKCMD := gzip -d -c $(BACK_PATH)download$/$(TARFILE_NAME).tar.gz $(TARFILE_FILTER) | tar $(TAR_EXCLUDE_SWITCH) -x$(tar_verbose_switch)f - )
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

$(MISC)$/%.unpack : $(PRJ)$/download$/%.tgz
    @-$(RM) $@
    @noop $(assign UNPACKCMD := gzip -d -c $(BACK_PATH)download$/$(TARFILE_NAME).tgz $(TARFILE_FILTER) | tar $(TAR_EXCLUDE_SWITCH) -x$(tar_verbose_switch)f - )
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

$(MISC)$/%.unpack : $(PRJ)$/download$/%.tar
    @-$(RM) $@
    noop $(assign UNPACKCMD := tar $(TAR_EXCLUDE_SWITCH) -x$(tar_verbose_switch)f $(BACK_PATH)download$/$(TARFILE_NAME).tar)
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

$(MISC)$/%.unpack : $(PRJ)$/download$/%.zip
    @-$(RM) $@
    noop $(assign UNPACKCMD := unzip $(unzip_quiet_switch)  -o $(BACK_PATH)download$/$(TARFILE_NAME).zip)
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

$(MISC)$/%.unpack : $(PRJ)$/download$/%.jar
    @-$(RM) $@
.IF "$(OS)"=="SOLARIS"
    noop $(assign UNPACKCMD := jar xf $(BACK_PATH)download$/$(TARFILE_NAME).jar)
.ELSE			# "$(OS)"=="SOLARIS"
    noop $(assign UNPACKCMD := unzip $(unzip_quiet_switch)  -o $(BACK_PATH)download$/$(TARFILE_NAME).jar)
.ENDIF			# "$(OS)"=="SOLARIS"
    @$(TYPE) $(mktmp $(UNPACKCMD)) > $@.$(INPATH)
    @$(RENAME) $@.$(INPATH) $@

#do unpack
$(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE) : $(PRJ)$/$(ROUT)$/misc$/$(TARFILE_NAME).unpack $(PATCH_FILE_DEP)
    $(IFEXIST) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR) $(THEN) $(RENAME:s/+//) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)_removeme $(FI)
    -rm -rf $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)_removeme
    @-$(MKDIRHIER) $(PACKAGE_DIR)
    cd $(PACKAGE_DIR) && ( $(shell @$(TYPE) $(PRJ)$/$(ROUT)$/misc$/$(TARFILE_NAME).unpack)) && $(TOUCH) $(UNTAR_FLAG_FILE)
    @echo make writeable...
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    @cd $(PACKAGE_DIR) && chmod -R +rw $(TARFILE_ROOTDIR) && $(TOUCH) $(UNTAR_FLAG_FILE)
    @cd $(PACKAGE_DIR) && find $(TARFILE_ROOTDIR) -type d -exec chmod a+x {{}} \;
.ELSE			# "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
# Native W32 tools generate only filedates with even seconds, cygwin also with odd seconds
    $(DELAY) 2
    @cd $(PACKAGE_DIR) && attrib /s -r $(TARFILE_ROOTDIR) >& $(NULLDEV) && $(TOUCH) $(UNTAR_FLAG_FILE)
.ENDIF			# "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"

#add new files to patch
$(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE) : $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE) $(T_ADDITIONAL_FILES:+".dummy")
.IF "$(GUI)"=="WNT"
    @$(TOUCH) $@
.ELSE			# "$(GUI)"=="WNT"
    @$(TOUCH) $@
.ENDIF			# "$(GUI)"=="WNT"

#patch
$(PACKAGE_DIR)$/$(PATCH_FLAG_FILE) : $(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE)
.IF "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""
    @echo no patch needed...
    $(TOUCH) $@
.ELSE			# "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""
.IF "$(GUI)"=="WNT"
# hack to make 4nt version 4,01 work and still get propper
# errorcodes for versions < 3,00
.IF "$(my4ver:s/.//:s/,//)" >= "300"
    cd $(PACKAGE_DIR) && ( $(TYPE:s/+//) $(BACK_PATH)$(PATH_IN_MODULE)$/$(PATCH_FILE_NAME) | tr -d "\015" | patch $(PATCHFLAGS) -p2 ) && $(TOUCH) $(PATCH_FLAG_FILE)
.ELSE			# "$(my4ver:s/.//:s/,//)" >= "300"
    cd $(PACKAGE_DIR) && $(TYPE:s/+//) $(BACK_PATH)$(PATH_IN_MODULE)$/$(PATCH_FILE_NAME) | tr -d "\015" | patch $(PATCHFLAGS) -p2 && $(TOUCH) $(PATCH_FLAG_FILE)
.ENDIF			# "$(my4ver:s/.//:s/,//)" >= "300"
.ELSE           # "$(GUI)"=="WNT"
.IF "$(BSCLIENT)"=="TRUE"
    cd $(PACKAGE_DIR) && $(TYPE) $(BACK_PATH)$(PATH_IN_MODULE)$/$(PATCH_FILE_NAME) | $(GNUPATCH) -f $(PATCHFLAGS) -p2 && $(TOUCH) $(PATCH_FLAG_FILE)
.ELSE           # "$(BSCLIENT)"!=""
    cd $(PACKAGE_DIR) && $(TYPE) $(BACK_PATH)$(PATH_IN_MODULE)$/$(PATCH_FILE_NAME) | $(GNUPATCH) $(PATCHFLAGS) -p2 && $(TOUCH) $(PATCH_FLAG_FILE)
.ENDIF          # "$(BSCLIENT)"!=""
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF			# "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""
.IF "$(T_ADDITIONAL_FILES)"!=""
.IF "$(GUI)"=="WNT"
# Native W32 tools generate only filedates with even seconds, cygwin also with odd seconds
    $(DELAY) 2
.ENDIF # "$(GUI)"=="WNT"
    $(TOUCH) $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
.ENDIF          # "$(T_ADDITIONAL_FILES)"!=""

.IF "$(CONVERTFILES)"!=""
$(MISC)$/$(TARGET)_convert_unx_flag :  $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)
    $(CONVERT) unix $(foreach,i,$(CONVERTFILES) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) && $(TOUCH) $(MISC)$/$(TARGET)_convert_unx_flag

$(PACKAGE_DIR)$/$(PATCH_FLAG_FILE) : $(MISC)$/$(TARGET)_convert_unx_flag

$(MISC)$/$(TARGET)_convert_dos_flag : $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
    $(CONVERT) dos  $(foreach,i,$(CONVERTFILES) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) && $(TOUCH) $(MISC)$/$(TARGET)_convert_dos_flag

$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(MISC)$/$(TARGET)_convert_dos_flag

patch : $(MISC)$/$(TARGET)_convert_dos_flag

.ENDIF          # "$(CONVERTFILES)"!=""

$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
    @@-$(RM) $@
.IF "$(CONFIGURE_ACTION)" == "none" || "$(CONFIGURE_ACTION)"==""
    $(TOUCH) $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE)
.ELSE			# "$(CONFIGURE_ACTION)"=="none" || "$(CONFIGURE_ACTION)"==""
    -$(MKDIR) $(P_CONFIGURE_DIR)
.IF "$(OS)"=="OS2"
    cd $(P_CONFIGURE_DIR) && sh -c "$(CONFIGURE_ACTION:s!\!/!) $(CONFIGURE_FLAGS:s!\!/!)" && $(TOUCH) $(CONFIGURE_FLAG_FILE)
.ELSE
    cd $(P_CONFIGURE_DIR) && $(CONFIGURE_ACTION) $(CONFIGURE_FLAGS) && $(TOUCH) $(CONFIGURE_FLAG_FILE)
.ENDIF
    mv $(P_CONFIGURE_DIR)$/$(CONFIGURE_FLAG_FILE) $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE)
.ENDIF			# "$(CONFIGURE_ACTION)"=="none" ||	"$(CONFIGURE_ACTION)"==""


$(PACKAGE_DIR)$/$(BUILD_FLAG_FILE) : $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE)
    @@-$(RM) $@
.IF "$(eq,x$(BUILD_ACTION:s/none//)x,xx true false)"=="true"
    $(TOUCH) $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
.ELSE			# "$(eq,x$(BUILD_ACTION:s/none//)x,xx true false)"=="true"
    -$(MKDIR) $(P_BUILD_DIR)
    cd $(P_BUILD_DIR) && $(BUILD_ACTION) $(BUILD_FLAGS) && $(TOUCH) $(ABS_PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
.ENDIF			# "$(eq,x$(BUILD_ACTION:s/none//)x,xx true false)"=="true"

$(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE) : $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
    @@-$(RM) $@
.IF "$(INSTALL_ACTION)"=="none" ||	"$(INSTALL_ACTION)"==""
    $(TOUCH) $(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE)
.ELSE			# "$(INSTALL_ACTION)"=="none" ||	"$(INSTALL_ACTION)"==""
    -$(MKDIR) $(P_INSTALL_DIR)
    -$(MKDIR) $(P_INSTALL_TARGET_DIR)
    cd $(P_INSTALL_DIR) && $(INSTALL_ACTION) $(INSTALL_FLAGS) && $(TOUCH) $(INSTALL_FLAG_FILE)
    mv $(P_INSTALL_DIR)$/$(INSTALL_FLAG_FILE) $(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE)
.ENDIF			# "$(INSTALL_ACTION)"=="none" ||	"$(INSTALL_ACTION)"==""

$(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) : $(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE)
.IF "$(OUT2LIB)"!=""
    $(COPY) $(foreach,i,$(OUT2LIB) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) $(LB)
.IF "$(OS)"=="MACOSX"
    $(PERL) $(SOLARENV)$/bin$/macosx-change-install-names.pl extshl \
        $(EXTRPATH) \
        $(shell ls $(foreach,j,$(OUT2LIB) $(LB)$/$(j:f)) | \
            (grep -v '\.a$$' || test $$? = 1))
.ENDIF
.ENDIF			# "$(OUT2LIB)"!=""
.IF "$(OUT2INC)"!=""
.IF "$(OUT2INC_SUBDIR)"!=""
    -$(MKDIR) $(INCCOM)$/$(OUT2INC_SUBDIR)
    $(COPY) $(foreach,i,$(OUT2INC) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) $(INCCOM)$/$(OUT2INC_SUBDIR)
.ELSE          # "$(OUT2INC_SUBDIR)"!=""
    $(COPY) $(foreach,i,$(OUT2INC) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) $(INCCOM)
.ENDIF          # "$(OUT2INC_SUBDIR)"!=""
.ENDIF			# "$(OUT2INC)"!=""
.IF "$(OUTDIR2INC)"!=""
.IF "$(USE_SHELL)"=="4nt"
    @-$(MKDIR) $(foreach,i,$(OUTDIR2INC) $(INCCOM)$/$(i:b))
    @echo copied $(foreach,i,$(OUTDIR2INC) $(shell @$(COPY) $(COPYRECURSE) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i$/* $(INCCOM)$/$(i:b) >& $(NULLDEV) && echo $i))
.ELSE			# "$(USE_SHELL)"=="4nt"
    $(COPY) $(COPYRECURSE) $(foreach,i,$(OUTDIR2INC) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) $(INCCOM)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(OUTDIR2INC)"!=""
.IF "$(OUT2BIN)"!=""
    $(COPY) $(foreach,i,$(OUT2BIN) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) $(BIN)
.IF "$(GUI)$(COM)$(COMEX)"=="WNTMSC12"
    @noop $(foreach,j,$(foreach,k,$(OUT2BIN) \
        $(shell -ls -1 $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$k | $(GREP) .dll)) \
        $(shell @$(IFEXIST) $(j).manifest $(THEN) mt.exe \
        -manifest $(j).manifest -outputresource:$(BIN)$/$(j:f)$(EMQ);2 $(FI)))
.ENDIF          # "$(GUI)$(COM)$(COMEX)"=="WNTMSC12"
.ENDIF			# "$(OUT2BIN)"!=""
.IF "$(OUT2CLASS)"!=""
    $(COPY) $(foreach,i,$(OUT2CLASS) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) $(CLASSDIR)
.ENDIF			# "$(OUT2BIN)"!=""
    $(TOUCH) $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)

$(MISC)$/$(TARFILE_ROOTDIR) : $(MISC)$/$(TARFILE_NAME).unpack
    @-mv $(MISC)$/$(TARFILE_ROOTDIR) $(MISC)$/$(TARFILE_ROOTDIR).old
    @-rm -rf $(MISC)$/$(TARFILE_ROOTDIR).old
    cd $(MISC) && $(subst,$(BACK_PATH),..$/..$/ $(shell @$(TYPE) $(PRJ)$/$(ROUT)$/misc$/$(TARFILE_NAME).unpack))
.IF "$(GUI)"=="UNX"	
    $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"	


.IF "$(P_ADDITIONAL_FILES)"!=""
$(P_ADDITIONAL_FILES) :
    @-$(MKDIRHIER) $(@:d)
    -echo dummy > $@
    -$(TOUCH) $@
.ENDIF			 "$(P_ADDITIONAL_FILES)"!=""

.IF "$(T_ADDITIONAL_FILES)"!=""
$(T_ADDITIONAL_FILES:+".dummy") : $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)
    @-$(MKDIRHIER) $(@:d)
    -echo dummy > $@
    -$(TOUCH) $@
    -echo dummy > $(@:d)$(@:b)
    -$(TOUCH) $(@:d)$(@:b)
.ENDIF			 "$(T_ADDITIONAL_FILES)"!=""

create_patch : $(MISC)$/$(TARFILE_ROOTDIR) $(P_ADDITIONAL_FILES) $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
    @@-$(MKDIRHIER) $(PRJ)$/$(NEW_PATCH_FILE_NAME:d)
    @@-$(RM) $(MISC)$/$(NEW_PATCH_FILE_NAME:f).tmp
    @@-$(RM) $(PRJ)$/$(PATH_IN_MODULE)$/$(NEW_PATCH_FILE_NAME).bak
#ignore returncode of 1 (indicates differences...)	
# hard coded again to get the same directory level as before. quite ugly...
    -cd $(PRJ)$/$(ROUT) && diff -ru misc$/$(TARFILE_ROOTDIR) misc$/build$/$(TARFILE_ROOTDIR) | $(PERL) $(SOLARENV)$/bin$/cleandiff.pl | tr -d "\015" > misc$/$(NEW_PATCH_FILE_NAME:f).tmp
    -mv $(PRJ)$/$(PATH_IN_MODULE)$/$(NEW_PATCH_FILE_NAME) $(PRJ)$/$(PATH_IN_MODULE)$/$(NEW_PATCH_FILE_NAME).bak
    $(PERL) $(SOLARENV)$/bin$/patch_sanitizer.pl $(PRJ)$/$(PATH_IN_MODULE)$/$(NEW_PATCH_FILE_NAME).bak $(MISC)$/$(NEW_PATCH_FILE_NAME:f).tmp $(PRJ)$/$(PATH_IN_MODULE)$/$(NEW_PATCH_FILE_NAME)
    @@-$(RM) $(MISC)$/$(NEW_PATCH_FILE_NAME:f).tmp
    $(MAKECMD) $(MAKEMACROS) patch
    @echo still some problems with win32 generated patches...

create_clean : $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)
    @echo done

patch : $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
    @echo done

.ENDIF			# "$(L10N_framework)"==""
