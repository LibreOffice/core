#*************************************************************************
#
#   $RCSfile: tg_ext.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: hjs $ $Date: 2001-09-20 19:53:17 $
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


.IF "$(GUI)"=="WNT"
INCLUDE!:=$(shell echo $(INCLUDE:s/\stl//) | sed "s/[ \t]*-I/;/g" )
.EXPORT : INCLUDE
.ENDIF			# "$(GUI)"=="WNT"

#.IF "$(OS)"=="SOLARIS"
#.IF "$(BUILD_SOSL)"==""
#CC:=$(COMPATH)$/bin$/cc
#.EXPORT : CC
#.ENDIF			# "$(BUILD_SOSL)"==""
#.ENDIF			# "$(OS)"=="SOLARIS"

#override
PACKAGE_DIR=build$/$(ROUT)
P_CONFIGURE_DIR=$(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$(CONFIGURE_DIR)
P_BUILD_DIR=$(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$(BUILD_DIR)
P_INSTALL_DIR=$(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$(BUILD_DIR)
P_INSTALL_TARGET_DIR=$(MISC)$/install

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
    

$(MISC)$/%.tar : $(PRJ)$/download$/%.tar.gz
    @+-$(RM) $@
    +gunzip -c $< > $@

$(MISC)$/%.tar : $(PRJ)$/download$/%.tar
    @+-$(RM) $@
    +$(COPY) $< $@

#unntar
$(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE) : $(MISC)$/$(TARFILE_NAME).tar
    @+-$(MKDIR) $(PACKAGE_DIR:d)
    @+-$(MKDIR) $(PACKAGE_DIR)
# see tg_zip.mk
    +cd $(PACKAGE_DIR) && tar -xvf ../../$(ROUT)$/misc$/$(TARFILE_NAME).tar && $(TOUCH) $(UNTAR_FLAG_FILE)
    @+echo make writeable...
.IF "$(GUI)"=="WNT"
    @+cd $(PACKAGE_DIR) && attrib /s -r  >& $(NULLDEV) && $(TOUCH) $(UNTAR_FLAG_FILE)
.ELSE			# "$(GUI)"=="WNT"
    @+cd $(PACKAGE_DIR) && chmod -R +w * && $(TOUCH) $(UNTAR_FLAG_FILE)
.ENDIF			# "$(GUI)"=="WNT"

#add new files to patch
$(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE) : $(T_ADDITIONAL_FILES) $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)
.IF "$(GUI)"=="WNT"
    @+if not exist $@ $(TOUCH) $@
.ELSE			# "$(GUI)"=="WNT"
    @+-test ! -e $@ && $(TOUCH) $@
.ENDIF			# "$(GUI)"=="WNT"

#patch
$(PACKAGE_DIR)$/$(PATCH_FLAG_FILE) : $(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE)
.IF "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""
    +cd $(PACKAGE_DIR) && echo no patch needed...
    +$(TOUCH) $@
.ELSE			# "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""
.IF "$(GUI)"=="WNT"
    +cd $(PACKAGE_DIR) && $(TYPE) ..$/..$/$(PATCH_FILE_NAME) | tr -d "\015" | patch -b -p2 && $(TOUCH) $(PATCH_FLAG_FILE)
.ELSE           # "$(GUI)"=="WNT"
    +cd $(PACKAGE_DIR) && $(TYPE) ..$/..$/$(PATCH_FILE_NAME) | patch -b -p2 && $(TOUCH) $(PATCH_FLAG_FILE)
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF			# "$(PATCH_FILE_NAME)"=="none" ||	"$(PATCH_FILE_NAME)"==""

$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
.IF "$(CONFIGURE_ACTION)" == "none" || "$(CONFIGURE_ACTION)"==""
    +$(TOUCH) $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE)
.ELSE			# "$(CONFIGURE_ACTION)"=="none" || "$(CONFIGURE_ACTION)"==""
    +-$(MKDIR) $(P_CONFIGURE_DIR)
    +cd $(P_CONFIGURE_DIR) && $(CONFIGURE_ACTION) $(CONFIGURE_FLAGS) && $(TOUCH) $(CONFIGURE_FLAG_FILE)
    +mv $(P_CONFIGURE_DIR)$/$(CONFIGURE_FLAG_FILE) $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE)
.ENDIF			# "$(CONFIGURE_ACTION)"=="none" ||	"$(CONFIGURE_ACTION)"==""

    
$(PACKAGE_DIR)$/$(BUILD_FLAG_FILE) : $(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE)
.IF "$(BUILD_ACTION)"=="none" ||	"$(BUILD_ACTION)"==""
    +$(TOUCH) $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
.ELSE			# "$(BUILD_ACTION)"=="none" ||	"$(BUILD_ACTION)"==""
    +-$(MKDIR) $(P_BUILD_DIR)
    @+-$(RM) $@
    +cd $(P_BUILD_DIR) && $(BUILD_ACTION) $(BUILD_FLAGS) && $(TOUCH) $(BUILD_FLAG_FILE)
    +mv $(P_BUILD_DIR)$/$(BUILD_FLAG_FILE) $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
.ENDIF			# "$(BUILD_ACTION)"=="none" ||	"$(BUILD_ACTION)"==""

$(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE) : $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
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
    @+echo copied $(foreach,i,$(OUTDIR2INC) $(shell +$(COPY) $(COPYRECURSE) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/$i $(INCCOM)$/$(i:b) >& $(NULLDEV) && echo $i))
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

$(MISC)$/$(TARFILE_ROOTDIR) : $(MISC)$/$(TARFILE_NAME).tar
    @+-mv $(MISC)$/$(TARFILE_ROOTDIR) $(MISC)$/$(TARFILE_ROOTDIR).old
    @+-rm -rf $(MISC)$/$(TARFILE_ROOTDIR).old
    +cd $(MISC) && tar -xvf $(TARFILE_NAME).tar
.IF "$(GUI)"=="UNX"	
    +$(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"	


.IF "$(P_ADDITIONAL_FILES)"!=""
$(P_ADDITIONAL_FILES) :
    +-echo dummy > $@
.ENDIF			 "$(P_ADDITIONAL_FILES)"!=""

.IF "$(T_ADDITIONAL_FILES)"!=""
$(T_ADDITIONAL_FILES) :
    +-echo dummy > $@
.ENDIF			 "$(T_ADDITIONAL_FILES)"!=""

create_patch : $(MISC)$/$(TARFILE_ROOTDIR) $(P_ADDITIONAL_FILES)
    @+-$(RM) $(MISC)$/$(TARFILE_NAME).patch.tmp >& $(NULLDEV)
    @+-$(RM) $(TARFILE_NAME).patch.bak >& $(NULLDEV)
#ignore returncode of 1 (indicates differences...)	
    +-diff -rc $(MISC)$/$(TARFILE_ROOTDIR) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR) | $(PERL) $(SOLARENV)$/bin$/cleandiff.pl | tr -d "\015" > $(MISC)$/$(TARFILE_NAME).patch.tmp
    @+-mv $(TARFILE_NAME).patch $(TARFILE_NAME).patch.bak >& $(NULLDEV)
    @+-mv $(MISC)$/$(TARFILE_NAME).patch.tmp $(TARFILE_NAME).patch >& $(NULLDEV)
    @+echo still some problems with win32 generated patches...

create_clean : $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)
    @+echo done
    
patch : $(PACKAGE_DIR)$/$(PATCH_FLAG_FILE)
    @+echo done

