#*************************************************************************
#
#   $RCSfile: tg_ext.mk,v $
#
#   $Revision: 1.47 $
#
#   last change: $Author: hjs $ $Date: 2003-08-18 14:49:11 $
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

.IF "$(L10N_framework)"==""

.EXPORT : CC CXX

# setup INCLUDE variable for use by VC++
.IF "$(GUI)"=="WNT"
.IF "$(USE_SHELL)"=="4nt"
.IF "$(EXT_USE_STLPORT)"==""
INCLUDE!:=$(shell echo $(INCLUDE:s/\stl//) | sed "s/[ \t]*-I/;/g" )
.ELSE			# "$(EXT_USE_STLPORT)"==""
INCLUDE!:=$(shell echo $(INCLUDE) | sed "s/[ \t]*-I/;/g" )
.ENDIF			# "$(EXT_USE_STLPORT)"==""
.ELSE			# "$(USE_SHELL)"=="4nt"
.IF "$(EXT_USE_STLPORT)"==""
INCLUDE!:=$(shell echo "$(EXT_INCLUDE:s/\stl//)" )
.ELSE			# "$(EXT_USE_STLPORT)"==""
INCLUDE!:=$(shell echo "$(EXT_INCLUDE)")
.ENDIF			# "$(EXT_USE_STLPORT)"==""
.ENDIF			# "$(USE_SHELL)"=="4nt"
.EXPORT : INCLUDE
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(OS)"!="NETBSD"
.IF "$(OS)"!="FREEBSD"
PATCHFLAGS=-b
.ENDIF			# "$(OS)"=="FREEBSD"
.ENDIF			# "$(OS)"=="NETBSD"

#override
PACKAGE_DIR=$(MISC)$/build
#MUST match with PACKAGE_DIR
BACK_PATH=..$/..$/..$/

# Remove entire package from output directory, for example, if new patches are
# to be applied.
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    REMOVE_PACKAGE_COMMAND=+-$(RM) -r $(PACKAGE_DIR) >& $(NULLDEV)
.ELSE			# "$(GUI)"=="WNT"
    REMOVE_PACKAGE_COMMAND=+-$(RM) /s $(PACKAGE_DIR) >& $(NULLDEV)
.ENDIF			# "$(GUI)"=="WNT"

P_CONFIGURE_DIR=$(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$(CONFIGURE_DIR)
P_BUILD_DIR=$(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$(BUILD_DIR)
P_INSTALL_DIR=$(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$(BUILD_DIR)
P_INSTALL_TARGET_DIR=$(MISC)$/install

.IF "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""
NEW_PATCH_FILE_NAME:=$(TARFILE_NAME)
.ELSE			# "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""
NEW_PATCH_FILE_NAME:=$(PATCH_FILE_NAME)
.ENDIF			# "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""

.IF "$(TAR_EXCLUDES)"!=""
TAR_EXCLUDE_SWITCH=--exclude=$(TAR_EXCLUDES)
.ENDIF          # "$(TAR_EXCLUDES)"!=""

#clean PWD to let a build_action=dmake set it with new value
.IF "$(GUI)"=="WNT"
PWD:=
.EXPORT : PWD
.ENDIF          # "$(GUI)"=="WNT"

.IF "$(ADDITIONAL_FILES)"!=""
P_ADDITIONAL_FILES=$(foreach,i,$(ADDITIONAL_FILES) $(MISC)$/$(TARFILE_ROOTDIR)$/$i)
T_ADDITIONAL_FILES=$(foreach,i,$(ADDITIONAL_FILES) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i)
.ENDIF			# "$(ADDITIONAL_FILES)"!=""


ALLTAR : \
    $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE) \
    $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE) \
    $(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE) \
    $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) \
    $(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE) \
    $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE) \
    $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)

clean:
    +cd $(P_BUILD_DIR) && $(BUILD_ACTION) $(BUILD_FLAGS) clean
    +$(RM) $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
    
$(MISC)$/%.unpack : $(PRJ)$/download$/%.tar.Z
    @+-$(RM) $@
.IF "$(GUI)"=="UNX"
    @+echo $(assign UNPACKCMD := sh -c "uncompress -c $(BACK_PATH)download$/$(TARFILE_NAME).tar.Z | tar $(TAR_EXCLUDE_SWITCH) -xvf - ") > $(NULLDEV)
.ELSE			# "$(GUI)"=="UNX"
    @+echo $(assign UNPACKCMD := uncompress -c $(BACK_PATH)download$/$(TARFILE_NAME).tar.Z | tar $(TAR_EXCLUDE_SWITCH) -xvf - ) > $(NULLDEV)
.ENDIF			# "$(GUI)"=="UNX"
    @+$(COPY) $(mktmp $(UNPACKCMD)) $@

$(MISC)$/%.unpack : $(PRJ)$/download$/%.tar.gz
    @+-$(RM) $@
.IF "$(GUI)"=="UNX"
    @+echo $(assign UNPACKCMD := sh -c "gunzip -c $(BACK_PATH)download$/$(TARFILE_NAME).tar.gz $(TARFILE_FILTER) | tar $(TAR_EXCLUDE_SWITCH) -xvf - ") > $(NULLDEV)
.ELSE			# "$(GUI)"=="UNX"
    @+echo $(assign UNPACKCMD := gunzip -c $(BACK_PATH)download$/$(TARFILE_NAME).tar.gz $(TARFILE_FILTER) | tar $(TAR_EXCLUDE_SWITCH) -xvf - ) > $(NULLDEV)
.ENDIF			# "$(GUI)"=="UNX"
    @+$(COPY) $(mktmp $(UNPACKCMD)) $@
    
$(MISC)$/%.unpack : $(PRJ)$/download$/%.tgz
    @+-$(RM) $@
.IF "$(GUI)"=="UNX"
    @+echo $(assign UNPACKCMD := sh -c "gunzip -c $(BACK_PATH)download$/$(TARFILE_NAME).tgz $(TARFILE_FILTER) | tar $(TAR_EXCLUDE_SWITCH) -xvf - ") > $(NULLDEV)
.ELSE			# "$(GUI)"=="UNX"
    @+echo $(assign UNPACKCMD := gunzip -c $(BACK_PATH)download$/$(TARFILE_NAME).tgz $(TARFILE_FILTER) | tar $(TAR_EXCLUDE_SWITCH) -xvf - ) > $(NULLDEV)
.ENDIF			# "$(GUI)"=="UNX"
    @+$(COPY) $(mktmp $(UNPACKCMD)) $@

$(MISC)$/%.unpack : $(PRJ)$/download$/%.tar
    @+-$(RM) $@
    +echo $(assign UNPACKCMD := tar $(TAR_EXCLUDE_SWITCH) -xvf $(BACK_PATH)download$/$(TARFILE_NAME).tar) > $(NULLDEV)
    @+$(COPY) $(mktmp $(UNPACKCMD)) $@

$(MISC)$/%.unpack : $(PRJ)$/download$/%.zip
    @+-$(RM) $@
    +echo $(assign UNPACKCMD := unzip $(BACK_PATH)download$/$(TARFILE_NAME).zip) > $(NULLDEV)
    @+$(COPY) $(mktmp $(UNPACKCMD)) $@

#untar
$(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE) : $(PRJ)$/$(ROUT)$/misc$/$(TARFILE_NAME).unpack
    @+-$(MKDIR) $(PACKAGE_DIR:d)
    @+-$(MKDIR) $(PACKAGE_DIR)
    +cd $(PACKAGE_DIR) && ( $(shell +$(TYPE) $(PRJ)$/$(ROUT)$/misc$/$(TARFILE_NAME).unpack)) && $(TOUCH) $(UNTAR_FLAG_FILE)
    @+echo make writeable...
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    @+cd $(PACKAGE_DIR) && chmod -R +rw * && $(TOUCH) $(UNTAR_FLAG_FILE)
    @+cd $(PACKAGE_DIR) && find . -type d -exec chmod a+x {{}} \;
.ELSE			# "$(GUI)"=="WNT"
    @+cd $(PACKAGE_DIR) && attrib /s -r  >& $(NULLDEV) && $(TOUCH) $(UNTAR_FLAG_FILE)
.ENDIF			# "$(GUI)"=="WNT"


#add new files to patch
$(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE) : $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE) $(T_ADDITIONAL_FILES)
.IF "$(GUI)"=="WNT"
    @$(TOUCH) $@
.ELSE			# "$(GUI)"=="WNT"
    @$(TOUCH) $@
.ENDIF			# "$(GUI)"=="WNT"

#patch
$(PACKAGE_DIR)$/$(PATCH_FLAG_FILE) : $(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE)
.IF "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""
    +cd $(PACKAGE_DIR) && echo no patch needed...
    +$(TOUCH) $@
.ELSE			# "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""
.IF "$(GUI)"=="WNT"
# hack to make 4nt version 4,01 work and still get propper
# errorcodes for versions < 3,00
.IF "$(my4ver:s/.//:s/,//)" >= "300"
    +cd $(PACKAGE_DIR) && ( $(TYPE) $(BACK_PATH)$(PATCH_FILE_NAME) | tr -d "\015" | patch $(PATCHFLAGS) -p2 ) && $(TOUCH) $(PATCH_FLAG_FILE)
.ELSE			# "$(my4ver:s/.//:s/,//)" >= "300"
    +cd $(PACKAGE_DIR) && $(TYPE) $(BACK_PATH)$(PATCH_FILE_NAME) | tr -d "\015" | patch $(PATCHFLAGS) -p2 && $(TOUCH) $(PATCH_FLAG_FILE)
.ENDIF			# "$(my4ver:s/.//:s/,//)" >= "300"
.ELSE           # "$(GUI)"=="WNT"
.IF "$(BSCLIENT)"=="TRUE"
    +cd $(PACKAGE_DIR) && $(TYPE) $(BACK_PATH)$(PATCH_FILE_NAME) | $(GNUPATCH) -f $(PATCHFLAGS) -p2 && $(TOUCH) $(PATCH_FLAG_FILE)
.ELSE           # "$(BSCLIENT)"!=""
    +cd $(PACKAGE_DIR) && $(TYPE) $(BACK_PATH)$(PATCH_FILE_NAME) | $(GNUPATCH) $(PATCHFLAGS) -p2 && $(TOUCH) $(PATCH_FLAG_FILE)
.ENDIF          # "$(BSCLIENT)"!=""
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF			# "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""
.IF "$(T_ADDITIONAL_FILES)"!=""
    +$(TOUCH) $(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE)
    +$(TOUCH) $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
.ENDIF          # "$(T_ADDITIONAL_FILES)"!=""

$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(CONFIGURE_ACTION)" == "none" || "$(CONFIGURE_ACTION)"==""
    +$(TOUCH) $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE)
.ELSE			# "$(CONFIGURE_ACTION)"=="none" || "$(CONFIGURE_ACTION)"==""
    +-$(MKDIR) $(P_CONFIGURE_DIR)
    +cd $(P_CONFIGURE_DIR) && $(CONFIGURE_ACTION) $(CONFIGURE_FLAGS) && $(TOUCH) $(CONFIGURE_FLAG_FILE)
    +mv $(P_CONFIGURE_DIR)$/$(CONFIGURE_FLAG_FILE) $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE)
.ENDIF			# "$(CONFIGURE_ACTION)"=="none" ||	"$(CONFIGURE_ACTION)"==""

    
$(PACKAGE_DIR)$/$(BUILD_FLAG_FILE) : $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(BUILD_ACTION)"=="none" ||	"$(BUILD_ACTION)"==""
    +$(TOUCH) $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
.ELSE			# "$(BUILD_ACTION)"=="none" ||	"$(BUILD_ACTION)"==""
    +-$(MKDIR) $(P_BUILD_DIR)
    +cd $(P_BUILD_DIR) && $(BUILD_ACTION) $(BUILD_FLAGS) && $(TOUCH) $(BUILD_FLAG_FILE)
    +mv $(P_BUILD_DIR)$/$(BUILD_FLAG_FILE) $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
.ENDIF			# "$(BUILD_ACTION)"=="none" ||	"$(BUILD_ACTION)"==""

$(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE) : $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(INSTALL_ACTION)"=="none" ||	"$(INSTALL_ACTION)"==""
    +$(TOUCH) $(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE)
.ELSE			# "$(INSTALL_ACTION)"=="none" ||	"$(INSTALL_ACTION)"==""
    +-$(MKDIR) $(P_INSTALL_DIR)
    +-$(MKDIR) $(P_INSTALL_TARGET_DIR)
    +cd $(P_INSTALL_DIR) && $(INSTALL_ACTION) $(INSTALL_FLAGS) && $(TOUCH) $(INSTALL_FLAG_FILE)
    +mv $(P_INSTALL_DIR)$/$(INSTALL_FLAG_FILE) $(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE)
.ENDIF			# "$(INSTALL_ACTION)"=="none" ||	"$(INSTALL_ACTION)"==""

$(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) : $(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE)
.IF "$(OUT2LIB)"!=""
    +$(COPY) $(foreach,i,$(OUT2LIB) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) $(LB)
.ENDIF			# "$(OUT2LIB)"!=""
.IF "$(OUT2INC)"!=""
    +$(COPY) $(foreach,i,$(OUT2INC) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) $(INCCOM)
.ENDIF			# "$(OUT2INC)"!=""
.IF "$(OUTDIR2INC)"!=""
.IF "$(GUI)"=="WNT"
    @$(MKDIR) $(foreach,i,$(OUTDIR2INC) $(INCCOM)$/$(i:b))
    @+echo copied $(foreach,i,$(OUTDIR2INC) $(shell +$(COPY) $(COPYRECURSE) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i$/* $(INCCOM)$/$(i:b) >& $(NULLDEV) && echo $i))
.ELSE			# "$(GUI)"=="WNT"
    +$(COPY) $(COPYRECURSE) $(foreach,i,$(OUTDIR2INC) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) $(INCCOM)
.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(OUTDIR2INC)"!=""
.IF "$(OUT2BIN)"!=""
    +$(COPY) $(foreach,i,$(OUT2BIN) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) $(BIN)
.ENDIF			# "$(OUT2BIN)"!=""
.IF "$(OUT2CLASS)"!=""
    +$(COPY) $(foreach,i,$(OUT2CLASS) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i) $(CLASSDIR)
.ENDIF			# "$(OUT2BIN)"!=""
    +$(TOUCH) $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)

$(MISC)$/$(TARFILE_ROOTDIR) : $(MISC)$/$(TARFILE_NAME).unpack
    @+-mv $(MISC)$/$(TARFILE_ROOTDIR) $(MISC)$/$(TARFILE_ROOTDIR).old
    @+-rm -rf $(MISC)$/$(TARFILE_ROOTDIR).old
    +cd $(MISC) && $(subst,$(BACK_PATH),..$/..$/ $(shell +$(TYPE) $(PRJ)$/$(ROUT)$/misc$/$(TARFILE_NAME).unpack))
.IF "$(GUI)"=="UNX"	
    +$(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"	


.IF "$(P_ADDITIONAL_FILES)"!=""
$(P_ADDITIONAL_FILES) :
    +-echo dummy > $@
    +-$(TOUCH) $@
.ENDIF			 "$(P_ADDITIONAL_FILES)"!=""

.IF "$(T_ADDITIONAL_FILES)"!=""
$(T_ADDITIONAL_FILES) : $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)
    +-echo dummy > $@
    +-$(TOUCH) $@
.ENDIF			 "$(T_ADDITIONAL_FILES)"!=""

create_patch : $(MISC)$/$(TARFILE_ROOTDIR) $(P_ADDITIONAL_FILES)
    @+-$(RM) $(MISC)$/$(NEW_PATCH_FILE_NAME).tmp >& $(NULLDEV)
    @+-$(RM) $(NEW_PATCH_FILE_NAME).bak >& $(NULLDEV)
#ignore returncode of 1 (indicates differences...)	
# hard coded again to get the same directory level as before. quite ugly...
    +-cd $(ROUT) && diff -rc misc$/$(TARFILE_ROOTDIR) misc$/build$/$(TARFILE_ROOTDIR) | $(PERL) $(SOLARENV)$/bin$/cleandiff.pl | tr -d "\015" > misc$/$(NEW_PATCH_FILE_NAME).tmp
    @+-mv $(NEW_PATCH_FILE_NAME) $(NEW_PATCH_FILE_NAME).bak >& $(NULLDEV)
    @+-mv $(MISC)$/$(NEW_PATCH_FILE_NAME).tmp $(NEW_PATCH_FILE_NAME) >& $(NULLDEV)
    @+echo still some problems with win32 generated patches...

create_clean : $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)
    @+echo done
    
patch : $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
    @+echo done

.ENDIF			# "$(L10N_framework)"==""
